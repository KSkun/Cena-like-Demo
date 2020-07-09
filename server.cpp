#include "common.h"
#include "server.h"
#include "ui_server.h"

UIServer::UIServer(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::UIServer) {
    ui->setupUi(this);

    ui->tableClients->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableClients->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableClients->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableClients->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableClients->setAlternatingRowColors(true);

    u_socket = new QUdpSocket;
    u_socket->bind(UDP_SERVER_PORT, QUdpSocket::ShareAddress);

    connect(u_socket, SIGNAL(readyRead()), this, SLOT(slot_udp_recv()));

    connect(this, SIGNAL(sig_udp_recv_bc_req(const QStringList &)), this,
            SLOT(slot_udp_recv_bc_req(const QStringList &)));
    connect(this, SIGNAL(sig_udp_recv_ping_req(const QStringList &)), this,
            SLOT(slot_udp_recv_ping_req(const QStringList &)));
    connect(this, SIGNAL(sig_udp_recv_ping_rsp(const QStringList &)), this,
            SLOT(slot_udp_recv_ping_rsp(const QStringList &)), Qt::QueuedConnection);
}

UIServer::~UIServer() {
    delete ui;
}

void UIServer::slot_udp_recv() {
    QByteArray data;
    data.resize(u_socket->pendingDatagramSize());
    u_socket->readDatagram(data.data(), data.size());
    qDebug() << "[S] RECV: " << data;
    auto data_list = QString(data).split('&');
    if (data_list.at(0) == "REQ_PING") {
        emit sig_udp_recv_ping_req(data_list);
    }
    if (data_list.at(0) == "RSP_PING") {
        //emit sig_udp_recv_ping_rsp(data_list);
    }
    if (data_list.at(0) == "REQ_BC") {
        emit sig_udp_recv_bc_req(data_list);
    }
}

void UIServer::slot_udp_recv_bc_req(const QStringList &list) {
    QByteArray rsp = ("RSP_BC&" + get_local_addr() + "&" + ui->editServer->text()).toUtf8();
    qDebug() << "[S] SEND: " << rsp;
    u_socket->writeDatagram(rsp, QHostAddress(list.at(1)), UDP_CLIENT_PORT);
}

void UIServer::slot_udp_recv_ping_req(const QStringList &list) {
    QByteArray rsp = ("RSP_PING&" + list.at(2)).toUtf8();
    u_socket->writeDatagram(rsp, QHostAddress(list.at(1)), UDP_CLIENT_PORT);
    qDebug() << "[S] SEND: " << rsp;
}
