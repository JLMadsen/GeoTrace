
#include <string.h>

#include <QStringList>
#include <QString>
#include <QProcess>
#include <QDebug>
#include <QObject>
#include <QPainter>

#include "node.cpp"

#pragma once

namespace Util {

char* concat_const_char(const char* word1, const char* word2)
{
    // Not safe, consider fixing.
    char * s = new char[strlen(word1) + strlen(word2) + 1];
    strcpy(s, word1);
    strcat(s, word2);
    return s;
}

// exec traceroute
/*
 * https://stackoverflow.com/questions/19409940/how-to-get-output-system-command-in-qt
 * https://stackoverflow.com/questions/3227767/how-to-run-a-system-command-in-qt
 *
 * */

QStringList exec_traceroute(Node* target)
{
    QStringList retval;
    QString command = "tracepath";

#ifdef _WIN32
    command = "tracert";
#endif

    QProcess process;
    process.start(command + " " + target->ip);

    //process.waitForFinished();

    /*QObject::connect(&process, SIGNAL(finished(int, QProcess::ExitStatus)), [=, &process]() {

        QString out = process.readAllStandardOutput();
        QString err = process.readAllStandardError();

        qDebug() << "Traceroute stdout: " << out;
        qDebug() << "Traceroute stderr: " << err;

        //retval.append(out);
        //return retval;
    });*/


    /**/
    return retval;
}

void DrawLineWithArrow(QPainter& painter, QPoint start, QPoint end) {

  painter.setRenderHint(QPainter::Antialiasing, true);

  qreal arrowSize = 20; // size of head
  painter.setPen(Qt::green);
  painter.setBrush(Qt::green);

  QLineF line(end, start);

  double angle = std::atan2(-line.dy(), line.dx());
  QPointF arrowP1 = line.p1() + QPointF(sin(angle + M_PI / 3) * arrowSize,
                                        cos(angle + M_PI / 3) * arrowSize);
  QPointF arrowP2 = line.p1() + QPointF(sin(angle + M_PI - M_PI / 3) * arrowSize,
                                        cos(angle + M_PI - M_PI / 3) * arrowSize);

  QPolygonF arrowHead;
  arrowHead.clear();
  arrowHead << line.p1() << arrowP1 << arrowP2;
  painter.drawLine(line);
  painter.drawPolygon(arrowHead);

}

}
