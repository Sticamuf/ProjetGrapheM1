#include <ogdf/basic/GraphAttributes.h>

using ogdf::edge;
using ogdf::GraphAttributes;
using ogdf::node;
using ogdf::DPolyline;
using ogdf::ListIterator;
using ogdf::DPoint;

// Calculer la longueur d'un edge
double calcEdgeLength(edge& e, const GraphAttributes& GA) {
    node source = e->source();
    node target = e->target();
    double length = 0.0;
    double sourceX = GA.x(source);
    double sourceY = GA.y(source);
    double targetX, targetY;
    DPolyline bends = GA.bends(e);
    if (bends.size() > 0) {
        int l = 0;
        for (ListIterator<DPoint> i = bends.begin(); i.valid(); i++) {
            targetX = (*i).m_x;
            targetY = (*i).m_y;
            length += sqrt(pow((targetX - sourceX), 2) + pow((targetY - sourceY), 2));
            sourceX = targetX;
            sourceY = targetY;
            l++;
        }
    }
    targetX = GA.x(target);
    targetY = GA.y(target);
    length += sqrt(pow((targetX - sourceX), 2) + pow((targetY - sourceY), 2));
    return length;
}