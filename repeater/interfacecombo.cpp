#include "interfacecombo.h"

InterfaceCombo::InterfaceCombo(QWidget *parent) : QComboBox(parent)
{
}


void InterfaceCombo::populate(QString startsWith)
{
    QList<QNetworkInterface> interfaces( QNetworkInterface::allInterfaces());
    for ( QList<QNetworkInterface>::iterator i = interfaces.begin(); i != interfaces.end(); i++)
    {
        if( ! ((*i).flags() & QNetworkInterface::IsUp) ) continue;
        if( ! (*i).isValid()) continue;
        QList<QNetworkAddressEntry> addresses( (*i).addressEntries() );
        for( QList<QNetworkAddressEntry>::iterator j = addresses.begin(); j != addresses.end(); j++)
        {
            if( (*j).ip().protocol() != QAbstractSocket::IPv4Protocol) continue;
            QString ipstr((*j).ip().toString());
            this->addItem(ipstr);
            if(ipstr.startsWith(startsWith))
            {
                int xx = this->model()->rowCount();
                setCurrentIndex( xx-1 );
            }

            netaddr.append( (*j).ip() );
        }
    }
}


QHostAddress InterfaceCombo::getSelected()
{
    return netaddr[ this->currentIndex() ];
}
