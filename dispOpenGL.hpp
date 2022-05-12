#ifndef DISPOPENGL_H
#define DISPOPENGL_H

#include <Windows.h>
#include <gl/GL.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/basic/CombinatorialEmbedding.h>
#include <ogdf/basic/Layout.h>
#include "calcEdgeLength.hpp"
#include "EdgeMap.hpp"

using namespace ogdf;

bool move_randomly = false;
edge selectedEdge;
node selectedNode;
adjEntry selectedAdj;
bool show_grid_size = true;
std::set<face> setFace;
std::set<edge> setEdge;
bool showAllEdges = false;
ConstCombinatorialEmbedding CCE;

void error_callback(int error, const char* description) {
    fputs(description, stderr);
}

// Callback pour OpenGL
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS)
        switch (key) {
            // Fermer l'application
            case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
                // Action deplacer un noeud aleatoirement?
                // TOUCHE Q SUR UN CLAVIER AZERTY!
            case GLFW_KEY_A:
                move_randomly = !move_randomly;
                break;
            case GLFW_KEY_D:
                if (selectedEdge->pred() != nullptr)
                    selectedEdge = selectedEdge->pred();
                break;
            case GLFW_KEY_F:
                if (selectedEdge->succ() != nullptr)
                    selectedEdge = selectedEdge->succ();
                break;
            case GLFW_KEY_C:
                if (selectedNode->pred() != nullptr)
                    selectedNode = selectedNode->pred();
                break;
            case GLFW_KEY_V:
                if (selectedNode->succ() != nullptr)
                    selectedNode = selectedNode->succ();
                break;
            case GLFW_KEY_G:
                selectedAdj = selectedNode->firstAdj();
                selectedEdge = selectedAdj->theEdge();
                selectedNode = selectedAdj->theNode();
                break;
            case GLFW_KEY_T:
                selectedAdj = selectedAdj->twin();
                selectedEdge = selectedAdj->theEdge();
                selectedNode = selectedAdj->theNode();
                break;
            case GLFW_KEY_H:
                if (selectedAdj != nullptr)
                    selectedEdge = selectedAdj->theEdge();
                break;
            case GLFW_KEY_J:
                if (selectedAdj != nullptr)
                    if (selectedAdj->pred() != nullptr) {
                        selectedAdj = selectedAdj->pred();
                        selectedEdge = selectedAdj->theEdge();
                        selectedNode = selectedAdj->theNode();
                    }
                break;
            case GLFW_KEY_R:
                show_grid_size = !show_grid_size;
                break;
            case GLFW_KEY_P:
                if (selectedNode != nullptr) {
                    setFace.clear();
                    setEdge.clear();
                    SListPure<edge> edges;
                    selectedNode->adjEdges(edges);
                    for (SListConstIterator<edge> i = edges.begin(); i.valid(); i++) {
                        edge e = (*i);
                        setFace.insert(CCE.rightFace(e->adjSource()));
                        setFace.insert(CCE.leftFace(e->adjSource()));
                    }
                    for (auto it = setFace.begin(); it != setFace.end(); it++) {
                        adjEntry firstAdj = (*it)->firstAdj();
                        adjEntry nextAdj = firstAdj;
                        if (firstAdj != nullptr) {
                            do {
                                if (nextAdj->theEdge() != nullptr) {
                                    setEdge.insert(nextAdj->theEdge());
                                }
                                nextAdj = (*it)->nextFaceEdge(nextAdj);
                            } while ((nextAdj != firstAdj)&&(nextAdj != nullptr));
                        }
                    }
                }
                break;
            case GLFW_KEY_L:
                showAllEdges = !showAllEdges;
                break;
        }
}

double calcEdgeLengthRatio() {
    double ratio = (mapLengthEdgeSet.rbegin()->first / mapLengthEdgeSet.begin()->first);
    return ratio;
}

void changeEdgeMapValue(edge e, GraphAttributes& GA) {
    auto it = mapEdgeLength.find(e);
    double length = it->second;
    double newLength = calcEdgeLength(e, GA);
    if (length != newLength) {
        it->second = newLength;
        auto it2 = mapLengthEdgeSet.find(newLength);
        if (it2 != mapLengthEdgeSet.end()) {
            it2->second.insert(e);
        }
        else {
            std::set<edge> tmpSet;
            tmpSet.insert(e);
            mapLengthEdgeSet.insert(std::pair<double, std::set<edge>>(newLength, tmpSet));
        }
        auto it3 = mapLengthEdgeSet.find(length);
        it3->second.erase(e);
        if (it3->second.empty()) {
            mapLengthEdgeSet.erase(length);
        }
    }
}

void dispOpenGL(const Graph& G, GraphAttributes& GA, const int gridWidth, const int gridHeight, int maxX, int maxY) {
    //debut ogdf
    node n = G.firstNode();
    CCE = ConstCombinatorialEmbedding{ G };

    //fin ogdf
    if (!glfwInit())
        exit(EXIT_FAILURE);
    GLFWwindow* window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glfwSetErrorCallback(error_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);
    int width, height;
    double tempX, tempY;
    selectedEdge = G.firstEdge();
    selectedNode = G.firstNode();
    while (!glfwWindowShouldClose(window))
    {
        float ratio;
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float)height;
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        if (show_grid_size) {
            glOrtho(-1, static_cast<float>(gridWidth) + 1, -1, static_cast<float>(gridHeight) + 1, 1.f, -1.f);
        }
        else {
            glOrtho(-1, static_cast<float>(maxX) + 1, -1, static_cast<float>(maxY) + 1, 1.f, -1.f);
        }
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        // Deplacer un noeud aléatoirement
        if (move_randomly) {
            node n = G.firstNode();
            GA.x(n) += 1;
            SListPure<edge> edges;
            n->adjEdges(edges);
            for (SListConstIterator<edge> i = edges.begin(); i.valid(); i++) {
                edge e = (*i);
                changeEdgeMapValue(e, GA);
            }
            std::cout << "Ratio: " << calcEdgeLengthRatio() << std::endl;
            move_randomly = false;
        }
        //afficher les edge
        glColor3f(1.0f, 1.0f, 1.0f);
        for (auto e : G.edges)
        {
            if ((showAllEdges)&&(setEdge.find(e) != setEdge.end())) {
                glColor3f(0.0f, 0.0f, 1.0f);
            }
            else if (e == selectedEdge) {
                glColor3f(0.0f, 1.0f, 0.0f);
            }
            else {
                glColor3f(1.0f, 1.0f, 1.0f);
            }
            glBegin(GL_LINE_STRIP);
            glVertex2d(GA.x(e->source()), GA.y(e->source()));
            DPolyline& bends = GA.bends(e);
            for (ListIterator<DPoint> i = bends.begin(); i.valid(); i++) {
                glVertex2d((*i).m_x, (*i).m_y);
            }
            glVertex2d(GA.x(e->target()), GA.y(e->target()));
            glEnd();
        }
        //afficher les nodes
        glColor3f(1.0f, 0.0f, 0.0f);
        glPointSize(5);
        n = G.firstNode();
        glBegin(GL_POINTS);
        while (n != nullptr) {
            if (n == selectedNode) {
                glColor3f(0.0f, 0.0f, 1.0f);
            }
            else {
                glColor3f(1.0f, 0.0f, 0.0f);
            }
            glVertex2d(GA.x(n), GA.y(n));
            n = n->succ();
        }
        glEnd();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
}

#endif