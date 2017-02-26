#include "DCxMainWindow.h"
#include <QDockWidget>
#include <QFileDialog>
#include <QListWidget>
#include <QMainWindow>
#include <QMenuBar>
#include <QSettings>
#include "main.h"

DCxMainWindow::DCxMainWindow()
{
    createActions();
    QDockWidget* dock = new QDockWidget(tr("Files list"), this);
    dock->setFeatures(QDockWidget::DockWidgetFeature::DockWidgetMovable |
                      QDockWidget::DockWidgetFeature::DockWidgetFloatable);
    mpqFileList = new QListWidget(dock);

    connect(mpqFileList, &QListWidget::currentItemChanged,
            [](QListWidgetItem* current, QListWidgetItem* previous) {
                DCxViewerApp::instance()->fileActivated(current->text());
            });
    dock->setWidget(mpqFileList);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    dock = new QDockWidget(tr("File information"), this);
    dock->setFeatures(QDockWidget::DockWidgetFeature::DockWidgetMovable |
                      QDockWidget::DockWidgetFeature::DockWidgetFloatable);
    DC6View* dc6view = new DC6View(this);
    dc6view->loadPalettes(DCxViewerApp::instance()->getPalettesFolder());
    dock->setWidget(dc6view);
    connect(DCxViewerApp::instance(), &DCxViewerApp::requestDisplayDC6, dc6view,
            &DC6View::displayDC6);

    connect(DCxViewerApp::instance(), &DCxViewerApp::paletteFolderChanged, dc6view,
            &DC6View::loadPalettes);

    addDockWidget(Qt::RightDockWidgetArea, dock);

    readSettings();
    connect(DCxViewerApp::instance(), &DCxViewerApp::fileListUpdated, this,
            &DCxMainWindow::refreshMPQList);

    refreshMPQList();
}

void DCxMainWindow::closeEvent(QCloseEvent* event)
{
    writeSettings();
    QMainWindow::closeEvent(event);
}

void DCxMainWindow::readSettings()
{
    QSettings settings;
    settings.beginGroup("DCxMainWindow");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    settings.endGroup();
}

void DCxMainWindow::writeSettings()
{
    QSettings settings;
    settings.beginGroup("DCxMainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.endGroup();
}

void DCxMainWindow::createActions()
{
    QMenu* menu = menuBar()->addMenu(tr("&File"));
    menu->addAction(tr("&Open MPQ file"), this, &DCxMainWindow::browseForMPQ, QKeySequence::Open);
    menu->addAction(tr("&Add a listfile"), this, &DCxMainWindow::browseForListFile);
    menu->addAction(tr("Choose &palettes folder"), this, &DCxMainWindow::browseForPalettesFolder);
    menu->addAction(tr("&Close"), this, &QWidget::close, QKeySequence::Close);
}

void DCxMainWindow::refreshMPQList()
{
    if (mpqFileList) {
        mpqFileList->clear();
        mpqFileList->addItems(DCxViewerApp::instance()->getFileList());
    }
}

void DCxMainWindow::browseForMPQ()
{
    QUrl fileName = QFileDialog::getOpenFileName(this, tr("Open MPQ file"), {},
                                                 tr("MoPaQ files (*.mpq)", "All files (*)"));
    if (!fileName.isEmpty()) DCxViewerApp::instance()->openMpq(fileName);
}

void DCxMainWindow::browseForListFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open lisftile"));
    if (!fileName.isEmpty()) DCxViewerApp::instance()->addListFile(fileName);
}

void DCxMainWindow::browseForPalettesFolder()
{
    QString fileName = QFileDialog::getExistingDirectory(this, tr("Choose palettes folder"));
    if (!fileName.isEmpty()) DCxViewerApp::instance()->setPalettesFolder(fileName);
}
