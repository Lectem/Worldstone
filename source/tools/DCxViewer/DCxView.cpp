#include "DCxView.h"
#include <MpqArchive.h>
#include <QDebug>
#include <QtWidgets>
#include <fmt\format.h>
#include "main.h"

DC6Sprite::DC6Sprite(StreamPtr&& streamPtr)
{
    if (!dc6.decode(std::move(streamPtr))) {
        valid = false;
        return;
    }
    const DC6::Header& header = dc6.getHeader();
    headerDesc                = QString::fromStdString(
        fmt::format("<table>"
                    "<tr><td>Version:       </td> <td>{}.{}</td></tr>"
                    "<tr><td>Directions:    </td> <td>{}   </td></tr>"
                    "<tr><td>Frames per dir:</td> <td>{}   </td></tr>"
                    "</table>",
                    header.version, header.subVersion, header.directions, header.framesPerDir));

    const size_t totalNbFrames = header.framesPerDir * header.directions;
    for (size_t frameIndex = 0; frameIndex < totalNbFrames; frameIndex++)
    {
        framesImages.push_back(dc6.decompressFrame(frameIndex));
    }
    valid = true;
}

QString DC6Sprite::getFrameHeaderDesc(size_t dir, size_t frameIndex) const
{
    const size_t            dc6FrameIndex = dir * dc6.getHeader().framesPerDir + frameIndex;
    const DC6::FrameHeader& frameHeader   = dc6.getFameHeaders()[dc6FrameIndex];
    const QString           qstr          = QString::fromStdString(
        fmt::format("<table>"
                    "<tr><td>size:     </td> <td>{}x{}</td></tr>"
                    "<tr><td>offsetX:  </td> <td>{}   </td></tr>"
                    "<tr><td>offsetY:  </td> <td>{}   </td></tr>"
                    "<tr><td>flip:     </td> <td>{}   </td></tr>"
                    "<tr><td>chunks:   </td> <td>{}   </td></tr>"
                    "</table>",
                    frameHeader.width, frameHeader.height, frameHeader.offsetX, frameHeader.offsetY,
                    (bool)frameHeader.flip, frameHeader.length));
    return qstr;
}

ImageView<const uint8_t> DC6Sprite::getFrameImage(size_t dir, size_t frameIndex) const
{
    const size_t            dc6FrameIndex = dir * dc6.getHeader().framesPerDir + frameIndex;
    const DC6::FrameHeader& frameHeader   = dc6.getFameHeaders()[dc6FrameIndex];
    return {framesImages[dc6FrameIndex].data(), size_t(frameHeader.width),
            size_t(frameHeader.height), size_t(frameHeader.width)};
}

DCCSprite::DCCSprite(StreamPtr&& streamPtr)
{
    if (!dcc.decode(std::move(streamPtr))) {
        valid = false;
        return;
    }

    const DCC::Header& header = dcc.getHeader();
    headerDesc                = QString::fromStdString(fmt::format("<table>"
                                                    "<tr><td>Version:       </td> <td>{}</td></tr>"
                                                    "<tr><td>Directions:    </td> <td>{}</td></tr>"
                                                    "<tr><td>Frames per dir:</td> <td>{}</td></tr>"
                                                    "<tr><td>Tag:           </td> <td>{}</td></tr>"
                                                    "</table>",
                                                    header.version, header.directions,
                                                    header.framesPerDir, header.tag));

    const size_t nbDirs = header.directions;
    directions.resize(nbDirs);
    directionImgProviders.resize(nbDirs);
    for (uint32_t dirIndex = 0; dirIndex < nbDirs; dirIndex++)
    {
        DCC::Direction& dir = directions[dirIndex];
        dcc.readDirection(dir, dirIndex, directionImgProviders[dirIndex]);
    }
    valid = true;
}

QString DCCSprite::getFrameHeaderDesc(size_t dir, size_t frameIndex) const
{
    const DCC::DirectionHeader& dirHeader   = directions[dir].header;
    const DCC::FrameHeader&     frameHeader = directions[dir].frameHeaders[frameIndex];
    const QString               qstr        = QString::fromStdString(
        fmt::format("<table>"
                    "<tr><td>size:           </td> <td>{}x{}</td></tr>"
                    "<tr><td>offsetX:        </td> <td>{}   </td></tr>"
                    "<tr><td>offsetY:        </td> <td>{}   </td></tr>"
                    "<tr><td>BottomUp:       </td> <td>{}   </td></tr>"
                    "<tr><td>optional bytes: </td> <td>{}   </td></tr>"
                    "<tr><td>coded bytes:    </td> <td>{}   </td></tr>"
                    "<tr><td>variable0:      </td> <td>{}   </td></tr>"
                    "</table>",
                    frameHeader.width, frameHeader.height, frameHeader.xOffset, frameHeader.yOffset,
                    (bool)frameHeader.frameBottomUp, frameHeader.optionalBytes,
                    frameHeader.codedBytes, frameHeader.variable0));
    return qstr;
}

ImageView<const uint8_t> DCCSprite::getFrameImage(size_t dir, size_t frameIndex) const
{
    return directionImgProviders[dir].getImage(frameIndex);
}

DCxView::DCxView(QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags),
      paletteSelector(new QListWidget(this)),
      headerInfo(new QLabel(this)),
      frameInfo(new QLabel(this)),
      frameHeaderInfo(new QLabel(this)),
      image(new QLabel(this)),
      frameSpinBox(new QSpinBox(this)),
      directionSpinBox(new QSpinBox(this)),
      animationTimer(new QTimer(this)),
      framerateSlider(new QSlider(Qt::Horizontal, this))
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(headerInfo);
    layout->addWidget(frameInfo);
    layout->addWidget(frameHeaderInfo);
    layout->addWidget(image);
    layout->addStretch();

    frameSpinBox->setRange(0, 0);
    frameSpinBox->setWrapping(true);
    layout->addWidget(new QLabel(tr("Frame"), frameSpinBox));
    layout->addWidget(frameSpinBox);
    directionSpinBox->setRange(0, 0);
    directionSpinBox->setWrapping(true);
    layout->addWidget(new QLabel(tr("Direction"), directionSpinBox));
    layout->addWidget(directionSpinBox);

    connect(frameSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            [this] { refreshFrame(); });
    connect(directionSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            [this] { refreshFrame(); });

    connect(animationTimer, &QTimer::timeout, this, &DCxView::nextFrame);

    frameSpinBox->installEventFilter(this);

    layout->addWidget(framerateSlider);
    connect(framerateSlider, &QSlider::valueChanged, [=](int newValue) {
        if (newValue) {
            animationTimer->setInterval(1000.0 / newValue);
            animationTimer->start();
        }
        else
            animationTimer->stop();
    });
    framerateSlider->setMinimum(0);
    framerateSlider->setMaximum(120);
    framerateSlider->setValue(5);

    animationTimer->stop();

    paletteSelector->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    paletteLabel = new QLabel(tr("Palettes"), paletteSelector);
    layout->addWidget(paletteLabel);
    loadPalette(DCxViewerApp::instance()->getPaletteFile());
    connect(paletteSelector, &QListWidget::itemActivated, [this](QListWidgetItem* item) {
        DCxViewerApp::instance()->setPaletteFile(QUrl::fromLocalFile(item->text()));
    });
    connect(DCxViewerApp::instance(), &DCxViewerApp::paletteFileChanged,
            [this](const QString& paletteFile) { loadPalette(paletteFile); });
    layout->addWidget(paletteSelector);
}

void DCxView::loadPalette(const QString& paletteFile)
{
    WorldStone::StreamPtr stream = DCxViewerApp::instance()->getFilePtr(paletteFile);
    if (stream) {
        paletteLabel->setText(QString("Palettes (Current=%1)").arg(paletteFile));
        palette.decode(stream.get());
    }
    refreshFrame();
}

void DCxView::palettesListUpdated(const QStringList& palettesList)
{
    paletteSelector->clear();
    paletteSelector->addItems(palettesList);
}

void DCxView::displayDCx(const QString& fileName)
{
    animationTimer->stop();
    currentDCx = nullptr;
    if (fileName.endsWith(".dc6", Qt::CaseInsensitive))
        currentDCx = std::make_unique<DC6Sprite>(DCxViewerApp::instance()->getFilePtr(fileName));
    else if (fileName.endsWith(".dcc", Qt::CaseInsensitive))
        currentDCx = std::make_unique<DCCSprite>(DCxViewerApp::instance()->getFilePtr(fileName));

    if (!currentDCx->isValid()) {
        qDebug() << "Failed to decode a valid frame data" << fileName;
        currentDCx = nullptr;
        return;
    }

    DCxSprite& dcx = *currentDCx;

    headerInfo->setText(dcx.getHeaderDesc());

    const size_t nbDirs       = dcx.getNbDirections();
    const size_t framesPerDir = dcx.getNbFramesPerDir();

    if (!nbDirs || !framesPerDir) {
        frameSpinBox->hide();
        directionSpinBox->hide();
    }
    else
    {
        directionSpinBox->setValue(0);
        directionSpinBox->setMaximum(int(nbDirs - 1));
        directionSpinBox->setSuffix("/" + QString::number(int(nbDirs - 1)));
        frameSpinBox->setValue(0);
        frameSpinBox->setMaximum(int(framesPerDir - 1));
        frameSpinBox->setSuffix("/" + QString::number(int(framesPerDir - 1)));
        refreshFrame();
    }
    if (framerateSlider->value() > 0) animationTimer->start();
}

void DCxView::nextFrame() { frameSpinBox->setValue(frameSpinBox->value() + 1); }

void DCxView::refreshFrame()
{
    if (currentDCx) {
        image->show();
        frameHeaderInfo->show();
        int   dir          = directionSpinBox->value();
        int   frameInDir   = frameSpinBox->value();

        if (!animationTimer->isActive()) {
            frameHeaderInfo->setText(currentDCx->getFrameHeaderDesc(dir, frameInDir));
        }
        // Display the image
        // This costs way too much when playing the animation, but enough for now
        ImageView<const uint8_t> frameView = currentDCx->getFrameImage(dir, frameInDir);
        if (!frameView.isValid()) {
            qDebug() << "Failed to get a valid frame data";
        }
        QImage bmp(int(frameView.width), int(frameView.height), QImage::Format::Format_Indexed8);
        // Copy line per line because scanlines must be 32bits aligned for Qt
        for (int y = 0; y < frameView.height; ++y)
        {
            memcpy(bmp.scanLine(y), &frameView(0, y), frameView.width);
        }
        bmp.setColorCount(Palette::colorCount);
        if (palette.isValid()) {
            for (int i = 0; i < Palette::colorCount; ++i)
            {
                auto& c = palette.colors[i];
                bmp.setColor(i, QColor(c.r, c.g, c.b).rgb());
            }
        }
        else // Display grayscale if no palette
        {
            for (int i = 0; i < Palette::colorCount; ++i)
            {
                bmp.setColor(i, QColor(i, i, i).rgb());
            }
        }
        image->setPixmap(QPixmap::fromImage(bmp));
    }
    else
    {
        frameHeaderInfo->hide();
        image->hide();
    }
}

bool DCxView::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == frameSpinBox) {
        if (event->type() == QEvent::FocusIn) {
            animationTimer->stop();
        }
    }
    return QObject::eventFilter(obj, event);
}
