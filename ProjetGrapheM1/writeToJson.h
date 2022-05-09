#include <string>
#include <nlohmann/json.hpp>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/fileformats/GraphIO.h>

using std::string;
using nlohmann::json;
using namespace ogdf;

// ----- ECRITURE D'UN Graph DANS UN FICHIER JSON -----
void writeToJson(string output, const Graph& G, const GraphAttributes& GA, int gridWidth, int gridHeight, int maxBends);