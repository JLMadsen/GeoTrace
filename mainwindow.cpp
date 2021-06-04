#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "utils.cpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    qDebug() << "MainWindow()";
    ui->setupUi(this);
    origin = new Node();
    target = new Node();

    // attach button events to functions
    connect(ui->actionCopy_Traceroute_to_Clipboard, &QAction::triggered,        this, &MainWindow::tr_clipboard);
    connect(ui->actionCopy_GPS_to_Clipboard,        &QAction::triggered,        this, &MainWindow::gps_clipboard );
    connect(ui->actionCopy_X_Y_to_Clipboard,        &QAction::triggered,        this, &MainWindow::xy_clipboard );
    connect(ui->actionDraw_Markers,                 &QAction::triggered,        this, &MainWindow::toggle_markers );
    connect(ui->actionExport_to_PNG,                &QAction::triggered,        this, &MainWindow::export_image );
    connect(ui->actionDraw_arrows,                  &QAction::triggered,        this, &MainWindow::toggle_arrows );
    connect(ui->actionDraw_Lines,                   &QAction::triggered,        this, &MainWindow::toggle_lines );
    connect(ui->actionAdd_API_key,                  &QAction::triggered,        this, &MainWindow::add_api_key );
    connect(ui->pushButton,                         &QAbstractButton::released, this, &MainWindow::start_trace );
    connect(ui->toolButton,                         &QAbstractButton::released, this, &MainWindow::stop_trace );

    ui->actionDraw_Lines->setCheckable(true);
    ui->actionDraw_Lines->setChecked(draw_lines);

    ui->actionDraw_Markers->setCheckable(true);
    ui->actionDraw_Markers->setChecked(draw_markers);

    ui->actionDraw_arrows->setCheckable(true);
    ui->actionDraw_arrows->setChecked(draw_arrows);

    ui->statusbar->setStyleSheet("color: gray");

    // fix placeholder color
    connect(ui->lineEdit, &QLineEdit::textChanged, [=]
    {
        if(ui->lineEdit->text().isEmpty())
        {
            ui->lineEdit->setStyleSheet("color: gray;");
        } else
        {
            ui->lineEdit->setStyleSheet("color: black;");
        }
    });

    manager = new QNetworkAccessManager();
    ip_regex = new QRegularExpression("(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})");
    geolocation_api_key = QProcessEnvironment::systemEnvironment().value("tr_api_key");

    set_trace_status(false);
    ui->toolButton->setVisible(0);

    if(geolocation_api_key.isEmpty())
    {
        ui->statusbar->showMessage("Missing API key");
        add_api_key();
    } else
    {
        fetch_origin();
    }

    orange_marker = new QPixmap(":/resources/resources/marker.png");
    green_marker  = new QPixmap(":/resources/resources/marker_green.png");
    purple_marker = new QPixmap(":/resources/resources/marker_magenta.png");
}

MainWindow::~MainWindow()
{
    delete ui;
    delete manager;
    delete selected;
    delete origin;
    delete target;
    delete ip_regex;
    delete process;
    delete green_marker;
    delete orange_marker;
    delete purple_marker;

    foreach(Node* n, path)
        delete n;

    process->close();
}

void MainWindow::fetch_origin()
{
    qDebug() << "fetch_origin()";
    ui->statusbar->showMessage("Fetcing my ip ...");

    request.setUrl(QUrl("https://api.ipify.org/?format=text"));
    QNetworkReply* reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, [=]()
    {
        if(reply->error() == QNetworkReply::NoError)
        {
            QByteArray response = reply->readAll();
            origin->ip = QString(response);
            ui->label->setText("My IP: " + origin->ip );

            set_trace_status(true);
            ui->statusbar->showMessage("");

            fetch_coordinates(origin);

        } else
        {
            ui->label->setText("Could not get IP");
            ui->label->setStyleSheet("QLabel { color : red; }");
            ui->statusbar->showMessage(reply->errorString());
            qWarning() << "fetch_origin() failed";
        }
    });
}

void MainWindow::fetch_coordinates(Node* node) {
    qDebug() << "fetch_coordinates(" << node->ip <<")";

    if( QHostAddress(node->ip).isNull() )
    {
        request.setUrl(QUrl("https://api.ipgeolocation.io/ipgeo?apiKey="+ geolocation_api_key +"&dns="+ node->ip +"&fields=geo"));
    } else
    {
        request.setUrl(QUrl("https://api.ipgeolocation.io/ipgeo?apiKey="+ geolocation_api_key +"&ip="+ node->ip +"&fields=geo"));
    }

    QNetworkReply* reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, [=]()
    {

        if(reply->error() == QNetworkReply::NoError)
        {
            QJsonObject obj = QJsonDocument::fromJson( QString(reply->readAll()).toUtf8() ).object();

            node->lat = obj["latitude"].toString();
            node->lon = obj["longitude"].toString();

            draw_listview();
            draw_node(node);

        } else
        {
            qWarning() << "fetch_coordinates( " << node->position << " ) failed";

            path.removeOne(node);

            foreach(Node* n, path)
                if(n->position > node->position)
                    n->position--;

            node_counter--;
            node->dead = true;
        }
    });
}

void MainWindow::draw_listview()
{
    qDebug() << "draw_listview()";
    ui->listWidget->clear();

    ui->listWidget->addItem(origin->ip);

    if(draw_coordinates)
        ui->listWidget->addItem(("("+ origin->lat +", "+ origin->lon +")"));

    foreach(Node* node, path)
    {
        ui->listWidget->addItem(node->ip);

        if(draw_coordinates)
            ui->listWidget->addItem(("("+ node->lat +", "+ node->lon +")"));
    }
}

void MainWindow::draw_complete_path()
{
    draw_node(origin);
    foreach(Node* node, path)
        draw_node(node);
}

void MainWindow::cleanup_trace()
{
    ui->statusbar->showMessage("");
    set_trace_status(true);
    ui->toolButton->setVisible(0);
}

void MainWindow::set_trace_status(bool ready)
{
    ui->lineEdit->setDisabled( !ready );
    ui->pushButton->setDisabled( !ready );
}

void MainWindow::clear_world()
{
    ui->label_2->setPixmap( QPixmap(":/resources/resources/world.jpg") );
}

int MainWindow::lon_to_x(QString lon, const int width) {
    return ((width/360.0) * (180 + lon.toDouble()));   }

int MainWindow::lat_to_y(QString lat, const int height) {
    return ((height/180.0) * (90 - lat.toDouble()));    }

void MainWindow::draw_node(Node* node)
{
    qDebug() << "draw_node()";
    if(node->dead)
        return;

    QPixmap* pixmap = new QPixmap(ui->label_2->pixmap());

    QPainter painter(pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    node->x = lon_to_x(node->lon, pixmap->width());
    node->y = lat_to_y(node->lat, pixmap->height());

    if(node->x == 1024 && node->y == 512) return;

    if(node != origin && draw_lines) {

        Node* previous;
        if(node->position == 0)
        {
            previous = origin;

        } else
        {
            foreach(Node* prev, path)
                if(prev->position == (node->position - 1))
                    previous = prev;
        }
        if( (previous->x != 1024 && previous->y != 512) || previous->dead)
        {
            qDebug() << "Line from " << previous->x <<","<< previous->y <<" -> "<< node->x<<","<<node->y;
            painter.setPen( QPen(Qt::green, 4, Qt::DashDotLine, Qt::RoundCap) );

            if(draw_arrows) {
                Util::DrawLineWithArrow(painter, QPoint(previous->x, previous->y), QPoint(node->x, node->y));
            } else {
                painter.drawLine( previous->x, previous->y, node->x, node->y  );
            }

        }
    }

    if(draw_markers) {
        const int s = 50;
        QPixmap* image;

        image = orange_marker;

        painter.drawPixmap( node->x-s/2, node->y-s, s, s, *image);
    }

    painter.end();
    ui->label_2->setPixmap(*pixmap);
}

/**
 * TODO: Add cancellation of trace.
 * TODO: Add visuals to ongoing trace.
 * TODO: Check if input is valid IP or Domain
 */
void MainWindow::start_trace()
{
    qDebug() << "start_trace()";

    target = new Node;
    target->ip = ui->lineEdit->text();

    if(target->ip.length() == 0)
    {
        return;
    }

    path.clear();
    node_counter = 0;

    ui->listWidget->clear();
    draw_listview();

    ui->label_2->setPixmap( QPixmap(":/resources/resources/world.jpg") );
    draw_node(origin);

    raw_traceroute = "";

    ui->statusbar->showMessage("Tracing ...");
    set_trace_status(false);
    ui->toolButton->setVisible(1); 

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

void MainWindow::stop_trace()
{
    qDebug() << "stop_trace()";
    process->close();
    cleanup_trace();
}

// TODO: Check if trace failed
void MainWindow::handle_output()
{
    qDebug() << "handle_output()";

    QString data = (QString) process->readAllStandardOutput();

    if(data.length() > 5)
        ui->statusbar->showMessage(data);

    raw_traceroute += data + "\n";

    // if includes "Trace complete" set finished
    if(data.contains("Trace complete"))
    {
        qDebug() << "Trace complete !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";

        target->position = node_counter;
        path.emplace_back(target);
        node_counter++;

        draw_complete_path();
        cleanup_trace();
        return;
    }

    // extract IP adress (IPv4 for now)
    QRegularExpressionMatch match = ip_regex->match(data);
    if(match.hasMatch())
    {
        QString ip = match.captured(1);
        if(!ip.contains(target->ip) && !ip.contains("192.168."))
        {
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
    ui->actionDraw_arrows->setDisabled(!draw_lines);
}

void MainWindow::toggle_arrows()
{
    qDebug() << "toggle_arrows()";
    draw_arrows = !draw_arrows;
    clear_world();
    draw_complete_path();
    ui->actionDraw_arrows->setChecked(draw_arrows);
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
    if (clipboard->supportsSelection())
    {
        clipboard->setText(string, QClipboard::Selection);
    }

    #if defined(Q_OS_LINUX)
        QThread::msleep(1); //workaround for copied text not being available...
    #endif
}

void MainWindow::xy_clipboard()
{
    QString string;
    string += QString::number(origin->x) + ',' + QString::number(origin->y) +'\n';
    foreach(Node* node, path)
    {
        string += QString::number(node->x) + ',' + QString::number(node->y) +'\n';
    }

    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(string, QClipboard::Clipboard);
    if (clipboard->supportsSelection())
    {
        clipboard->setText(string, QClipboard::Selection);
    }

    #if defined(Q_OS_LINUX)
        QThread::msleep(1); //workaround for copied text not being available...
    #endif
}

void MainWindow::tr_clipboard()
{
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(raw_traceroute, QClipboard::Clipboard);
    if (clipboard->supportsSelection())
    {
        clipboard->setText(raw_traceroute, QClipboard::Selection);
    }

    #if defined(Q_OS_LINUX)
        QThread::msleep(1); //workaround for copied text not being available...
    #endif
}

void MainWindow::add_api_key()
{
    qDebug() << "add_api_key()";

    bool ok;
    QString input = QInputDialog::getText(
                0,
                "Add API key",
                "API key:",
                QLineEdit::Normal,
                "",
                &ok
                );

    if(ok && !input.isEmpty())
    {
        qDebug() << "Dialog input: " << input;

        // did not work
        // QProcessEnvironment::systemEnvironment().insert("tr_api_key", input);

        // temporary fix
        geolocation_api_key = input;
        fetch_origin();
    }
}
