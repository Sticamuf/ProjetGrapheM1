#include <ogdf/basic/GraphAttributes.h>

#include "writeToJson.h"
#include "readFromJson.h"
#include "dispOpenGL.h"

using ogdf::Graph;
using ogdf::GraphAttributes;

int main() {

    Graph G;
    GraphAttributes GA{G};
    int gridWidth, gridHeight, maxBends;

    // ----- LECTURE D'UN FICHIER JSON DANS UN Graph -----
    string file = "test.json";
    readFromJson(file, G, GA, gridWidth, gridHeight, maxBends);
    writeToJson("output.json", G, GA, gridWidth, gridHeight, maxBends);

    // Affichage openGL
    srand(static_cast<unsigned int>(time(NULL)));

    //debut opengl
    //dispOpenGL(G,GA,gridWidth,gridHeight);
    //fin opengl
    return 0;
}