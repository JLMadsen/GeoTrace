#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QObject>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <QProcessEnvironment>
#include <QProcess>

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>

#include <QPainter>
#include <QPixmap>

//#include <QGeoRoute>

#include <regex>
#include <string>
#include <string.h>
#include <vector>
#include <iostream>

#include "node.cpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void start_trace();
    void fetch_origin();
    void fetch_coordinates(Node* node);
    void draw_listview();
    void trace(Node *node);
    void draw_node(Node* node);
    int lon_to_x(QString lon, const int width);
    int lat_to_y(QString lat, const int height);

    bool readyToTrace = false;

    QString geolocation_api_key;

    Node* origin;
    Node* target;
    std::vector<Node*> path;

    /*QString origin;
    QString origin_coordinates[2];
    QStringList ip_path;
    QStringList ip_path_coordinates;*/

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager *manager;
    QNetworkRequest request;

};

#endif // MAINWINDOW_H
