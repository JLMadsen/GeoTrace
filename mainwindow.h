#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QObject>
#include <QAction>
#include <QFile>
#include <QFileDialog>
#include <QStandardPaths>
#include <QVector>

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

#include <QRegularExpression>
#include <QRegularExpressionMatch>

//#include <QGeoRoute>

#include <regex>
#include <string>
#include <string.h>
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

    // setup
    void fetch_origin();
    void fetch_coordinates(Node* node);

    // main traceroute functionality
    void start_trace();
    void trace(Node *node);
    void cleanup_trace();

    // menu options
    void export_image();
    void toggle_markers();
    void toggle_lines();

    // visuals
    void draw_listview();
    void draw_node(Node* node);
    void draw_complete_path();
    void clear_world();

    // math
    int lon_to_x(QString lon, const int width);
    int lat_to_y(QString lat, const int height);

    // member values
    bool readyToTrace = false;
    bool draw_markers = true;
    bool draw_lines = true;
    int node_counter;

    // api key, get your own.
    QString geolocation_api_key;

    // traceroute process
    QProcess* process;

    Node* origin;
    Node* target; // temp node
    QVector<Node*> path;

    // regex
    QRegularExpression* ip_regex;;

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager *manager;
    QNetworkRequest request;

private slots:
    void handle_output();

};

#endif // MAINWINDOW_H
