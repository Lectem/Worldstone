#include "main.h"
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
    palettesFolder = settings.value("PaletteFolder", QString()).toString();
    if (!mpqFileName.isEmpty()) openMpq(mpqFileName);
    if (!palettesFolder.isEmpty()) emit paletteFolderChanged(palettesFolder);
}

void DCxViewerApp::writeSettings()
{
    QSettings settings;
    settings.setValue("MPQFile", mpqFileName);
    settings.setValue("Listfile", listFileName);
    settings.setValue("PaletteFolder", palettesFolder);
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

void DCxViewerApp::setPalettesFolder(const QUrl& paletteFolderUrl)
{
    if (paletteFolderUrl.isLocalFile())
        palettesFolder = paletteFolderUrl.toLocalFile();
    else
        palettesFolder = paletteFolderUrl.toString();
    emit paletteFolderChanged(palettesFolder);
}

void DCxViewerApp::updateMpqFileList()
{
    if (!mpqArchive)return;
    std::vector<std::string> files = mpqArchive->findFiles();
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

void DCxViewerApp::fileActivated(const QString& fileName)
{
    if (!mpqArchive) return;
    if (fileName.endsWith(".dc6", Qt::CaseInsensitive)) {
        emit requestDisplayDC6(fileName);
    }
}
