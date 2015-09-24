/*
 * graph_viz_prop_writer.hpp
 *
 *  Created on: 08/apr/2015
 *      Author: filippo
 */

#ifndef GRAPH_VIZ_PROP_WRITER_HPP_
#define GRAPH_VIZ_PROP_WRITER_HPP_
#include <iostream>


//DEFINES THE FUNCTIONS TO PROPERLY DRAW THE GRAPH

// Redefine how edge are represented
template <class WeightMap,class CapacityMap, class PropertyMap>
class edge_writer {
public:
  edge_writer(WeightMap w, CapacityMap c, PropertyMap r) : wm(w),cm(c), pm(r) {}
  template <class Edge>
  void operator()(std::ostream &out, const Edge& e) const {
		if (pm[e]==WAIT)
    out << "[label=\"" << wm[e] << "\", taildistance=\"" << cm[e] << "\""<< ",  minlen=1"<< "]";
		else if (pm[e]==TRAVEL)
	out << "[label=\"" << wm[e] << "\", taildistance=\"" << cm[e] << "\""<< ",  constraint=false"<< "]";
		else if(pm[e]==RELOCATION)
	out << "[label=\"" << wm[e] << "\", taildistance=\"" << cm[e] << "\""<< ",  style=dotted,  constraint=false"<< "]";
		else if (pm[e]==TRANSFER)
	out << "[label=\"" << wm[e] << "\", taildistance=\"" << cm[e] << "\""<< ", color=\"red\", constraint=false, style=\"invis\""<< "]";
  }


// Add propery maps 
private:
  WeightMap wm;
  CapacityMap cm;
  PropertyMap pm;
};

template <class WeightMap, class CapacityMap, class PropertyMap>
inline edge_writer<WeightMap,CapacityMap, PropertyMap>
make_edge_writer(WeightMap w,CapacityMap c, PropertyMap r) {
  return edge_writer<WeightMap,CapacityMap,PropertyMap>(w,c,r);
}


// Redefine how vertices are represented
template <class WeightMap,class CapacityMap>
class vertex_writer {
public:
  vertex_writer(WeightMap w, CapacityMap c) : wm(w),cm(c) {}
  template <class Vertex>
  void operator()(std::ostream &out, const Vertex& v) const {
	  out << "[label=\"s" << wm[v] << "t" << cm[v] << "\""<<"]";
  }

// Add property maps 
private:
  WeightMap wm;
  CapacityMap cm;
 };

template <class WeightMap, class CapacityMap>
inline vertex_writer<WeightMap,CapacityMap>
make_vertex_writer(WeightMap w,CapacityMap c) {
  return vertex_writer<WeightMap,CapacityMap>(w,c);
}




#endif  
