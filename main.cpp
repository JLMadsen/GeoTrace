#include "mainwindow.h"

int main(int argc, char *argv[])
{
    // check if env supports OpenSSL
    qDebug() << "OpenSSL support status:" << QSslSocket::supportsSsl();

    QApplication app (argc, argv);

    app.setWindowIcon(QIcon(":/images/favicon.png"));

    MainWindow window;
    window.show();

    // start by fetching my IP
    window.fetch_origin();

    return app.exec();
}
