#include "mainwindow.h"

int main(int argc, char *argv[])
{
    // check if env supports OpenSSL
    qDebug() << "OpenSSL support status:" << QSslSocket::supportsSsl();
    QApplication app (argc, argv);
    QFont font("consolas");
    font.setStyleHint(QFont::Monospace);
    app.setFont(font);
    app.setWindowIcon(QIcon(":/images/favicon.ico"));
    MainWindow window;
    window.show();
    return app.exec();
}
