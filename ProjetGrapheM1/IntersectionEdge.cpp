#include <ogdf/basic/GraphAttributes.h>

using ogdf::edge;
using ogdf::GraphAttributes;
using ogdf::node;
using ogdf::DPolyline;
using ogdf::ListIterator;
using ogdf::DPoint;


std::vector<edge> in;
int posE1=-1;
int posE2=-1;

bool estDansVector(const edge& e)
{
	if(in.size()==0)
	{
		return false; 
	}
	else
	{
		for(int i=0;i<in.size();i++)
		{
			if(e==in[i])
			{
				posE1=i;
				return true; 
			}
		}
    return false;
	}
}

bool surSegment(int px, int py, int qx, int qy, int rx, int ry) // pour vérifier si c'est aligné
{
	if (qx <= max(px, rx) && qx >= min(px, rx) && qy <= max(py, ry) && qy >= min(py, ry))
	{
		return true;
	}
    return false;
}

int orientation(int px, int py, int qx, double qy, double rx, double ry)
{
    long long val = ((qy - py) * (rx - qx)) - ((qx - px) * (ry - qy));
 
    if (val == 0) return 0;  // collinéaire
 
    return (val > 0)? 1: 2; // dans le sens des aiguilles d'une montre ou contraire
}

bool intersection(const edge& e1,const edge& e2,const GraphAttributes& GA)
{
	bool inter=false;
	
  
	node source1 = e1->source();
	node target1 = e1->target();
  
	node source2 = e2->source();
	node target2 = e2->target();
  
	double sourceX1 = GA.x(source1);
	double sourceY1 = GA.y(source1);
  
	double sourceX2 = GA.x(source2);
	double sourceY2 = GA.y(source2);
  
	double targetX1 = GA.x(target1);
	double targetY1 = GA.y(target1);
  
	double targetX2 = GA.x(target2);
	double targetY2 = GA.y(target2);
	
	int o1 = orientation(sourceX1,sourceY1,targetX1,targetY1,sourceX2,sourceY2);
    int o2 = orientation(sourceX1,sourceY1,targetX1,targetY1,targetX2,targetY2);
    int o3 = orientation(sourceX2,sourceY2,targetX2,targetY2,sourceX1,sourceY1);
    int o4 = orientation(sourceX2,sourceY2,targetX2,targetY2,targetX1,targetY1);
	
	// cas général
    if (o1 != o2 && o3 != o4)
	{
		inter = true;
	}
 
    // cas particuliers
    if (o1 == 0 && surSegment(sourceX1,sourceY1,targetX1,targetY1,sourceX2,sourceY2))
	{
		inter=true;
	}
 
    if (o2 == 0 && surSegment(sourceX1,sourceY1,targetX1,targetY1,targetX2,targetY2))
	{
		inter=true;
	}
 
    if (o3 == 0 && surSegment(sourceX2,sourceY2,targetX2,targetY2,sourceX1,sourceY1))
	{
		inter=true;
	}
 
    if (o4 == 0 && surSegment(sourceX2,sourceY2,targetX2,targetY2,targetX1,targetY1))
	{
		inter=true;
	}
	// Si vraie on les ajoutent dans le vector s'ils n'y sont pas
	if(inter==true)
	{
		if(estDansVector(e1)==false)
		{
			in.push_back(e1);
		}
		if(estDansVector(e2)==false)
		{
			in.push_back(e2);
		}
	}
	else //Si faux on les enlèvent du vector s'ils s'y trouvent
	{
		posE1=-1;
		posE2=-1;
		if(estDansVector(e2)==true)
		{
			posE2=posE1;
			in.erase(posE2);
		}
		if(estDansVector(e1)==true)
		{
			in.erase(posE1);
		}


	}
	return inter;

}
