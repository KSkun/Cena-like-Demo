#include "common.h"
#include "client.h"
#include "ui_client.h"

UIClient::UIClient(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::UIClient) {
    ui->setupUi(this);

    ui->tableServers->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableServers->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableServers->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableServers->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableServers->setAlternatingRowColors(true);

    servers = new QList<Server>;

    ping_active = false;
    ping_count = 0;
    ping_timers = new QMap<quint16, QPair<int, QTimer *>>;

    u_socket = new QUdpSocket;
    u_socket->bind(UDP_CLIENT_PORT, QUdpSocket::ShareAddress);

    connect(u_socket, SIGNAL(readyRead()), this, SLOT(slot_udp_recv()));

    connect(this, SIGNAL(sig_udp_recv_bc_rsp(const QStringList &)), this,
            SLOT(slot_udp_recv_bc_rsp(const QStringList &)));
    connect(this, SIGNAL(sig_udp_recv_ping_req(const QStringList &)), this,
            SLOT(slot_udp_recv_ping_req(const QStringList &)));
    connect(this, SIGNAL(sig_udp_recv_ping_rsp(const QStringList &)), this,
            SLOT(slot_udp_recv_ping_rsp(const QStringList &)), Qt::QueuedConnection);

    connect(this, SIGNAL(sig_ui_refresh_list()), this,
            SLOT(slot_ui_refresh_list()), Qt::QueuedConnection);
    connect(this, SIGNAL(sig_ui_update_list_info(int, int, const QString &)), this,
            SLOT(slot_ui_update_list_info(int, int, const QString &)), Qt::QueuedConnection);

    connect(this, SIGNAL(sig_ping_start(int)), this,
            SLOT(slot_ping_start(int)), Qt::QueuedConnection);
}

UIClient::~UIClient() {
    delete ui;
}

// udp

void UIClient::slot_udp_recv() {
    QByteArray data;
    while (u_socket->hasPendingDatagrams()) {
        data.resize(u_socket->pendingDatagramSize());
        u_socket->readDatagram(data.data(), data.size());
        qDebug() << "[C] RECV: " << data;
        auto data_list = QString(data).split('&');
        if (data_list.at(0) == "REQ_PING") {
            emit sig_udp_recv_ping_req(data_list);
        }
        if (data_list.at(0) == "RSP_PING") {
            emit sig_udp_recv_ping_rsp(data_list);
        }
        if (data_list.at(0) == "RSP_BC") {
            emit sig_udp_recv_bc_rsp(data_list);
        }
    }
}

void UIClient::slot_udp_recv_bc_rsp(const QStringList &list) {
    servers->append({list.at(1), list.at(2)});
    emit slot_ui_refresh_list();
}

void UIClient::slot_udp_recv_ping_req(const QStringList &list) {
    QByteArray rsp = ("RSP_PING&" + list.at(2)).toUtf8();
    u_socket->writeDatagram(rsp, QHostAddress(list.at(1)), UDP_SERVER_PORT);
    qDebug() << "[C] SEND:" << rsp;
}

void UIClient::slot_udp_recv_ping_rsp(const QStringList &list) {
    if (!ping_active) return;
    auto req_id = list.at(1).toUShort();
    if (!ping_timers->contains(req_id)) return;

    // stop this
    auto[row, timer] = ping_timers->find(req_id).value();
    emit sig_ui_update_list_info(row,
            UI_CLIENT_PING_COL, "OK (" + QString::number(PING_TIMEOUT - timer->remainingTime()) + " ms)");
    timer->stop();
    ping_timers->remove(req_id);
    delete timer;

    // start next
    if (ping_count >= servers->count()) {
        if (ping_timers->empty()) ping_active = false;
        return;
    }
    emit sig_ping_start(ping_count);
    ping_count++;
}

void UIClient::slot_ping_start(int id) {
    emit slot_ui_update_list_info(id, UI_CLIENT_PING_COL, "");
    quint16 req_id;
    while (req_id = rand_uint16(), ping_timers->contains(req_id)) {}
    auto timer = new QTimer;
    ping_timers->insert(req_id, qMakePair(id, timer));
    QByteArray req = ("REQ_PING&" + get_local_addr() + "&" + QString::number(req_id)).toUtf8();
    u_socket->writeDatagram(req, QHostAddress(servers->at(id).ip), UDP_SERVER_PORT);
    qDebug() << "[C] SEND: " << req;
    timer->start(PING_TIMEOUT);
    connect(timer, &QTimer::timeout, this, [=]() {
        slot_ping_timeout(req_id);
    }, Qt::QueuedConnection);
}

void UIClient::slot_ping_timeout(int req_id) {
    auto[row, timer] = ping_timers->find(req_id).value();
    emit sig_ui_update_list_info(row, UI_CLIENT_PING_COL, "Error (Timed Out)");
    ping_timers->remove(req_id);
    delete timer;

    // start next
    if (ping_count >= servers->count()) {
        if (ping_timers->empty()) ping_active = false;
        return;
    }
    emit sig_ping_start(ping_count);
    ping_count++;
}

// ui

void UIClient::on_buttonRefresh_clicked() {
    servers->clear();

    QByteArray data = ("REQ_BC&" + get_local_addr()).toUtf8();
    u_socket->writeDatagram(data, QHostAddress::Broadcast, UDP_SERVER_PORT);
    qDebug() << "[C] SEND: " << data;
}

void UIClient::on_buttonPing_clicked() {
    ping_active = true;
    ping_count = 0;
    for (int i = 0; i < PING_THREAD_COUNT; i++) {
        if (ping_count >= servers->count()) break;
        emit slot_ping_start(ping_count);
        ping_count++;
    }
}

void UIClient::slot_ui_refresh_list() {
    for (auto i = ui->tableServers->rowCount() - 1; i >= 0; i--) {
        ui->tableServers->removeRow(i);
    }
    for (const auto &server : *servers) {
        auto row_cnt = ui->tableServers->rowCount();
        ui->tableServers->insertRow(row_cnt);
        ui->tableServers->setItem(row_cnt, UI_CLIENT_IP_COL, new QTableWidgetItem(server.ip));
        ui->tableServers->setItem(row_cnt, UI_CLIENT_SERV_COL, new QTableWidgetItem(server.name));
    }
}

void UIClient::slot_ui_update_list_info(int row, int column, const QString &info) {
    ui->tableServers->setItem(row, column, new QTableWidgetItem(info));
}
