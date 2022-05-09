#include <Windows.h>
#include <gl/GL.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/basic/Layout.h>

using ogdf::Graph;
using ogdf::node;
using ogdf::DPolyline;
using ogdf::GraphAttributes;
using ogdf::ListIterator;
using ogdf::DPoint;
using ogdf::Layout;

bool move_randomly = false;

void error_callback(int error, const char* description);

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void dispOpenGL(const Graph& G, /*const*/GraphAttributes& GA, const int gridWidth, const int gridHeight);