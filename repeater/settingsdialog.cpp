#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include "interfacecombo.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    ui->comboBoxIF->populate("192.");

    ui->lineEditCommandPort->setText("1510");
    ui->lineEditDataPort->setText("1511");
    ui->lineEditRemote->setText("192.168.1.20");

    ui->lineEditUdpHost->setText("192.168.1.22");
    ui->lineEditUdpPort->setText("9119");

    ui->lineEditMayaHost->setText("192.168.1.22");
    ui->lineEditMayaPort->setText("8885");
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}


QHostAddress SettingsDialog::getIF()
    { return ui->comboBoxIF->getSelected(); }

int SettingsDialog::getCommandPort()
    { return ui->lineEditCommandPort->text().toInt(); }

int SettingsDialog::getDataPort()
    { return ui->lineEditDataPort->text().toInt(); }

QString SettingsDialog::getRemote()
    { return ui->lineEditRemote->text(); }

bool SettingsDialog::getMulticast()
    { return ui->checkBoxMulticast->isChecked(); }

QHostAddress SettingsDialog::getUdpHost()
    { return QHostAddress( ui->lineEditUdpHost->text() ); }

int SettingsDialog::getUdpPort()
    { return ui->lineEditUdpPort->text().toInt(); }

QHostAddress SettingsDialog::getMayaHost()
    { return QHostAddress( ui->lineEditMayaHost->text() ); }

int SettingsDialog::getMayaPort()
    { return ui->lineEditMayaPort->text().toInt(); }


