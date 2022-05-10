#ifndef EDGEMAP_H
#define EDGEMAP_H

#include<map>
#include <set>
#include <ogdf/basic/EdgeArray.h>

extern std::map<ogdf::edge, double> mapEdgeLength;
extern std::map<double, std::set<ogdf::edge>> mapLengthEdgeSet;

#endif