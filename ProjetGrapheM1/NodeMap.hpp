#ifndef NODEMAP_H
#define NODEMAP_H
#include <vector>
#include <set>
#include "NodeBend.hpp"
extern std::vector<std::vector<bool>> mapPosNode;
extern std::vector<NodeBend> vectorNodeBends;
extern std::set<NodeBend> setNodeBends;

// Map qui indique si une node ou un bend se trouve a ses coordonn�es.
// mapPosNode[Y][X]
std::vector<std::vector<bool>> mapPosNode;
// Vector qui contient la liste de tout les noeuds et bends d'un graphe
std::vector<NodeBend> vectorNodeBends;
//set qui contient les nodebend
std::set<NodeBend> setNodeBends;
#endif