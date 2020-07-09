#ifndef SERVER_H
#define SERVER_H

#include <QMainWindow>
#include <QUdpSocket>

namespace Ui {
    class UIServer;
}

class UIServer : public QMainWindow {
Q_OBJECT

public:
    explicit UIServer(QWidget *parent = nullptr);
    ~UIServer() override;

signals:
    void sig_udp_recv_bc_req(const QStringList &);
    void sig_udp_recv_ping_req(const QStringList &);
    //void sig_udp_recv_ping_rsp(const QStringList &);

private slots:
    void slot_udp_recv();
    void slot_udp_recv_bc_req(const QStringList& list);
    void slot_udp_recv_ping_req(const QStringList &list);
    //void slot_udp_recv_ping_rsp(const QStringList &list);

private:
    Ui::UIServer *ui;
    QUdpSocket *u_socket;
};

#endif // SERVER_H
