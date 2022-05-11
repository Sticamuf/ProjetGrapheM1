#ifndef CALCEDGELENGTH_HPP
#define CALCEDGELENGTH_HPP
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/basic/EdgeArray.h> //non necessaire car GraphAttributes inclut EdgeArray

using namespace ogdf;

double calcEdgeLength(const edge& e, const GraphAttributes& GA) {
    node source = e->source();
    node target = e->target();
    double length = 0.0;
    double sourceX = GA.x(source);
    double sourceY = GA.y(source);
    double targetX, targetY;
    DPolyline bends = GA.bends(e);
    if (bends.size() > 0) {
        for (ListIterator<DPoint> i = bends.begin(); i.valid(); i++) {
            targetX = (*i).m_x;
            targetY = (*i).m_y;
            //alternantive ???
            //if (sourceX == targetX) { length += abs(targetY - sourceY); }
            //else if (sourceY == sourceX) { length += abs(targetX - sourceX); }
            //else
            length += sqrt(pow((targetX - sourceX), 2) + pow((targetY - sourceY), 2));
            sourceX = targetX;
            sourceY = targetY;
        }
    }
    targetX = GA.x(target);
    targetY = GA.y(target);
    //alternantive ???
    //if (sourceX == targetX) { length += abs(targetY - sourceY); }
    //else if (sourceY == sourceX) { length += abs(targetX - sourceX); }
    //else
    length += sqrt(pow((targetX - sourceX), 2) + pow((targetY - sourceY), 2));
    return length;
}

unsigned long long calcEdgeLengthSquared(const edge& e, const GraphAttributes& GA) {
    node source = e->source();
    node target = e->target();
    unsigned long long length = 0.0;
    double sourceX = GA.x(source);
    double sourceY = GA.y(source);
    double targetX, targetY;
    DPolyline bends = GA.bends(e);
    if (bends.size() > 0) {
        for (ListIterator<DPoint> i = bends.begin(); i.valid(); i++) {
            targetX = (*i).m_x;
            targetY = (*i).m_y;
            length += pow((targetX - sourceX), 2) + pow((targetY - sourceY), 2);
            sourceX = targetX;
            sourceY = targetY;
        }
    }
    targetX = GA.x(target);
    targetY = GA.y(target);
    length += pow((targetX - sourceX), 2) + pow((targetY - sourceY), 2);
    return length;
}

#endif