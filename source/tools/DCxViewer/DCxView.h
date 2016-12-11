#pragma once

#include <MpqArchive.h>
#include <QWidget>
#include <dc6.h>

using WorldStone::DC6;

class DC6View : public QWidget
{
    Q_OBJECT
public:
    DC6View(QWidget* parent = nullptr, Qt::WindowFlags flags = {});
public slots:
    void displayDC6(const QString& fileName);

private:
    Palette         palette;
    class QLabel*   headerInfo       = nullptr;
    class QLabel*   frameInfo        = nullptr;
    class QSpinBox* frameSpinBox     = nullptr;
    class QSpinBox* directionSpinBox = nullptr;
};