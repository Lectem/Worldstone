#pragma once

#include <FileStream.h>
#include <ImageView.h>
#include <MpqArchive.h>
#include <QWidget>
#include <Vector.h>
#include <dc6.h>
#include <dcc.h>
#include <vector>

using WorldStone::ImageView;
using WorldStone::DC6;
using WorldStone::DCC;
using WorldStone::Palette;
using WorldStone::StreamPtr;
using WorldStone::Vector;

struct DCxSprite
{
    virtual bool    isValid() const           = 0;
    virtual size_t  getNbDirections() const   = 0;
    virtual size_t  getNbFramesPerDir() const = 0;
    virtual QString getHeaderDesc() const     = 0;
    virtual QString getFrameHeaderDesc(size_t dir, size_t frameIndex) const             = 0;
    virtual ImageView<const uint8_t> getFrameImage(size_t dir, size_t frameIndex) const = 0;
    ~DCxSprite() {}
};

struct DC6Sprite : public DCxSprite
{
    DC6Sprite(StreamPtr&& streamPtr);
    bool           isValid() const override { return valid; }
    size_t         getNbDirections() const override { return dc6.getHeader().directions; }
    virtual size_t getNbFramesPerDir() const override { return dc6.getHeader().framesPerDir; }
    QString        getHeaderDesc() const override { return headerDesc; }
    QString getFrameHeaderDesc(size_t dir, size_t frameIndex) const override;
    ImageView<const uint8_t> getFrameImage(size_t dir, size_t frameIndex) const override;

private:
    Vector<Vector<uint8_t>> framesImages;
    DC6                     dc6;
    QString                 headerDesc;
    bool                    valid = false;
};

struct DCCSprite : public DCxSprite
{
    DCCSprite(StreamPtr&& streamPtr);
    bool           isValid() const override { return valid; }
    size_t         getNbDirections() const override { return dcc.getHeader().directions; }
    virtual size_t getNbFramesPerDir() const override { return dcc.getHeader().framesPerDir; }
    QString        getHeaderDesc() const override { return headerDesc; }
    QString getFrameHeaderDesc(size_t dir, size_t frameIndex) const override;
    ImageView<const uint8_t> getFrameImage(size_t dir, size_t frameIndex) const override;

private:
    using ImgProvider = WorldStone::SimpleImageProvider<uint8_t>;
    Vector<DCC::Direction> directions;
    Vector<ImgProvider>    directionImgProviders;
    DCC                    dcc;
    QString                headerDesc;
    bool                   valid = false;
};

class DCxView : public QWidget
{
    Q_OBJECT
public:
    DCxView(QWidget* parent = nullptr, Qt::WindowFlags flags = {});
public slots:
    void loadPalette(const QString& paletteFile);
    void palettesListUpdated(const QStringList& palettesList);
    void displayDCx(const QString& fileName);
    void nextFrame();
    void refreshFrame();

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

    std::unique_ptr<DCxSprite> currentDCx;
};