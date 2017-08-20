#pragma once

#include <MpqArchive.h>
#include <QWidget>
#include <dc6.h>
#include <vector>

using WorldStone::DC6;
using WorldStone::Palette;

class DC6View : public QWidget
{
    Q_OBJECT
public:
    DC6View(QWidget* parent = nullptr, Qt::WindowFlags flags = {});
public slots:
    void loadPalette(const QString& paletteFile);
    void palettesListUpdated(const QStringList& palettesList);
    void displayDC6(const QString& fileName);
    void nextFrame();
    void refreshDC6Frame();
private:
    bool eventFilter(QObject* obj, QEvent* event);

    Palette         palette;
    class QLabel*      paletteLabel    = nullptr;
    class QListWidget* paletteSelector = nullptr;

    class QLabel*   headerInfo       = nullptr;
    class QLabel*   frameHeaderInfo  = nullptr;
    class QLabel*   frameInfo        = nullptr;
    /// Could be changed to QGraphicsView for debug stuff
    class QLabel*   image            = nullptr;
    class QSpinBox* frameSpinBox     = nullptr;
    class QSpinBox* directionSpinBox = nullptr;

    /// QTimer isn't the best way to do this, but enough for visualization purposes
    class QTimer*  animationTimer  = nullptr;
    class QSlider* framerateSlider = nullptr;

    std::unique_ptr<DC6> currentDC6;
};