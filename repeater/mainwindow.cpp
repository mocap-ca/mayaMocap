#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"

#include <QKeySequence>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    SettingsDialog sd(this);
    sd.exec();

    setWindowTitle("Repeater 0.4");

    ui->motive->initialize(sd.getMulticast(),
        sd.getIF().toString(),
        sd.getRemote(),
        sd.getCommandPort(),
        sd.getDataPort(),
        sd.getUdpHost().toString(),
        sd.getUdpPort());

    QPair< QString, int> mayaVal;
    mayaVal.first = sd.getMayaHost().toString();
    mayaVal.second = sd.getMayaPort();

    QString x = QString("Maya: %1:%2").arg(mayaVal.first).arg(mayaVal.second);
    ui->lineEdit_2->setText( x );

    ui->maya->setName("maya");
    ui->maya->initialize( mayaVal );


    if(ui->motive->doConnect() == 0)
    {
        ui->lineEdit->setText("Motive: Connected");
    }
    else
    {
        ui->lineEdit->setText("Motive: Error");
    }


    connect(ui->motive, SIGNAL(outFps(int)), this, SLOT(showFps(int)));


}

void MainWindow::showFps(int fps)
{
    ui->labelFps->setText( QString::number(fps));
}

MainWindow::~MainWindow()
{
    delete ui;
}



bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *e = static_cast<QKeyEvent*>(event);
        int val = e->key();
        if(val == Qt::Key_PageUp )
        {
            ui->maya->sendMessage("pageup\n");
        }
        if(val == Qt::Key_PageDown )
        {
            ui->maya->sendMessage("pagedown\n");
        }
    }
    return false;
}
