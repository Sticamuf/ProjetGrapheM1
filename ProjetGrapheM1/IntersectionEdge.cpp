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
bool intersection(const edge& e1,const edge& e2,const GraphAttributes& GA)
{
  bool inter=false;
  //push_back si n'est pas dans le vector
  //erase si on doit l'enlever
  
  node source1 = e1->source();
  node target2 = e2->target();
  
  node source2 = e2->source();
  node target2 = e2->target();
  
  double sourceX1 = GA.x(source1);
  double sourceY1 = GA.y(source1);
  
  double sourceX2 = GA.x(source2);
  double sourceY2 = GA.y(source2);
  return inter;
  
  
}
