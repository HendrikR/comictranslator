#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QImage>
#include <QPainter>
#include <QFileDialog>
#include <QColorDialog>

#include "comicfile.hpp"
#include "qt_editWindow.hpp"

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    if (argc < 2 || argc > 3) {
        std::cerr << "usage: " << argv[0] << " <XML/JSON file> [output file]\n";
        exit(-1);
    }

    Comicfile* comic = parse_file(argv[1]);

    QMainWindow window;
    MainWindow* box = new MainWindow(comic, &window);
    // Todo this direct access is not good code
    box->filename_in = argv[1];
    if (argc == 3) {
        box->filename_out = argv[2];
    } else {
        // Derive filename and format for output file
        std::string filename_ext = box->filename_in.substr(box->filename_in.rfind('.')+1);
        box->filename_out = QString::fromStdString(box->filename_in.substr(0, box->filename_in.rfind('.')+1) + comic->getLanguage() + "." + filename_ext).toStdString();
    }

    window.setCentralWidget(box);
    //window.resize(comic->getImageWidth(), comic->getImageHeight() + 30);
    window.show();

    return app.exec();
}
