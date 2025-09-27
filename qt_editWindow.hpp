#pragma once

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include "comicfile.hpp"
#undef Unsorted

class MainWindow : public QWidget {
    Q_OBJECT

private:
    enum EditMode { DM_HOVER, DM_DRAW, DM_MOVE, DM_RESIZE, DM_PICK };
    enum EditSubMode { DSM_RECT, DSM_CIRC };

    QImage image;
    QVBoxLayout* layout;
    QPushButton* rectButton;
    QPushButton* circButton;
    QPushButton* delButton;
    QLineEdit* textBar;
    QPushButton* setButton;
    QPushButton* saveButton;

    Comicfile* comic;
    Bubble* current;
    int oldx, oldy;
    EditMode editmode = DM_HOVER;
    EditSubMode submode = DSM_RECT;

public:
    MainWindow(Comicfile* comic, QWidget* parent = nullptr);

    /*void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;*/

    // todo why do i need filename_out here, not determined by a file dialog
    std::string filename_in, filename_out;
};
