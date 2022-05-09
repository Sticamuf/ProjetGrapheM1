#include "writeToJson.h"
// ----- ECRITURE D'UN Graph DANS UN FICHIER JSON -----
void writeToJson(string output, const Graph& G, const GraphAttributes& GA, int gridWidth, int gridHeight, int maxBends) {
    json j2;
    j2["width"] = gridWidth;
    j2["height"] = gridHeight;
    j2["bends"] = maxBends;

    node n = G.firstNode();
    int m = 0;
    while (n != nullptr) {
        j2["nodes"][m]["id"] = n->index();
        j2["nodes"][m]["x"] = (int)GA.x(n);
        j2["nodes"][m]["y"] = (int)GA.y(n);
        n = n->succ();
        m++;
    }

    edge e = G.firstEdge();
    m = 0;
    while (e != nullptr) {
        j2["edges"][m]["source"] = e->source()->index();
        j2["edges"][m]["target"] = e->target()->index();
        DPolyline bends = GA.bends(e);
        if (bends.size() > 0) {
            int l = 0;
            for (ListIterator<DPoint> i = bends.begin(); i.valid(); i++) {
                j2["edges"][m]["bends"][l]["x"] = (int)(*i).m_x;
                j2["edges"][m]["bends"][l]["y"] = (int)(*i).m_y;
                l++;
            }
        }
        e = e->succ();
        m++;
    }

    std::ofstream o(output);
    o << std::setw(4) << j2 << std::endl;
}