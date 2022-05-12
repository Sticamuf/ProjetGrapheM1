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
int align=-1;


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

/*bool surSegment(int px, int py, int qx, int qy, int rx, int ry)
{
	if (qx <= max(px, rx) && qx >= min(px, rx) && qy <= max(py, ry) && qy >= min(py, ry))
	{
		return true;
	}
    	return false;
}*/

bool sontAlignes(int px, int py, int qx, int qy, int rx, int ry) // pour vérifier si c'est aligné
{
 	return((ry-py)/(rx-px)==(qy-py)/(qx-px)); 	
}

bool Alignes(int px, int py, int qx, int qy, int rx, int ry, int sx, int sy)
{
	if(sontAlignes(px,py,qx,qy,rx,ry)==true)
	{
		align=1;
		return true;
	}
	else if(sontAlignes(px,py,qx,qy,sx,sy)==true)
	{
		align=2;
		return true;
	}
	else if(sontAlignes(rx,ry,sx,sy,px,py)==true)
	{
		align=3;
		return true;
	}
	else if(sontAlignes(rx,ry,sx,sy,qx,qy)==true)
	{
		align=4;
		return true;
	}
	align=-1;
	return false;
		   
}

bool estADroite(int px,int rx)
{
	if(px<rx)
	{
		return true;	
	}
	return false;
}

bool estAGauche(int px,int rx)
{
	if(px>rx)
	{
		return true;	
	}
	return false;
}

bool estEnHaut(int py,int ry)
{
	if(py<ry)
	{
		return true;	
	}
	return false;
}

bool estEnBas(int py,int ry)
{
	if(py>ry)
	{
		return true;	
	}
	return false;
}

/*int orientation(int px, int py, int qx, int qy, int rx, int ry)
{
    long long val = ((qy - py) * (rx - qx)) - ((qx - px) * (ry - qy));
 
    if (val == 0) return 0;  // collinéaire
 
    return (val > 0)? 1: 2; // dans le sens des aiguilles d'une montre ou contraire
}*/

bool seCroisent(int px, int py, int qx, int qy, int rx, int ry, int sx, int sy)
{
	if(estADroite(px,rx)==true&&estAGauche(qx,rx)==true&&
			   estEnHaut(py,ry)&&estEnHaut(qy,ry)&&
			   estEnBas(py,sy)&&estEnBas(qy,sy))
	{
		return true;	
	}
}

bool Croisement(int px, int py, int qx, int qy, int rx, int ry, int sx, int sy)
{
	if(Alignes(px,py,qx,qy,rx,ry,sx,sy)==true)
	{
		if(align==1)
		{
			if((estAGauche(px,sx)==true&&estAGauche(qx,sx)==true)||
			   (estADroite(px,sx)==true&&estADroite(qx,sx)==true)||
			   (estEnHaut(py,sy)==true&&estEnHaut(qy,sy)==true)||
			   (estEnBas(py,sy)==true&&estEnBas(qy,sy)==true))
			{
				return true;	
			}
			else
			{
				return false;
			}
					
		}
		else if(align==2)
		{
			if((estAGauche(px,rx)==true&&estAGauche(qx,rx)==true)||
			   (estADroite(px,rx)==true&&estADroite(qx,rx)==true)||
			   (estEnHaut(py,ry)==true&&estEnHaut(qy,ry)==true)||
			   (estEnBas(py,ry)==true&&estEnBas(qy,ry)==true))
			{
				return true;	
			}
			else
			{
				return false;
			}
		}
		else if(align==3)
		{
			if()
			{
				return true;	
			}
			else
			{
				return false;
			}
		}
		else if(align==4)
		{
			if()
			{
				return true;	
			}
			else
			{
				return false;
			}
		}
	}
	else
	{
		if(seCroisent(px,py,qx,qy,rx,ry,sx,sy)==true)
		{
			return true;	
		}
	}	
}

bool intersection(const edge& e1,const edge& e2,const GraphAttributes& GA)
{
	bool inter=false;
	align=-1;
	
  
	node source1 = e1->source();
	node target1 = e1->target();
  
	node source2 = e2->source();
	node target2 = e2->target();
  
	int sourceX1 = GA.x(source1);
	int sourceY1 = GA.y(source1);
  
	int sourceX2 = GA.x(source2);
	int sourceY2 = GA.y(source2);
  
	int targetX1,targetY1,targetX2,targetY2;
	
	DPolyline bends1 = GA.bends(e1);
	DPolyline bends2 = GA.bends(e2);
	if (bends1.size() == 0 && bends2.size()==0) {
		targetX1 = GA.x(target1);
		targetY1 = GA.y(target1);
  
		targetX2 = GA.x(target2);
		targetY2 = GA.y(target2);
        	if(Croisement(sourceX1,sourceY1,targetX1,targetY1,sourceX2,sourceY2,targetX2,targetY2)==true)
		{
			inter=true;	
		}
    	}
	else
	{
		for(ListIterator<DPoint> i = bends1.begin(); i.valid(); i++) {
			targetX1 = (*i).m_x;
            		targetY1 = (*i).m_y;
			for(ListIterator<DPoint> j= bends2.begin(); j.valid(); j++)
			{
				targetX2 = (*j).m_x;
            			targetY2 = (*j).m_y;
				if(Croisement(sourceX1,sourceY1,targetX1,targetY1,sourceX2,sourceY2,targetX2,targetY2)==true)
				{
					inter=true;	
				}
				sourceX2 = targetX2;
            			sourceY2 = targetY2;
			}
            		sourceX1 = targetX1;
            		sourceY1 = targetY1;
        	}
		targetX1 = GA.x(target1);
		targetY1 = GA.y(target1);
  
		targetX2 = GA.x(target2);
		targetY2 = GA.y(target2);
        	if(Croisement(sourceX1,sourceY1,targetX1,targetY1,sourceX2,sourceY2,targetX2,targetY2)==true)
		{
			inter=true;	
		}
		
	}
	
	/*int o1 = orientation(sourceX1,sourceY1,targetX1,targetY1,sourceX2,sourceY2);
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
	*/
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
