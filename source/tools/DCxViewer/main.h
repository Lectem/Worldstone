#pragma once
#include <QGuiApplication>
#include <QUrl>
#include <memory>
#include <MpqArchive.h>

using WorldStone::MpqArchive;

class DCxViewerApp : public QGuiApplication
{
    Q_OBJECT
    Q_PROPERTY(QString mpqFileName READ getMpqFileName);
    Q_PROPERTY(QStringList mpqFiles READ getFileList WRITE setFileList NOTIFY fileListUpdated);
    QStringList mpqFiles;
    QString mpqFileName;
    QString listFileName;
    std::unique_ptr<MpqArchive> mpqArchive;
public:
    DCxViewerApp(int & argc, char ** argv):QGuiApplication(argc,argv){}
    const QStringList& getFileList()const { return mpqFiles; };
    void setFileList(QStringList & newMpqFilesList);
    void updateMpqFileList();
    QString getMpqFileName() { return mpqFileName; }
public slots :
    void openMpq(const QUrl& mpqFileUrl);
    void addListFile(const QUrl& listFileUrl);
    void fileActivated(const QString& fileName);
signals:
    void fileListUpdated();
};