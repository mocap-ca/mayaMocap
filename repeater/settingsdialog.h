#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QHostAddress>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

    QHostAddress getIF();
    int     getCommandPort();
    int     getDataPort();
    QString getRemote();
    bool    getMulticast();

    QHostAddress getUdpHost();
    int          getUdpPort();

    QHostAddress getMayaHost();
    int          getMayaPort();

private:
    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
