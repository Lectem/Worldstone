#include "main.h"
#include <FileStream.h>
#include <QtWidgets>
#include "DCxMainWindow.h"

int main(int argc, char* argv[])
{
    QCoreApplication::setOrganizationName("WorldStone");
    QCoreApplication::setApplicationName("DCxViewer");
    DCxViewerApp  app(argc, argv);
    DCxMainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}

DCxViewerApp::DCxViewerApp(int& argc, char** argv) : QApplication(argc, argv)
{
    readSettings();
}

DCxViewerApp::~DCxViewerApp()
{
    writeSettings();
}

void DCxViewerApp::readSettings()
{
    QSettings settings;
    mpqFileName  = settings.value("MPQFile", QString()).toString();
    listFileName = settings.value("Listfile", QString()).toString();
    paletteFile  = settings.value("PaletteFile", QString()).toString();
    if (!mpqFileName.isEmpty()) openMpq(mpqFileName);
    if (!paletteFile.isEmpty()) emit paletteFileChanged(paletteFile);
}

void DCxViewerApp::writeSettings()
{
    QSettings settings;
    settings.setValue("MPQFile", mpqFileName);
    settings.setValue("Listfile", listFileName);
    settings.setValue("PaletteFile", paletteFile);
}

void DCxViewerApp::openMpq(const QUrl& mpqFileUrl)
{
    if (mpqFileUrl.isLocalFile())
        mpqFileName = mpqFileUrl.toLocalFile();
    else
        mpqFileName = mpqFileUrl.toString();
    mpqArchive      = std::make_unique<MpqArchive>(mpqFileName.toStdString());
    if (!mpqArchive->good()) qDebug() << "Failed to open" << mpqFileName << ".";
    if (!listFileName.isEmpty())
    {
        mpqArchive->addListFile(listFileName.toStdString());
    }
    updateMpqFileList();
}

void DCxViewerApp::addListFile(const QUrl& listFileUrl)
{
    if (listFileUrl.isLocalFile())
        listFileName = listFileUrl.toLocalFile();
    else
        listFileName = listFileUrl.toString();
    if (mpqArchive)
    {
        mpqArchive->addListFile(listFileName.toStdString());
        updateMpqFileList();
    }
}

void DCxViewerApp::setPaletteFile(const QUrl& paletteUrl)
{
    if (paletteUrl.isLocalFile())
        paletteFile = paletteUrl.toLocalFile();
    else
        paletteFile = paletteUrl.toString();
    emit paletteFileChanged(paletteFile);
}

void DCxViewerApp::updateMpqFileList()
{
    if (!mpqArchive)return;
    std::vector<MpqArchive::path> files = mpqArchive->findFiles();
    mpqFiles.clear();
    emit fileListUpdated();
    for (std::string & file : files)
        mpqFiles.push_back(QString::fromStdString(file));
    mpqFiles.sort();
    emit fileListUpdated();
}

void DCxViewerApp::setFileList(QStringList & newMpqFilesList)
{
    mpqFiles.swap(newMpqFilesList);
    emit fileListUpdated();
}

WorldStone::StreamPtr DCxViewerApp::getFilePtr(const QString& fileName)
{
    WorldStone::StreamPtr stream;
    QString               fileNameBackslashes = fileName;
    fileNameBackslashes.replace('/', '\\');
    qDebug() << "getFilePtr(" << fileName << ")";
    stream = mpqArchive->open(fileNameBackslashes.toStdString());
    if (!stream) {
        stream = std::make_unique<WorldStone::FileStream>(fileName.toStdString());
        if (stream->fail()) stream = nullptr;
    }
    return stream;
}

void DCxViewerApp::fileActivated(const QString& fileName)
{
    if (!mpqArchive) return;
    if (fileName.endsWith(".dc6", Qt::CaseInsensitive) ||
        fileName.endsWith(".dcc", Qt::CaseInsensitive))
    {
        emit requestDisplayDC6(fileName);
    }
}
