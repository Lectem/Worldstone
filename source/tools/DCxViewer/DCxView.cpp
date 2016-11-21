#include "DCxView.h"
#include <MpqArchive.h>
#include <QDebug>

void QDC6::Decode(QString filename, QString mpqFile)
{
    Reset();
    // Not the correct way to do it...
    mpqArchive = std::move(WorldStone::MpqArchive{mpqFile.toStdString()});
    if (!mpqArchive) return;
    DC6::Decode(mpqArchive.open(filename.toStdString()));
    qHeader = header;
    emit updated();
}