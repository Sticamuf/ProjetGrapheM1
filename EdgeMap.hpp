#ifndef EDGEMAP_H
#define EDGEMAP_H
#include <map>
#include <set>
#include <ogdf/basic/EdgeArray.h>
using ogdf::edge;
extern std::map<edge, double> mapEdgeLength;
extern std::map<double, std::set<edge>> mapLengthEdgeSet;

std::map<edge, double> mapEdgeLength;
std::map<double, std::set<edge>> mapLengthEdgeSet;
#endif