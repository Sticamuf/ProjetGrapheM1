#ifndef NODEMAP_H
#define NODEMAP_H
#include <vector>
extern std::vector<std::vector<bool>> mapPosNode;

// Map qui indique si une node ou un bend se trouve a ses coordonnées.
// mapPosNode[Y][X]
std::vector<std::vector<bool>> mapPosNode;
#endif