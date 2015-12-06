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

#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <algorithm>
#include <stdint.h>

using namespace cv;

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

void myConvexityDefects( InputArray _points, InputArray _hull, OutputArray _defects ) {
    Mat points = _points.getMat();
    int ptnum = points.checkVector(2, CV_32S);
    CV_Assert( ptnum > 3 );
    Mat hull = _hull.getMat();
    CV_Assert( hull.checkVector(1, CV_32S) > 2 );
    Ptr<CvMemStorage> storage = cvCreateMemStorage();
    
    CvMat c_points = points, c_hull = hull;
    CvSeq* seq = cvConvexityDefects(&c_points, &c_hull, storage);
    int i, n = seq->total;
    
    if( n == 0 ) {
        _defects.release();
        return;
    }
    
    _defects.create(n, 1, CV_32SC4);
    Mat defects = _defects.getMat();
    
    SeqIterator<CvConvexityDefect> it = Seq<CvConvexityDefect>(seq).begin();
    CvPoint* ptorg = (CvPoint*)points.data;
    
    for( i = 0; i < n; i++, ++it ) {
        CvConvexityDefect& d = *it;
        int idx0 = (int)(d.start - ptorg);
        int idx1 = (int)(d.end - ptorg);
        int idx2 = (int)(d.depth_point - ptorg);
        CV_Assert( 0 <= idx0 && idx0 < ptnum );
        CV_Assert( 0 <= idx1 && idx1 < ptnum );
        CV_Assert( 0 <= idx2 && idx2 < ptnum );
        CV_Assert( d.depth >= 0 );
        int idepth = cvRound(d.depth*256);
        defects.at<Vec4i>(i) = Vec4i(idx0, idx1, idx2, idepth);
    }
}

void checkEllipse(Mat dst, vector<vector<Point> > contours, vector<RotatedRect> &ellipses, float tolerance, int min_size, int max_size) {
    for(vector<vector<Point> >::iterator contour = contours.begin() ; contour != contours.end(); contour++ ) {
	// calculate the area of the original contour and its convex hull.
	vector<Point> hull;
	vector<int> hullIdx;
	vector<Vec4i> hullError;
	convexHull(*contour, hull);
	convexHull(*contour, hullIdx);
	myConvexityDefects(*contour, hullIdx, hullError);
	double contour_area = fabs(contourArea(*contour));
	double hull_area = fabs(contourArea(hull));
	if (hull_area < min_size*min_size) continue;
	// check if the contour is roughly convex (i.e. a potential ellipse).
	if (fabs(contour_area - hull_area) < contour_area*tolerance) {
	    // okay, it's covex enough, we are stupid and pretend it's an ellipse, then.
	    // TODO: Do it right with fitEllipse().
	    if (contour->size() >= 5) {
		RotatedRect ebox = fitEllipse(*contour);
		// Nur achsenparallele Ellipsen interessieren uns.
		// Damit werden ellipsenähnliche Objekte ausgefiltert, die keine Sprechblasen sind.
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
		//circle(dst, (*contour)[hullError[i][2]], 3, Scalar(hullError[i][3]/4, 0, 0));
		if (hullError[i][3] > maxError) {
		    maxError = hullError[i][3];
		    maxIdx   = hullError[i][2];
		}
	    }
	    // Punkt auf der Kontur finden, der
	    // (a) innerhalb eines Lapsus den max. Abstand zur konv. Hülle hat
	    // (b) am dichtesten an Punkt *contour[maxIdx] liegt.
	    if (hullError.size() < 2) continue;
	    int minIdx, minError = INT_MAX;
	    for(uint16_t i=0; i<hullError.size(); i++) {
		if (hullError[i][2] == maxIdx) continue;
		int d = cvRound(dist((*contour)[hullError[i][2]], (*contour)[maxIdx])*256);
		if (d < minError) {
		    minError = d;
		    minIdx   = hullError[i][2];
		}
	    }
	    circle(dst, (*contour)[maxIdx], 3, Scalar(0, 255, 255));
	    circle(dst, (*contour)[minIdx], 3, Scalar(0, 255, 0));
	    // Kontur aufspalten an den oben ermittelten Punkten
	    vector<Point> c1, c2;
	    if (maxIdx > minIdx) {
		int temp = maxIdx;
		maxIdx   = minIdx;
		minIdx   = temp;
	    }
	    for (uint16_t i=0; i<contour->size(); i++) {
		if (maxIdx < i and i <= minIdx) c2.push_back((*contour)[i]);
		else                            c1.push_back((*contour)[i]);
	    }
	    vector<vector<Point> > hullsa;
	    if(c1.size() > 3) hullsa.push_back(c1);
	    if(c2.size() > 3) hullsa.push_back(c2);
	    //if (hullsa.size() > 0) drawContours(dst, hullsa, -1, Scalar(255,255,255));
	    // todo: repeat algo for the shapes gained
	    if (hullsa.size() > 0) checkEllipse(dst, hullsa, ellipses, tolerance, min_size, max_size);
	}
	vector<vector<Point> > hullsa;
	hullsa.push_back(hull);
	drawContours(dst, hullsa, -1, Scalar(255,0,255));
    }
}

void findEllipses(Mat src, Mat dst, vector<RotatedRect> &ellipses, float tolerance=0.0, int min_size=30, int max_size=-1) {
    vector<vector<Point> > contours;
    findContours(src, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
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
    
int main( int argc, char** argv )
{
    Mat src;
    vector<RotatedRect> ellipses;

    if(argc == 2) {
	src = imread(argv[1], 0);
	if (src.data == 0) {
	    fprintf(stderr, "Error loading image file %s\n", argv[1]);
	    exit(-1);
	}
    } else {
	fprintf(stderr, "usage: %s <image file>\n", argv[0]);
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
    
    printf("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
    printf("<comic name=\"%s\" lang=\"de\">\n", argv[1]);
    printf("<bgcolor id=\"default\" r=\"255\" g=\"255\" b=\"255\" />\n");
    printf("<font id=\"default\" name=\"ComicSansMSBold\" size=\"8\" colorr=\"0\" colorg=\"0\" colorb=\"0\" />\n");
    
    std::sort(ellipses.begin(), ellipses.end(), smaller_by_coords);
    vector<RotatedRect>::iterator e = ellipses.begin();
    for(uint16_t i = 0; i < ellipses.size(); i++) {
	ellipse(dst, *e, Scalar(255,0,0), 2);
	printf("<ellipse centerx=\"%d\" centery=\"%d\" radiusx=\"%d\" radiusy=\"%d\" font=\"default\" bgcolor=\"default\">Text%02d\n</ellipse>\n",
	       cvRound(e->center.x), cvRound(e->center.y),
	       cvRound(e->size.height/2)-1, cvRound(e->size.width/2)-1,
	       i);
	++e;
    }
    printf("</comic>\n");
}
