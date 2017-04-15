#include "DCxView.h"
#include <MpqArchive.h>
#include <QDebug>
#include <QtWidgets>
#include <fmt\format.h>
#include "main.h"

DC6View::DC6View(QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags),
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
            [this] { refreshDC6Frame(); });
    connect(directionSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            [this] { refreshDC6Frame(); });

    connect(animationTimer, &QTimer::timeout, this, &DC6View::nextFrame);

    frameSpinBox->installEventFilter(this);

    layout->addWidget(framerateSlider);
    connect(framerateSlider, &QSlider::valueChanged, [=](int newValue) {
        animationTimer->setInterval(1000.0 / newValue);
        animationTimer->start();
    });
    framerateSlider->setMinimum(1);
    framerateSlider->setMaximum(120);
    framerateSlider->setValue(5);

    animationTimer->stop();
}

void DC6View::loadPalettes(const QString& paletteFolder)
{
    palette.Decode((paletteFolder + "/pal.dat").toUtf8());
}

void DC6View::displayDC6(const QString& fileName)
{
    animationTimer->stop();
    currentDC6 = std::make_unique<DC6>();
    DC6& dc6   = *currentDC6;
	if (!dc6.Decode(DCxViewerApp::instance()->getFilePtr(fileName)))
	{
		qDebug() << "Failed to decode " << fileName;
		return;
	}
    const DC6::Header& header = dc6.getHeader();
    const QString      qstr   = QString::fromStdString(
        fmt::format("<table>"
                    "<tr><td>Version:       </td> <td>{}.{}</td></tr>"
                    "<tr><td>Directions:    </td> <td>{}   </td></tr>"
                    "<tr><td>Frames per dir:</td> <td>{}   </td></tr>"
                    "</table>",
                    header.version, header.sub_version, header.directions, header.frames_per_dir));
    headerInfo->setText(qstr);

    if (!header.directions || !header.frames_per_dir) {
        frameSpinBox->hide();
        directionSpinBox->hide();
    }
    else
    {
        frameSpinBox->setValue(0);
        frameSpinBox->setMaximum(header.frames_per_dir - 1);
        frameSpinBox->setSuffix("/" + QString::number(header.frames_per_dir - 1));
        directionSpinBox->setValue(0);
        directionSpinBox->setMaximum(header.directions - 1);
        directionSpinBox->setSuffix("/" + QString::number(header.directions - 1));
        refreshDC6Frame();
    }
    animationTimer->start();
}

void DC6View::nextFrame()
{
    frameSpinBox->setValue(frameSpinBox->value() + 1);
}

void DC6View::refreshDC6Frame()
{
    if (currentDC6) {
        image->show();
        frameHeaderInfo->show();
        int   dir          = directionSpinBox->value();
        int   frameInDir   = frameSpinBox->value();
        auto& frameHeaders = currentDC6->getFameHeaders();

        const DC6::Header& header      = currentDC6->getHeader();
        size_t             frame       = dir * header.frames_per_dir + frameInDir;
        auto&              frameHeader = frameHeaders[frame];

        // Display frame header information

        const QString qstr = QString::fromStdString(
            fmt::format("<table>"
                        "<tr><td>size:     </td> <td>{}x{}</td></tr>"
                        "<tr><td>offset_x: </td> <td>{}   </td></tr>"
                        "<tr><td>offset_y: </td> <td>{}   </td></tr>"
                        "<tr><td>flip:     </td> <td>{}   </td></tr>"
                        "<tr><td>chunks:   </td> <td>{}   </td></tr>"
                        "</table>",
                        frameHeader.width, frameHeader.height, frameHeader.offset_x,
                        frameHeader.offset_y, (bool)frameHeader.flip, frameHeader.length));
        frameHeaderInfo->setText(qstr);

        // Display the image
        // This costs way too much when playing the animation, but enough for now

        auto   data = currentDC6->decompressFrame(frame);
        QImage bmp(frameHeader.width, frameHeader.height, QImage::Format::Format_Indexed8);
        // Copy line per line because scanlines must be 32bits aligned for Qt
        for (int y = 0; y < frameHeader.height; ++y)
        {
            memcpy(bmp.scanLine(y), &data[frameHeader.width * y], frameHeader.width);
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

bool DC6View::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == frameSpinBox) {
        if (event->type() == QEvent::FocusIn) {
            animationTimer->stop();
        }
    }
    return QObject::eventFilter(obj, event);
}
