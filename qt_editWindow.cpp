#include "qt_editWindow.hpp"
#include <QDir>
//#include <QFileDialog>
#include <fstream>

MainWindow::MainWindow(Comicfile* comic, QWidget* parent) : QWidget(parent), comic(comic) {
    layout = new QVBoxLayout;
    setLayout(layout);

    rectButton = new QPushButton("Rect");
    circButton = new QPushButton("Circ");
    delButton = new QPushButton("Del");
    textBar = new QLineEdit;
    setButton = new QPushButton("Set");
    saveButton = new QPushButton("Save");

    layout->addWidget(rectButton);
    layout->addWidget(circButton);
    layout->addWidget(delButton);
    layout->addWidget(textBar);
    layout->addWidget(setButton);
    layout->addWidget(saveButton);

    connect(rectButton, &QPushButton::clicked, this, [this] {
        editmode = DM_DRAW;
        submode = DSM_RECT;
    });

    connect(circButton, &QPushButton::clicked, this, [this] {
        editmode = DM_DRAW;
        submode = DSM_CIRC;
    });

    connect(delButton, &QPushButton::clicked, this, [this] {
        if (current) {
            this->comic->del(current);
            current = nullptr;
            update();
        }
    });

    connect(setButton, &QPushButton::clicked, this, [this] {
        if (current) {
            current->setText(textBar->text().toStdString());
        }
    });

    connect(saveButton, &QPushButton::clicked, this, [this] {
        filename_out = "doof";//QFileDialog::getSaveFileName(this, "Save File", QString::fromStdString(filename_out));
    
        // todo: write to file instead of cout
        std::cout << "write file to " << filename_out << std::endl;
        string ext = filename_out.substr(filename_out.rfind('.')+1);
        std::ofstream file_out(filename_out);
        if (!file_out.good()) {
            std::cerr << "could not open file '" << filename_out << "' for writing" << std::endl;
            exit(-1);
        }
        if      (ext ==  "xml") this->comic->writeXML(file_out);
        else if (ext == "json") this->comic->writeJSON(file_out);
        else {
            std::cerr << "cannot save to unknown output file format: " << ext << std::endl;
            exit(-1);
        }
    });
}
