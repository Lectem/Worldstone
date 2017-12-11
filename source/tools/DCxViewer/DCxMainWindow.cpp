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
                if (current) DCxViewerApp::instance()->fileActivated(current->text());
            });
    dock->setWidget(mpqFileList);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    dock = new QDockWidget(tr("File information"), this);
    dock->setFeatures(QDockWidget::DockWidgetFeature::DockWidgetMovable |
                      QDockWidget::DockWidgetFeature::DockWidgetFloatable);
    dock->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

    dc6View = new DCxView(this);

    dock->setWidget(dc6View);
    connect(DCxViewerApp::instance(), &DCxViewerApp::requestDisplayDC6, dc6View,
            &DCxView::displayDCx);

    addDockWidget(Qt::RightDockWidgetArea, dock);

    readSettings();
    connect(DCxViewerApp::instance(), &DCxViewerApp::fileListUpdated, this,
            &DCxMainWindow::refreshMPQList);
    connect(this, &DCxMainWindow::palettesListUpdated, dc6View, &DCxView::palettesListUpdated);
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
    menu->addAction(tr("Choose a custom &palette"), this, &DCxMainWindow::browseForPaletteFile);
    menu->addAction(tr("&Close"), this, &QWidget::close, QKeySequence::Close);
}

void DCxMainWindow::refreshMPQList()
{
    if (mpqFileList) {
        mpqFileList->clear();
        mpqFileList->addItems(DCxViewerApp::instance()->getFileList());
        QStringList paletteFiles =
            DCxViewerApp::instance()->getFileList().filter(".dat", Qt::CaseInsensitive);
        emit palettesListUpdated(paletteFiles);
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

void DCxMainWindow::browseForPaletteFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose a custom palette file"), {},
                                                    tr("Palette files (*.dat)", "All files (*)"));
    if (!fileName.isEmpty()) DCxViewerApp::instance()->setPaletteFile(fileName);
}
