#ifndef COMMON_H
#define COMMON_H

#include <QRandomGenerator>
#include <QString>

#define UDP_SERVER_PORT 23333
#define UDP_CLIENT_PORT 23334

#define PING_THREAD_COUNT 5
#define PING_TIMEOUT 1000

void comm_init();

QString get_local_addr();

extern QRandomGenerator *glo_rand;

quint16 rand_uint16();

#endif
