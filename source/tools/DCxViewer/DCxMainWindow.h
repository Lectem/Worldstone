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
    void browseForPalettesFolder();

private:
    void               createActions();
    class QListWidget* mpqFileList = nullptr;
};
