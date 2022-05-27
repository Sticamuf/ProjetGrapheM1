#ifndef NODEBEND_HPP
#define NODEBEND_HPP
class NodeBend {
public:
	bool isNode;
	int* a_x;
	int* a_y;
	NodeBend(node n, GridLayout& GL) {
		isNode = true;
		m_n = n;
		a_x = &GL.x(n);
		a_y = &GL.y(n);
	}
	NodeBend(IPoint& p, edge e) {
		isNode = false;
		m_p = &p;
		m_e = e;
		a_x = &p.m_x;
		a_y = &p.m_y;
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

private:
	node m_n = nullptr;
	edge m_e = nullptr;
	IPoint* m_p = nullptr;
};
#endif