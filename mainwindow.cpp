#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "utils.cpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    qDebug() << "MainWindow()";
    ui->setupUi(this);

    // attach button events to functions
    connect(ui->actionCopy_GPS_to_Clipboard, &QAction::triggered, this, &MainWindow::gps_clipboard );
    connect(ui->actionDraw_Markers, &QAction::triggered, this, &MainWindow::toggle_markers );
    connect(ui->actionExport_to_PNG, &QAction::triggered, this, &MainWindow::export_image );
    connect(ui->actionDraw_Lines, &QAction::triggered, this, &MainWindow::toggle_lines );
    connect(ui->pushButton, &QPushButton::released, this, &MainWindow::start_trace);

    ui->actionDraw_Lines->setCheckable(true);
    ui->actionDraw_Lines->setChecked(draw_lines);

    ui->actionDraw_Markers->setCheckable(true);
    ui->actionDraw_Markers->setChecked(draw_markers);

    manager = new QNetworkAccessManager();

    ip_regex = new QRegularExpression("(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})");

    geolocation_api_key = QProcessEnvironment::systemEnvironment().value("tr_api_key");

    origin = new Node();
    target = new Node();

    readyToTrace = false;
    ui->lineEdit->setDisabled(1);
    ui->pushButton->setDisabled(1);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete manager;
}

void MainWindow::fetch_origin()
{
    qDebug() << "fetch_origin()";
    ui->statusbar->showMessage("Fetcing my ip ...");

    request.setUrl(QUrl("https://api.ipify.org/?format=text"));
    QNetworkReply* reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, [=]() {
        if(reply->error() == QNetworkReply::NoError)
        {
            QByteArray response = reply->readAll();
            origin->ip = QString(response);
            ui->label->setText("My IP: " + origin->ip );

            readyToTrace = true;
            ui->lineEdit->setDisabled(0);
            ui->pushButton->setDisabled(0);
            ui->statusbar->showMessage("");

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
            //draw_complete_path();

        } else {
            qWarning() << "fetch_coordinates( " << node->position << " ) failed";
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

void MainWindow::draw_complete_path()
{
    draw_node(origin);
    foreach(Node* node, path)
    {
        draw_node(node);
    }
}

void MainWindow::cleanup_trace()
{
    ui->statusbar->showMessage("");
    readyToTrace = true;
    ui->pushButton->setDisabled(0);
    ui->lineEdit->setDisabled(0);
}

void MainWindow::clear_world()
{
    ui->label_2->setPixmap( QPixmap(":/images/world.jpg") );
}

int MainWindow::lon_to_x(QString lon, const int width) {
    return ((width/360.0) * (180 + lon.toDouble()));   }

int MainWindow::lat_to_y(QString lat, const int height) {
    return ((height/180.0) * (90 - lat.toDouble()));    }

void MainWindow::draw_node(Node* node)
{
    qDebug() << "draw_node()";
    QPixmap* pixmap = new QPixmap(ui->label_2->pixmap());

    QPainter painter(pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    node->x = lon_to_x(node->lon, pixmap->width());
    node->y = lat_to_y(node->lat, pixmap->height());

    if(node->x == 1024 && node->y == 512) return;

    if(node != origin && draw_lines) {

        Node* previous;
        if(node->position == 0) {
            previous = origin;
        } else {
            foreach(Node* prev, path) {
                if(prev->position == (node->position - 1))
                    previous = prev;
            }
        }

        qDebug() << "Line from " << node->position << " to " << previous->position;

        if(previous->x != 1024 && previous->y != 512) {
            painter.setPen( QPen(Qt::green, 4, Qt::DashDotLine, Qt::RoundCap) ); //QPen( Qt::green, 12, Qt::RightArrow, Qt::RoundCap ));
            painter.drawLine( previous->x, previous->y, node->x, node->y  );
        }
    }

    if(draw_markers) {
        const int s = 50;
        QPixmap* image = new QPixmap(":/images/marker.png");
        painter.drawPixmap( node->x-s/2, node->y-s, s, s, *image);
    }

    painter.end();
    ui->label_2->setPixmap(*pixmap);
}

/**
 * TODO: Add cancellation of trace.
 * TODO: Add visuals to ongoing trace.
 */
void MainWindow::start_trace()
{
    qDebug() << "start_trace()";

    target->ip = ui->lineEdit->text();

    if(target->ip.length() == 0) {
        return;
    }

    ui->statusbar->showMessage("Tracing ...");
    readyToTrace = false;
    ui->pushButton->setDisabled(1);
    ui->lineEdit->setDisabled(1);

    // reset world image
    path.clear();
    ui->label_2->setPixmap( QPixmap(":/images/world.jpg") );
    draw_node(origin);

    qDebug() << "Trace target: " << target->ip;

    trace(target);
}

// https://stackoverflow.com/questions/17338877/get-qprocess-output-in-slot
void MainWindow::trace(Node* node)
{
    qDebug() << "trace()";

    fetch_coordinates(target);

    QString command = "traceroute";

#ifdef Q_OS_WIN
    command = "tracert";
#endif

    qDebug() << "cmd: " << command;

    process = new QProcess(this);
    process->start(command, QStringList() << node->ip);
    process->waitForStarted();

    QObject::connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(handle_output()));
}

void MainWindow::handle_output()
{
    qDebug() << "handle_output()";
    QString data = (QString) process->readAllStandardOutput();

    // if includes "Trace complete" set finished
    if(data.contains("Trace complete")) {
        qDebug() << "Trace complete!";

        target->position = node_counter;
        path.emplace_back(target);
        node_counter++;

        draw_complete_path();
        cleanup_trace();
    }

    // extract IP adress (IPv4 for now)
    QRegularExpressionMatch match = ip_regex->match(data);
    if(match.hasMatch()) {
        QString ip = match.captured(1);
        if(!ip.contains(target->ip) && !ip.contains("192.168.0.")) {
            qDebug() << "Found ip: " << ip;

            Node* temp = new Node(ip);
            fetch_coordinates(temp);
            temp->position = node_counter;
            node_counter++;
            path.emplace_back(temp);
        }
    }
}

void MainWindow::export_image()
{
    qDebug() << "export_image()";
    QPixmap pixmap = ui->label_2->grab(QRect(QPoint(0, 0), QSize( ui->label_2->width(), ui->label_2->height()) ));

    QString fname = QFileDialog::getSaveFileName(nullptr, "Export image", ".", "Image (*.png *.jpg)" );

    if(!pixmap.toImage().save(fname))
        qCritical("Save Failed!");
}

void MainWindow::toggle_markers()
{
    qDebug() << "toggle_markers()";
    draw_markers = !draw_markers;
    clear_world();
    draw_complete_path();
    ui->actionDraw_Markers->setChecked(draw_markers);
}

void MainWindow::toggle_lines()
{
    qDebug() << "toggle_lines()";
    draw_lines = !draw_lines;
    clear_world();
    draw_complete_path();
    ui->actionDraw_Lines->setChecked(draw_lines);
}

void MainWindow::gps_clipboard()
{
    QString string;

    string += origin->lat + ',' + origin->lon +'\n';

    foreach(Node* node, path) {
        string += node->lat + ',' + node->lon +'\n';
    }

    QClipboard* clipboard = QApplication::clipboard();

    clipboard->setText(string, QClipboard::Clipboard);

    if (clipboard->supportsSelection()) {
        clipboard->setText(string, QClipboard::Selection);
    }

    #if defined(Q_OS_LINUX)
        QThread::msleep(1); //workaround for copied text not being available...
    #endif
}






