#include "main.h"

#include <QApplication>
#include <QDebug>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "DCxView.h"

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
    if (!mpqArchive) return;
    Palette palette;
    palette.Decode(
        "E:\\progs\\PC\\diabloSS\\resources\\PaulSiramy\\dcc\\merge_dcc\\datas\\act1.dat");
    DC6 dc6;
    dc6.Decode(mpqArchive->open(fileName.toStdString()));
    dc6.exportToPPM("test", palette);
}

int main(int argc, char *argv[])
{
    qmlRegisterType<QDC6Header>("DCxViewer", 1, 0, "QDC6Header");
    qmlRegisterType<QDC6FrameHeader>("DCxViewer", 1, 0, "QDC6FrameHeader");
    qmlRegisterType<QDC6>("DCxViewer", 1, 0, "QDC6");

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