#ifndef READFROMJSON_H
#define READFROMJSON_H

#include <string>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/fileformats/GraphIO.h>
#include <nlohmann/json.hpp>

#include <ogdf/basic/heap/BinaryHeap.h>
#include "calcEdgeLength.h"

using std::string;
using nlohmann::json;
using namespace ogdf;

void readFromJson(string input, Graph& G, GraphAttributes& GA, int& gridWidth, int& gridHeight, int& maxBends);
#endif