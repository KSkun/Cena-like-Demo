#include <QNetworkInterface>
#include "common.h"

QRandomGenerator *glo_rand;

void comm_init() {
    glo_rand = new QRandomGenerator;
    glo_rand->seed(time(nullptr));
}

QString get_local_addr() {
    for (auto &address: QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress::LocalHost)
            return address.toString();
    }
    return QHostAddress(QHostAddress::LocalHost).toString();
}

quint16 rand_uint16() {
    return (quint16) glo_rand->bounded(USHRT_MAX);
}