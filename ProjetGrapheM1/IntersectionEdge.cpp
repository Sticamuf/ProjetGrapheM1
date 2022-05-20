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
	if(sontAlignes(px,py,qx,qy,rx,ry))
	{
		align=1;
		return true;
	}
	else if(sontAlignes(px,py,qx,qy,sx,sy))
	{
		align=2;
		return true;
	}
	else if(sontAlignes(rx,ry,sx,sy,px,py))
	{
		align=3;
		return true;
	}
	else if(sontAlignes(rx,ry,sx,sy,qx,qy))
	{
		align=4;
		return true;
	}
	align=-1;
	return false;
		   
}

bool EstSuperieurX(int px,int rx)
{
	if(px<rx)
	{
		return true;	
	}
	return false;
}

bool EstInferieurX(int px,int rx)
{
	if(px>rx)
	{
		return true;	
	}
	return false;
}

bool EstSuperieurY(int py,int ry)
{
	if(py<ry)
	{
		return true;	
	}
	return false;
}

bool EstInferieurY(int py,int ry)
{
	if(py>ry)
	{
		return true;	
	}
	return false;
}

int AGauche(int px, int py, int qx, int qy, int rx, int ry)
{
    long long val = ((qy - py) * (rx - qx)) - ((qx - px) * (ry - qy));
 
    if (val == 0)
    {
	    return 0;
    }
    else if(val>0)
    {
	    return 1;
    }
    else
    {
	    return -1;
    }
    
}

bool seCroisent(int px, int py, int qx, int qy, int rx, int ry, int sx, int sy)
{
	int ag1 = AGauche(px,py,qx,qy,rx,ry);
    	int ag2 = AGauche(px,py,qx,qy,sx,sy);
    	int ag3 = AGauche(rx,ry,sx,sy,px,py);
   	int ag4 = AGauche(rx,ry,sx,sy,qx,qy);
	
	// cas général
    	if (ag1 != ag2 && ag3 != ag4)
	{
		return true;
	}
	else
	{
		return false;	
	}
}

bool Croisement(int px, int py, int qx, int qy, int rx, int ry, int sx, int sy)
{
	if(Alignes(px,py,qx,qy,rx,ry,sx,sy))
	{
		if(align==1)
		{
			if((EstInferieurX(px,rx)&&EstSuperieurX(qx,rx))||
			   (EstSuperieurX(px,rx)&&EstInferieurX(qx,rx))||
			   (EstSuperieurY(py,ry)&&EstInferieurY(qy,ry))||
			   (EstInferieurY(py,ry)&&EstSuperieurY(qy,ry)))
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
			if((EstInferieurX(px,sx)&&EstSuperieurX(qx,sx))||
			   (EstSuperieurX(px,sx)&&EstInferieurX(qx,sx))||
			   (EstSuperieurY(py,sy)&&EstInferieurY(qy,sy))||
			   (EstInferieurY(py,sy)&&EstSuperieurY(qy,sy)))
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
			if((EstInferieurX(rx,px)&&EstSuperieurX(sx,px))||
			   (EstSuperieurX(rx,px)&&EstInferieurX(sx,px))||
			   (EstSuperieurY(ry,py)&&EstInferieurY(sy,py))||
			   (EstInferieurY(ry,py)&&EstSuperieurY(sy,py)))
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
			if((EstInferieurX(rx,qx)&&EstSuperieurX(sx,qx))||
			   (EstSuperieurX(rx,qx)&&EstInferieurX(sx,qx))||
			   (EstSuperieurY(ry,qy)&&EstInferieurY(sy,qy))||
			   (EstInferieurY(ry,qy)&&EstSuperieurY(sy,qy)))
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
		if(seCroisent(px,py,qx,qy,rx,ry,sx,sy))
		{
			return true;	
		}
	}	
}

bool intersection(const edge& e1,const edge& e2,const GraphAttributes& GA)
{
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
        	if(Croisement(sourceX1,sourceY1,targetX1,targetY1,sourceX2,sourceY2,targetX2,targetY2))
		{
			return true;	
		}
    	}
	else if (bends1.size() > 0 && bends2.size()>0)
	{
		for(ListIterator<DPoint> i = bends1.begin(); i.valid(); i++) {
			targetX1 = (*i).m_x;
            		targetY1 = (*i).m_y;
			for(ListIterator<DPoint> j= bends2.begin(); j.valid(); j++)
			{
				targetX2 = (*j).m_x;
            			targetY2 = (*j).m_y;
				if(Croisement(sourceX1,sourceY1,targetX1,targetY1,sourceX2,sourceY2,targetX2,targetY2))
				{
					return true;	
				)
				int endX2=GA.x(target2);
				int endY2=GA.y(target2);
				int originX2=GA.x(source2);
				int originY2=GA.y(source2);
				if(Croisement(originX2,originY2,endX2,endY2,sourceX2,sourceY2,targetX2,targetY2))
				{
					if((endX2==targetX2&&endY2==targetY2)||(originX2==sourceX2&&originY2==sourceY2))
					{
						return false;
					}
					else
					{
						return true;
					}					
				)
				sourceX2 = targetX2;
            			sourceY2 = targetY2;
			}
			sourceX2 = GA.x(source2);
            		sourceY2 = GA.y(source2);
					int endX1=GA.x(target1);
				int endY1=GA.y(target1);
				int originX1=GA.x(source1);
				int originY1=GA.y(source1);
				if(Croisement(originX1,originY1,endX1,endY1,sourceX1,sourceY1,targetX1,targetY1))
				{
					if((endX1==targetX1&&endY1==targetY1)||(originX1==sourceX1&&originY1==sourceY1))
					{
						return false;
					}
					else
					{
						return true;
					}					
				)
            		sourceX1 = targetX1;
            		sourceY1 = targetY1;
        	}
		targetX1 = GA.x(target1);
		targetY1 = GA.y(target1);
  
		targetX2 = GA.x(target2);
		targetY2 = GA.y(target2);
        	if(Croisement(sourceX1,sourceY1,targetX1,targetY1,sourceX2,sourceY2,targetX2,targetY2))
		{
			return true;	
		}
		
	}
	else if(bends2.size()==0)
	{
		targetX2 = GA.x(target2);
		targetY2 = GA.y(target2);
		for(ListIterator<DPoint> i = bends1.begin(); i.valid(); i++) {
			targetX1 = (*i).m_x;
            		targetY1 = (*i).m_y;
			if(Croisement(sourceX1,sourceY1,targetX1,targetY1,sourceX2,sourceY2,targetX2,targetY2))
			{
				return true;	
			}
			int endX2=GA.x(target2);
				int endY2=GA.y(target2);
				int originX2=GA.x(source2);
				int originY2=GA.y(source2);
				if(Croisement(originX2,originY2,endX2,endY2,sourceX2,sourceY2,targetX2,targetY2))
				{
					if((endX2==targetX2&&endY2==targetY2)||(originX2==sourceX2&&originY2==sourceY2))
					{
						return false;
					}
					else
					{
						return true;
					}					
				)
			sourceX1 = targetX1;
            		sourceY1 = targetY1;
		}
		targetX1 = GA.x(target1);
		targetY1 = GA.y(target1);
		if(Croisement(sourceX1,sourceY1,targetX1,targetY1,sourceX2,sourceY2,targetX2,targetY2))
		{
			return true;	
		}
	}
	else
	{
		targetX1 = GA.x(target1);
		targetY1 = GA.y(target1);
		for(ListIterator<DPoint> j= bends2.begin(); j.valid(); j++)
		{
			targetX2 = (*j).m_x;
            		targetY2 = (*j).m_y;
			if(Croisement(sourceX1,sourceY1,targetX1,targetY1,sourceX2,sourceY2,targetX2,targetY2))
			{
				return true;	
			}
			int endX1=GA.x(target1);
				int endY1=GA.y(target1);
				int originX1=GA.x(source1);
				int originY1=GA.y(source1);
				if(Croisement(originX1,originY1,endX1,endY1,sourceX1,sourceY1,targetX1,targetY1))
				{
					if((endX1==targetX1&&endY1==targetY1)||(originX1==sourceX1&&originY1==sourceY1))
					{
						return false;
					}
					else
					{
						return true;
					}					
				)
			sourceX2 = targetX2;
            		sourceY2 = targetY2;
		}
		targetX2 = GA.x(target2);
		targetY2 = GA.y(target2);
        	if(Croisement(sourceX1,sourceY1,targetX1,targetY1,sourceX2,sourceY2,targetX2,targetY2))
		{
			return true;	
		}
	}
	
	
 	/*
    	// cas particuliers
    	if (o1 == 0 && surSegment(sourceX1,sourceY1,targetX1,targetY1,sourceX2,sourceY2))
	{
		return true;
	}
 
    	if (o2 == 0 && surSegment(sourceX1,sourceY1,targetX1,targetY1,targetX2,targetY2))
	{
		return true;
	}
 
    	if (o3 == 0 && surSegment(sourceX2,sourceY2,targetX2,targetY2,sourceX1,sourceY1))
	{
		return true;
	}
 
    	if (o4 == 0 && surSegment(sourceX2,sourceY2,targetX2,targetY2,targetX1,targetY1))
	{
		return true;
	}
	*/
	
	
	return false;

}

void ModidfierVecteur(const edge& e1,const edge& e2,const GraphAttributes& GA)
{
	// Si vraie on les ajoutent dans le vector s'ils n'y sont pas
	if(intersection(e1,e2,GA))
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
		if(estDansVector(e2))
		{
			posE2=posE1;
			in.erase(in.begin()+posE2);
		}
		if(estDansVector(e1))
		{
			in.erase(in.begin()+posE1);
		}


	}
}
