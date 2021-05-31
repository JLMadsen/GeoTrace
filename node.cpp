#include <QString>

#pragma once

struct Node {

    Node() {};
    Node(QString ip_) : ip(ip_) {}

    mutable QString ip;
    mutable QString lat;
    mutable QString lon;

    mutable int position;
    mutable int x;
    mutable int y;

    mutable bool dead = false;
};



