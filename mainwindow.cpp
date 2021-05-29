#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "utils.cpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    qDebug() << "MainWindow()";
    ui->setupUi(this);
    readyToTrace = false;

    // attach button event to function
    connect(ui->pushButton, &QPushButton::released, this, &MainWindow::start_trace);

    manager = new QNetworkAccessManager();

    geolocation_api_key = QProcessEnvironment::systemEnvironment().value("tr_api_key");
    // qDebug() << geolocation_api_key;

    origin = new Node();
    target = new Node();

    origin->position = -1;
    ui->lineEdit->setDisabled(1);
    ui->pushButton->setDisabled(1);

    // not relevant, pixmap alters resolution
    // qDebug() << "image size: " << ui->label_2->height() << "x" << ui->label_2->width();

}

MainWindow::~MainWindow()
{
    delete ui;
    delete manager;
}

void MainWindow::fetch_origin()
{
    qDebug() << "fetch_origin()";

    request.setUrl(QUrl("https://api.ipify.org/?format=text"));
    QNetworkReply* reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, [=]() {
        if(reply->error() == QNetworkReply::NoError)
        {
            QByteArray response = reply->readAll();
            origin->ip = QString(response);
            ui->label->setText("My IP: " + origin->ip );
            ui->label->setStyleSheet("QLabel { color : green; }");

            readyToTrace = true;
            ui->lineEdit->setDisabled(0);
            ui->pushButton->setDisabled(0);

            fetch_coordinates(origin);

        } else {
            ui->label->setText("Could not get IP");
            ui->label->setStyleSheet("QLabel { color : red; }");
            qWarning() << "fetch_origin() failed";
        }
    });
}

void MainWindow::fetch_coordinates(Node* node) {
    qDebug() << "fetch_coordinates(" << node->ip <<")";

    request.setUrl(QUrl("https://api.ipgeolocation.io/ipgeo?apiKey="+ geolocation_api_key +"&ip="+ node->ip +"&fields=geo"));
    QNetworkReply* reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, [=]() {

        if(reply->error() == QNetworkReply::NoError)
        {
            QJsonObject obj = QJsonDocument::fromJson( QString(reply->readAll()).toUtf8() ).object();

            // keep as string as Qt struggles with toString from double.
            node->lat = obj["latitude"].toString();//.toDouble();
            node->lon = obj["longitude"].toString();//.toDouble();

            draw_listview();
            draw_node(node);

        } else {
            qWarning() << "fetch_coordinates() failed";
            // err
        }
    });
}

void MainWindow::draw_listview()
{
    qDebug() << "draw_listview()";
    ui->listWidget->clear();

    ui->listWidget->addItem(origin->ip);
    ui->listWidget->addItem(("("+ origin->lat +", "+ origin->lon +")"));

    foreach(Node* node, path)
    {
        ui->listWidget->addItem(node->ip);
        ui->listWidget->addItem(("("+ node->lat +", "+ node->lon +")"));
    }
}

/**
 * TODO: Add cancellation of trace.
 * TODO: Add visuals to ongoing trace.
 */
void MainWindow::start_trace()
{
    qDebug() << "start_trace()";

    // reset world image
    path.clear();
    ui->label_2->setPixmap( QPixmap(":/images/world.jpg") );
    draw_node(origin);

    target->ip = ui->lineEdit->text();

    if(target->ip.length() == 0) {
        return;
    }

    qDebug() << "Trace target: " << target->ip;
    //trace(target);

    // test data
    QStringList ip_list;
    ip_list << "1.1.1.1";
    ip_list << "2.2.2.2";
    ip_list << "3.3.3.3";
    ip_list << "4.4.4.4";

    int counter = 0;
    foreach(QString ip, ip_list)
    {
        Node* temp = new Node(ip);
        fetch_coordinates(temp);
        temp->position = counter;
        counter++;
        path.emplace_back(temp);
    }
}

int MainWindow::lon_to_x(QString lon, const int width) {
    return ((width/360.0) * (180 + lon.toDouble()));   }

int MainWindow::lat_to_y(QString lat, const int height) {
    return ((height/180.0) * (90 - lat.toDouble()));    }

void MainWindow::draw_node(Node* node)
{
    qDebug() << "draw_node()";
    QPixmap* pixmap = new QPixmap(ui->label_2->pixmap());
    QPixmap* image = new QPixmap(":/images/marker.png");

    QPainter painter(pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    node->x = lon_to_x(node->lon, pixmap->width());
    node->y = lat_to_y(node->lat, pixmap->height());

    //qDebug() << "lon_to_x(" << node->lon << ") = " << x;
    //qDebug() << "lat_to_y(" << node->lat << ") = " << y;

    const int s = 50;

    if(node->position != -1) {

        Node* previous;
        if(node->position == 0) {
            previous = origin;
        } else {
            previous = path[ node->position - 1 ];
        }

        qDebug() << "Line from " << node->position << " to " << previous->position;

        painter.setPen( QPen(Qt::green, 4, Qt::DashDotLine, Qt::RoundCap) ); //QPen( Qt::green, 12, Qt::RightArrow, Qt::RoundCap ));
        painter.drawLine( previous->x, previous->y, node->x, node->y  );
    }

    painter.drawPixmap( node->x-s/2, node->y-s, s, s, *image);
    painter.end();
    ui->label_2->setPixmap(*pixmap);
}

void MainWindow::trace(Node* node)
{
    qDebug() << "trace()";
    QString command = "traceroute";

#ifdef _WIN32
    command = "tracert";
#endif

    QProcess* process = new QProcess(this);

    qDebug() << "cmd: " << command;

    //process->start("sh", QStringList << command +" "+ node->ip);
    //process->start("sh "+ command +" "+ node->ip);
    process->start(command +" "+ node->ip);


    process->waitForFinished(-1);

    QString out = process->readAllStandardOutput();
    QString err = process->readAllStandardError();

    QString all = process->readAll();

    process->close();

    qDebug() << "Traceroute stdout: " << out;
    qDebug() << "Traceroute stderr: " << err;
    qDebug() << "Traceroute all   : " << all;



    //QObject::connect(process, SIGNAL(process.readyReadStandard()), [=]() {
    //});


}
