#include <algorithm>
#include <iostream>
#include <chrono>
#include "TrafficObject.h"

// init static variable
int TrafficObject::_idCnt = 0;

std::mutex TrafficObject::_mtx;

void TrafficObject::setPosition(double x, double y)
{
    _posX = x;
    _posY = y;
}

void TrafficObject::getPosition(double &x, double &y)
{
    x = _posX;
    y = _posY;
}

TrafficObject::TrafficObject()
{
    _type = ObjectType::noObject;
    _id = _idCnt++;
}

TrafficObject::~TrafficObject() {

    // replacing the below with a simpler and more up-to-date C++ syntax
//    std::for_each(threads.begin(), threads.end(), [](std::thread &t) {
//        t.join();
//    });

    // set up thread barrier before this object is destroyed
    for (auto &t: threads) {
        t.join();
    }
}
