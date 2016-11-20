#include "main.h"

#include <QApplication>
#include <QDebug>
#include <QQmlApplicationEngine>
#include <QQmlContext>

void DCxViewerApp::openMpq(const QUrl& mpqFileUrl)
{
    mpqFileName = mpqFileUrl.toLocalFile();
    qDebug() << "Opening..." << mpqFileName;
    mpqArchive = std::make_unique<MpqArchive>(mpqFileUrl.toLocalFile().toStdString());
    if (mpqArchive->good())
        qDebug() << "Successfully opened " << mpqFileUrl.toLocalFile() << "!";
    else
        qDebug() << "Failed to open" << mpqFileUrl.toLocalFile() << ".";
    if (!listFileName.isEmpty())
    {
        mpqArchive->addListFile(listFileName.toStdString());
        if (mpqArchive->good())
            qDebug() << "Successfully added listfile.";
    }
    updateMpqFileList();
}

void DCxViewerApp::addListFile(const QUrl& listFileUrl)
{
    listFileName = listFileUrl.toLocalFile();
    if (mpqArchive)
    {
        mpqArchive->addListFile(listFileName.toStdString());
        if (mpqArchive->good())
            qDebug() << "Successfully added listfile.";
        updateMpqFileList();
    }
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
    qDebug() << "activated " << fileName;
}

int main(int argc, char *argv[])
{
    DCxViewerApp app(argc, argv);
    QStringList files;
    files.append("Item 1");
    files.append("Item 2");
    files.append("Item 3");
    files.append("Item 4");
    app.setFileList(files);

    QQmlApplicationEngine engine;
    
    QQmlContext *ctxt = engine.rootContext();
    ctxt->setContextProperty("app", &app);
    engine.load(QUrl("qrc:///main.qml"));
    return app.exec();
}