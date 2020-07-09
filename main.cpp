#include <QApplication>
#include <QCommandLineParser>

#include "server.h"
#include "client.h"
#include "common.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("cena-like-demo");
    QApplication::setApplicationVersion("1.0");

    comm_init();

    QCommandLineParser parser;
    parser.setApplicationDescription("Cena-like Demo");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption
            serverOption = {{"s", "server"}, QCoreApplication::translate("main", "Run server.")},
            clientOption = {{"c", "client"}, QCoreApplication::translate("main", "Run client.")};

    parser.addOptions({serverOption, clientOption});

    parser.process(app);

    UIServer *ui_server;
    UIClient *ui_client;

    if (parser.isSet(serverOption)) {
        ui_server = new UIServer;
        ui_server->show();
    } else if (parser.isSet(clientOption)) {
        ui_client = new UIClient;
        ui_client->show();
    }

    return QApplication::exec();
}
