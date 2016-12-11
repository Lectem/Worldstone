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
      frameSpinBox(new QSpinBox(this)),
      directionSpinBox(new QSpinBox(this))
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(headerInfo);
    layout->addWidget(frameInfo);
    frameSpinBox->setRange(0, 0);
    frameSpinBox->setWrapping(true);
    layout->addWidget(frameSpinBox);
    directionSpinBox->setRange(0, 0);
    directionSpinBox->setWrapping(true);
    layout->addWidget(directionSpinBox);
}

void DC6View::displayDC6(const QString& fileName)
{
    DC6 dc6;
    dc6.Decode(DCxViewerApp::instance()->getFilePtr(fileName));
    const DC6::Header& header = dc6.getHeader();
    const QString      qstr   = QString::fromStdString(
        fmt::format("<table>"
                    "<tr><td>Version:       </td> <td>{}.{}</td></tr>"
                    "<tr><td>Directions:    </td> <td>{}   </td></tr>"
                    "<tr><td>Frames per dir:</td> <td>{}   </td></tr>"
                    "</table>",
                    header.version, header.sub_version, header.directions, header.frames_per_dir));
    headerInfo->setText(qstr);

    frameSpinBox->setValue(0);
    frameSpinBox->setMaximum(header.frames_per_dir - 1);
    frameSpinBox->setSuffix("/" + QString::number(header.frames_per_dir - 1));
    directionSpinBox->setValue(0);
    directionSpinBox->setMaximum(header.directions - 1);
    directionSpinBox->setSuffix("/" + QString::number(header.directions - 1));
}