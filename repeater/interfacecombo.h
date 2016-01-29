#ifndef INTERFACECOMBO_H
#define INTERFACECOMBO_H

#include <QNetworkInterface>
#include <QList>
#include <QComboBox>

class InterfaceCombo : public QComboBox
{
    Q_OBJECT

public:
    InterfaceCombo(QWidget*);

    void populate(QString startsWith);

    QHostAddress getSelected();

private:

    QList< QHostAddress > netaddr;
};

#endif // INTERFACECOMBO_H
