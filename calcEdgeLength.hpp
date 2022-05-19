#ifndef CALCEDGELENGTH_HPP
#define CALCEDGELENGTH_HPP
#include <ogdf/basic/GridLayout.h>

using namespace ogdf;

double calcEdgeLength(const edge& e, const GridLayout& GL) {
    node source = e->source();
    node target = e->target();
    double length = 0.0;
    double sourceX = GL.x(source);
    double sourceY = GL.y(source);
    double targetX, targetY;
    IPolyline bends = GL.bends(e);
    if (bends.size() > 0) {
        for (ListIterator<IPoint> i = bends.begin(); i.valid(); i++) {
            targetX = (*i).m_x;
            targetY = (*i).m_y;
            //alternantive ???
            //if (sourceX == targetX) { length += abs(targetY - sourceY); }
            //else if (sourceY == sourceX) { length += abs(targetX - sourceX); }
            //else
            length += pow((targetX - sourceX), 2) + pow((targetY - sourceY), 2);
            sourceX = targetX;
            sourceY = targetY;
        }
    }
    targetX = GL.x(target);
    targetY = GL.y(target);
    //alternantive ???
    //if (sourceX == targetX) { length += abs(targetY - sourceY); }
    //else if (sourceY == sourceX) { length += abs(targetX - sourceX); }
    //else
    length += pow((targetX - sourceX), 2) + pow((targetY - sourceY), 2);
    return length;
}

unsigned long long calcEdgeLengthSquared(const edge& e, const GridLayout& GL) {
    node source = e->source();
    node target = e->target();
    unsigned long long length = 0.0;
    double sourceX = GL.x(source);
    double sourceY = GL.y(source);
    double targetX, targetY;
    IPolyline bends = GL.bends(e);
    if (bends.size() > 0) {
        for (ListIterator<IPoint> i = bends.begin(); i.valid(); i++) {
            targetX = (*i).m_x;
            targetY = (*i).m_y;
            length += pow((targetX - sourceX), 2) + pow((targetY - sourceY), 2);
            sourceX = targetX;
            sourceY = targetY;
        }
    }
    targetX = GL.x(target);
    targetY = GL.y(target);
    length += pow((targetX - sourceX), 2) + pow((targetY - sourceY), 2);
    return length;
}
#endif