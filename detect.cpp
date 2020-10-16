/*****************************************************************
 * detect.cpp
 *
 * Copyright 2012-2014, Hendrik Radke
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the license contained in the
 * COPYING file.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Detect (most) text bubbles in a comic and construct an XML file
 * with these bubbles ready to be used with draw.cpp.
 */

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <stdint.h>
#include <vector>
#include <Imlib2.h>

using namespace cv;
using std::vector;

//
// We need this to be high enough to get rid of things that are too small too
// have a definite shape.  Otherwise, they will end up as ellipse false positives.
//
#define MIN_AREA 100.00
//
// One way to tell if an object is an ellipse is to look at the relationship
// of its area to its dimensions.  If its actual occupied area can be estimated
// using the well-known area formula Area = PI*A*B, then it has a good chance of
// being an ellipse.
//
// This value is the maximum permissible error between actual and estimated area.
//
#define MAX_TOL  100.00

float dist(Point a, Point b) {
    return sqrt(pow((a-b).x, 2) + pow((a-b).y, 2));
}

bool isMonotonous(vector<int> hullIdx) {
    // checks if the numbers in hullIdx are monotonous,
    // i.e. all in descending / ascending order
    int signum = hullIdx[1] - hullIdx[0];
    for (size_t i=2; i<hullIdx.size(); i++) {
	int new_signum = hullIdx[i] - hullIdx[i-1];
	if ( signum < 0 && new_signum > 0 ) return false;
	if ( signum > 0 && new_signum < 0 ) return false;
    }
    return true;
}

void checkEllipse(cv::Mat dst, vector<vector<Point> > contours, vector<RotatedRect> &ellipses, float tolerance, int min_size, int max_size) {
    for(const auto& contour : contours) {
	// calculate the area of the original contour and its convex hull.
	vector<Point> hull;
	vector<int> hullIdx;
	vector<Vec4i> hullError;
	convexHull(contour, hull);
	convexHull(contour, hullIdx);
	// convexityDefects cannot work with non-monotonous hulls -- skip them.
	if (! isMonotonous(hullIdx)) continue;
	cv::convexityDefects(contour, hullIdx, hullError);
	double contour_area = fabs(contourArea(contour));
	double hull_area = fabs(contourArea(hull));
	if (hull_area < min_size*min_size) continue;
	// check if the contour is roughly convex (i.e. a potential ellipse).
	if (fabs(contour_area - hull_area) < contour_area*tolerance) {
	    // okay, it's covex enough, we are stupid and pretend it's an ellipse, then.
	    if (contour.size() >= 5) {
		RotatedRect ebox = fitEllipse(contour);
		// We're only interested in Axis-parallel Ellipses.
		// This rejects ellipsis-like objects which are no speech bubbles.
		if (fabs(ebox.angle-90) < 5 || fabs(ebox.angle-270) < 5) {
		    ellipses.push_back(ebox);
		}
	    }
	} else { // Shape is quite concave. Subdivide contour and repeat the algorithm.
	    // subdivide along convex hull points
	    // Punkt auf der Kontur finden, der (über alle Lapsi) den größten Abstand zur konvexen Hülle hat.
	    int maxIdx, maxError = 0;
	    vector<Point> nonHull;
	    for(uint16_t i=0; i<hullError.size(); i++) {
		//circle(dst, contour[hullError[i][2]], 3, Scalar(hullError[i][3]/4, 0, 0));
		if (hullError[i][3] > maxError) {
		    maxError = hullError[i][3];
		    maxIdx   = hullError[i][2];
		}
	    }
	    // Punkt auf der Kontur finden, der
	    // (a) innerhalb eines Lapsus den max. Abstand zur konv. Hülle hat
	    // (b) am dichtesten an Punkt contour[maxIdx] liegt.
	    if (hullError.size() < 2) continue;
	    int minIdx, minError = INT_MAX;
	    for(uint16_t i=0; i<hullError.size(); i++) {
		if (hullError[i][2] == maxIdx) continue;
		int d = cvRound(dist(contour[hullError[i][2]], contour[maxIdx])*256);
		if (d < minError) {
		    minError = d;
		    minIdx   = hullError[i][2];
		}
	    }
	    circle(dst, contour[maxIdx], 3, Scalar(0, 255, 255));
	    circle(dst, contour[minIdx], 3, Scalar(0, 255, 0));
	    // Kontur aufspalten an den oben ermittelten Punkten
	    vector<Point> c1, c2;
	    if (maxIdx > minIdx) {
		int temp = maxIdx;
		maxIdx   = minIdx;
		minIdx   = temp;
	    }
	    for (uint16_t i=0; i<contour.size(); i++) {
		if (maxIdx < i and i <= minIdx) c2.push_back(contour[i]);
		else                            c1.push_back(contour[i]);
	    }
	    vector<vector<Point> > hullsa;
	    if(c1.size() > 3) hullsa.push_back(c1);
	    if(c2.size() > 3) hullsa.push_back(c2);
	    if (hullsa.size() > 0) drawContours(dst, hullsa, -1, Scalar(0,255,255));
	    if (hullsa.size() > 0) checkEllipse(dst, hullsa, ellipses, tolerance, min_size, max_size);
	}
	vector<vector<Point> > hullsb;
	hullsb.push_back(hull);
	drawContours(dst, hullsb, -1, Scalar(255,0,255));
    }
}

void findEllipses(cv::Mat src, cv::Mat dst, vector<RotatedRect> &ellipses, float tolerance=0.0, int min_size=30, int max_size=-1) {
    vector<vector<Point> > contours;
    findContours(src, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
    checkEllipse(dst, contours, ellipses, tolerance, min_size, max_size);
}

// Sortierfunktion für Ellipsen
bool smaller_by_coords(const RotatedRect &a, const RotatedRect &b) {
    if (a.center.y < b.center.y) {
	return true;
    } else {
	return false;
    }
}

int main( int ARGC, char** ARGV )
{
    cv::Mat src;
    vector<RotatedRect> ellipses;

    if(ARGC == 2) {
	src = cv::imread(ARGV[1], 0);
	if (src.data == 0) {
            std::cerr << "Error loading image file " << ARGV[1] << std::endl;
	    exit(-1);
        }
    } else {
        std::cerr << "usage: "<< ARGV[0] <<" <image file>" << std::endl;
	exit(0);
    }
    Mat dst(src.rows, src.cols, CV_8UC3);
    dst.setTo(0);

    // Rand schwarz malen, damit auch die Sprechblasen erkannt werden, die über den Rahmen hinausragen.
    floodFill(src, Point(0,0), 0);
    floodFill(src, Point(src.cols-1,0), 0);
    floodFill(src, Point(src.cols-1,src.rows-1), 0);
    floodFill(src, Point(0,src.rows-1), 0);
    // Nur blütenweiße Sprechblasen (mit schwarzem Text drin) auf schwarzem Grund übrig lassen.
    threshold(src, src, 254, 0, THRESH_TOZERO);
    // Text wegrechnen ("2. Ableitung" des Bildes)
    dilate(src, src, Mat(), Point(-1,-1), 2);
    //cv::imwrite("debug.png", src);
    // Und nach achsenparallelen Ellipsen suchen.
    findEllipses(src, dst, ellipses, 0.03);
    cv::imwrite("debug.png", dst);

    // TODO: use Comicfile::writeXML(ostream& str) instead
    std::cout << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
    std::cout << "<comic name=\""<< ARGV[1] <<"\" lang=\"de\">\n";
    std::cout << "<bgcolor id=\"default\" r=\"255\" g=\"255\" b=\"255\" />\n";
    std::cout << "<font id=\"default\" name=\"ComicSansMSBold\" size=\"8\" colorr=\"0\" colorg=\"0\" colorb=\"0\" />\n";

    std::sort(ellipses.begin(), ellipses.end(), smaller_by_coords);
    for(uint16_t i = 0; i < ellipses.size(); i++) {
	const RotatedRect& e = ellipses[i];
	ellipse(dst, e, Scalar(255,0,0), 2);
	std::cout << "<ellipse "
                  << "centerx=\""<< cvRound(e.center.x) << '"'
                  << "centery=\""<< cvRound(e.center.y) << '"'
                  << "radiusx=\""<< cvRound(e.size.height/2 - 1) << '"'
                  << "radiusy=\""<< cvRound(e.size.height/2 - 1) << '"'
                  << "font=\"default\" bgcolor=\"default\">"
                  << "Text"<< std::right << std::setfill('0') << std::setw(2) << i << "</ellipse>\n";
    }
    std::cout << "</comic>\n";
}
