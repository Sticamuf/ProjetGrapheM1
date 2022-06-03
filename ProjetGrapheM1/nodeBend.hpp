#ifndef NODEBEND_HPP
#define NODEBEND_HPP
#include <ogdf/basic/GridLayout.h>
#include <ogdf/basic/EdgeArray.h>
#include <ogdf/fileformats/GraphIO.h>
using namespace ogdf;
class NodeBend {
public:
	bool isNode;
	int* a_x;
	int* a_y;
	int numero;
	NodeBend(node n, GridLayout& GL) {
		isNode = true;
		m_n = n;
		a_x = &GL.x(n);
		a_y = &GL.y(n);
		numero = -1;
	}
	NodeBend(IPoint& p, edge e, int num) {
		isNode = false;
		m_p = &p;
		m_e = e;
		a_x = &p.m_x; 
		a_y = &p.m_y;
		numero = num;
	}
	node getNode() {
		return m_n;
	}
	IPoint* getPoint() {
		return m_p;
	}
	edge getEdge() {
		return m_e;
	}
	adjEntry getAdjEntry() {
		if (!isNode) {
			return m_e->adjSource();
		}
		return nullptr;
	}
	//const bool operator<(const NodeBend& nb) const {
	//	if (*a_x < *nb.a_x) {
	//		return (*a_y < *nb.a_y);
	//		//{
	//		//	return true;
	//		//}
	//		//return false;
	//	}
	//	return false;
	//	/*if (*a_x >= *nb.a_x) {
	//		return (*a_y < *nb.a_y);
	//	}
	//	if (*a_y >= *nb.a_y) {
	//		return (*a_x < *nb.a_x);
	//	}*/
	//}
	int getX() const {
		return *a_x;
	}
	int getY() const {
		return *a_y;
	}
private:
	node m_n = nullptr;
	edge m_e = nullptr;
	IPoint* m_p = nullptr;
};

struct NodeBendXComp {
	const bool operator() (const NodeBend& p1, const NodeBend& p2) const {
		if (p1.getX() < p2.getX()) {
			return true;
		}
		if (p1.getX() > p2.getX()) {
			return false;
		}
		return p1.getY() < p2.getY();
	}
};

struct NodeBendYComp {
	const bool operator() (const NodeBend& p1, const NodeBend& p2) const {
		if (p1.getY() < p2.getY()) {
			return true;
		}
		if (p1.getY() > p2.getY()) {
			return false;
		}
		return p1.getX() < p2.getX();
	}
};
#endif