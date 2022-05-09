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
bool intersection(const edge& e1,const edge& e2)
{
  bool inter=false;
  return inter;
  
  
}
