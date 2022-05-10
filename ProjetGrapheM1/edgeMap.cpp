#include "edgeMap.hpp"

std::map<ogdf::edge, double> mapEdgeLength;
std::map<double, std::set<ogdf::edge>> mapLengthEdgeSet;