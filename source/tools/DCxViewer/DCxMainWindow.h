#pragma once
#include <QMainWindow>

class DCxMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    DCxMainWindow();
    void readSettings();
    void writeSettings();
    void closeEvent(QCloseEvent* event) override;
private slots:
    void refreshMPQList();
    void browseForMPQ();
    void browseForListFile();
    void browseForPaletteFile();
signals:
    void palettesListUpdated(const QStringList& paletteFiles);

private:
    void               createActions();
    class QListWidget* mpqFileList = nullptr;
    class DC6View*     dc6View     = nullptr;
};
