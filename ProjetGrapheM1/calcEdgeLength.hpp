#ifndef CALCEDGELENGTH_HPP
#define CALCEDGELENGTH_HPP
#include <ogdf/basic/GridLayout.h>
#include "NodeBend.hpp"

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
            length += sqrt(pow((targetX - sourceX), 2) + pow((targetY - sourceY), 2));
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
    length += sqrt(pow((targetX - sourceX), 2) + pow((targetY - sourceY), 2));
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

unsigned long long calcTmpEdgeLengthSquared(const edge& e, int srcX, int srcY, const GridLayout& GL) {
    node target = e->target();
    unsigned long long length = 0.0;
    int sourceX = srcX;
    int sourceY = srcY;
    int targetX, targetY;
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

unsigned long long calcTmpEdgeLength(const edge& e, int srcX, int srcY, const GridLayout& GL) {
    node target = e->target();
    unsigned long long length = 0.0;
    int sourceX = srcX;
    int sourceY = srcY;
    int targetX, targetY;
    IPolyline bends = GL.bends(e);
    if (bends.size() > 0) {
        for (ListIterator<IPoint> i = bends.begin(); i.valid(); i++) {
            targetX = (*i).m_x;
            targetY = (*i).m_y;
            length += sqrt(pow((targetX - sourceX), 2) + pow((targetY - sourceY), 2));
            sourceX = targetX;
            sourceY = targetY;
        }
    }
    targetX = GL.x(target);
    targetY = GL.y(target);
    length += sqrt(pow((targetX - sourceX), 2) + pow((targetY - sourceY), 2));
    return length;
}

unsigned long long calcTmpEdgeLengthBends(const edge& e, NodeBend n, int bendX, int bendY, const GridLayout& GL) {
    node source = e->source();
    node target = e->target();
    unsigned long long length = 0.0;
    int sourceX = GL.x(source);
    int sourceY = GL.y(source);
    int targetX, targetY;
    IPolyline bends = GL.bends(e);
    if (bends.size() > 0) {
        int k = 0;
        for (ListIterator<IPoint> i = bends.begin(); i.valid(); i++, k++) {
            if (k == n.numero) {
                targetX = bendX;
                targetY = bendY;
            }
            else {
                targetX = (*i).m_x;
                targetY = (*i).m_y;
            }
            length += sqrt(pow((targetX - sourceX), 2) + pow((targetY - sourceY), 2));
            sourceX = targetX;
            sourceY = targetY;
        }
    }
    targetX = GL.x(target);
    targetY = GL.y(target);
    length += sqrt(pow((targetX - sourceX), 2) + pow((targetY - sourceY), 2));
    return length;
}
#endif