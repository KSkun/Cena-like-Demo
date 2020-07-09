#ifndef CLIENT_H
#define CLIENT_H

#include <QMainWindow>
#include <QSignalMapper>
#include <QTimer>
#include <QUdpSocket>

namespace Ui {
    class UIClient;
}

class UIClient : public QMainWindow {
Q_OBJECT

#define UI_CLIENT_IP_COL 0
#define UI_CLIENT_SERV_COL 1
#define UI_CLIENT_PING_COL 2
#define UI_CLIENT_AUTH_COL 3
#define UI_CLIENT_FETCH_COL 4

public:
    explicit UIClient(QWidget *parent = nullptr);
    ~UIClient() override;

signals:
    void sig_udp_recv_bc_rsp(const QStringList &);
    void sig_udp_recv_ping_req(const QStringList &);
    void sig_udp_recv_ping_rsp(const QStringList &);

    void sig_ui_refresh_list();
    void sig_ui_update_list_info(int, int, const QString &);

    void sig_ping_start(int);

private slots:
    void slot_udp_recv();
    void slot_udp_recv_bc_rsp(const QStringList &list);
    void slot_udp_recv_ping_req(const QStringList &list);
    void slot_udp_recv_ping_rsp(const QStringList &list);

    void on_buttonRefresh_clicked();
    void on_buttonPing_clicked();
    void slot_ui_refresh_list();
    void slot_ui_update_list_info(int, int, const QString &);

    void slot_ping_start(int);
    void slot_ping_timeout(int);

private:
    Ui::UIClient *ui;
    QUdpSocket *u_socket;

    struct Server {
        QString ip, name;
    };

    QList<Server> *servers;

    bool ping_active;
    int ping_count;
    QMap<quint16, QPair<int, QTimer *>> *ping_timers; // first: row, second: timer
};

#endif // CLIENT_H
