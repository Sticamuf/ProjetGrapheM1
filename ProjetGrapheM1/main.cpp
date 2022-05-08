#include <ogdf/basic/GraphAttributes.h>

#include "writeToJson.cpp"
#include "readFromJson.cpp"
#include "dispOpenGL.cpp"

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
    srand(time(NULL));

    //debut opengl
    dispOpenGL(G,GA,gridWidth,gridHeight);
    //fin opengl
    return 0;
}