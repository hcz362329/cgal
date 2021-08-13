#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <sstream>
#include <iostream>
#include <unordered_map>

#include <CGAL/boost/graph/named_params_helper.h>
#include <CGAL/boost/graph/Named_function_parameters.h>

namespace CGAL {
template <class PolygonMesh,
          class NamedParameters = parameters::Default_named_parameters >
void my_function_with_named_parameters(PolygonMesh& mesh, const NamedParameters& np = parameters::use_default_values())
{
  //The class containing all the geometric definitions for the PolygonMesh
  typedef typename GetGeomTraits<PolygonMesh, NamedParameters>::type  Traits;
  Traits t;
  CGAL_USE(t);
  //A vertex-index-map that is either taken from the NPs, either an already initialized map for vertex-indices.
  //Also exists for Faces, Edges and Halfedges
  typedef typename CGAL::GetInitializedVertexIndexMap<PolygonMesh, NamedParameters>::type VertexIndexMap;
  //A vertex-point-map either taken from the NPs, either a specified default map.
  typedef typename GetVertexPointMap < PolygonMesh, NamedParameters>::type VPM;
  //A face-normal-map either taken from the NPs, either a specified default map.
  typedef typename GetFaceNormalMap < PolygonMesh, NamedParameters>::type FNM;

  //The class defining all boost-graph types for the PolygonMesh, like vertex_descriptor and so.
  typedef boost::graph_traits<PolygonMesh> Graph_traits;
  typedef typename Graph_traits::vertex_descriptor vertex_descriptor;

  //in the case no helper function exists, this is how you get a type from a NP
  typedef Static_boolean_property_map<vertex_descriptor, false>                 Default_VCM;
  typedef typename internal_np::Lookup_named_param_def<internal_np::vertex_is_constrained_t,
                                                       NamedParameters,
                                                       Default_VCM>::type       VCM;

  using parameters::choose_parameter;
  using parameters::get_parameter;
  using parameters::is_default_parameter;

  //default face-normal-map if the NPs don't provide it.
  FNM default_fvmap;
  //If the NPs provide a vertex-index-map, returns it. Else, returns an initialized vertex-index map.
  VertexIndexMap vim = CGAL::get_initialized_vertex_index_map(mesh, np);
  //If the NPs provide a vertex-point-map, returns it. Else, returns the default boost-graph vpm.
  VPM vpm = choose_parameter(get_parameter(np, internal_np::vertex_point),
                             get_property_map(vertex_point, mesh));
  //If the NPs provide a face-normal-map, returns it. Else, returns default_fvmap.
  FNM fnm = choose_parameter(get_parameter(np, internal_np::face_normal),
                             default_fvmap);
  //is true if a face-normal-map is provided by he NPs
  bool has_fnm = !is_default_parameter(get_parameter(np, internal_np::face_normal));
  //boolean NP example. Default value is `false`
  bool do_project = choose_parameter(get_parameter(np, internal_np::do_project), false);

  VCM vcm_np = choose_parameter(get_parameter(np, internal_np::vertex_is_constrained), Default_VCM());


  //demonstrates usage for those values.
  if(has_fnm)
    for(auto f : faces(mesh))
    {
      std::cout<<get(fnm, f)<<std::endl;
    }

  for(auto v : vertices(mesh))
  {
    std::cout<<"vertex #"<<get(vim, v)<<" : "<<get(vpm, v)<<" : "<<get(vcm_np, v)<<std::endl;
  }

  if(do_project)
    std::cout<<"do project"<<std::endl;
  else
      std::cout<<"don't project"<<std::endl;
}

}

int main()
{
  typedef CGAL::Surface_mesh<CGAL::Epick::Point_3> SMesh;
  SMesh sm;
  std::stringstream poly("OFF\n"
                         "4 4 0\n"
                         "0 0 0\n"
                         "0 0 1\n"
                         "0 1 0\n"
                         "1 0 0\n"
                         "3  3 1 2\n"
                         "3  0 1 3\n"
                         "3  0 3 2\n"
                         "3  0 2 1\n");
  poly >> sm;
  //Define a custom face-normal-map
  typedef std::map<boost::graph_traits<SMesh>::face_descriptor,CGAL::Epick::Vector_3> FNmap;
  FNmap fnm;
  for(auto f : faces(sm))
    fnm[f] = {0,0,1};
  typedef boost::associative_property_map<FNmap> Face_normal_pmap;
  Face_normal_pmap fn_pmap(fnm);

  typedef std::map<boost::graph_traits<SMesh>::vertex_descriptor,bool> VCmap;
  VCmap vcm;
  for(auto v : vertices(sm))
  {
    if ((int)v %2 ==0)
      vcm[v] = true;
    else
      vcm[v] = false;
  }
  typedef boost::associative_property_map<VCmap> Vertex_constrained_pmap;
  Vertex_constrained_pmap vcm_pmap(vcm);
  CGAL::my_function_with_named_parameters(sm);
  CGAL::my_function_with_named_parameters(sm, CGAL::parameters::face_normal_map(fn_pmap)
                                          .vertex_is_constrained_map(vcm_pmap)
                                          .do_project(true));
  return 0;
}
