//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UpdateNeighborElementPairs.h"
#include "InputParameters.h"

#include "libmesh/periodic_boundary.h" // translation PBCs provided by libmesh
#include "libmesh/periodic_boundaries.h"

registerMooseObject("MooseApp", UpdateNeighborElementPairs);

InputParameters
UpdateNeighborElementPairs::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addClassDescription("This is a hard-code adding of a pair.");
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addRequiredParam<std::vector<BoundaryName>>("primary", "Boundary ID associated with the primary boundary.");
  params.addRequiredParam<std::vector<BoundaryName>>("secondary", "Boundary ID associated with the secondary boundary.");
  params.addParam<RealVectorValue>("translation",
                                   "Vector that translates coordinates on the "
                                   "primary boundary to coordinates on the "
                                   "secondary boundary.");

  return params;
}

UpdateNeighborElementPairs::UpdateNeighborElementPairs(const InputParameters & parameters)
  : MeshGenerator(parameters), _input(getMesh("input"))
{
}

// Used to temporarily store information about which lower-dimensional
// sides to add and what subdomain id to use for the added sides.
struct ElemSideDoubleUEN
{
  ElemSideDoubleUEN(Elem * elem_in, unsigned short int side_in) : elem(elem_in), side(side_in) {}

  Elem * elem;
  unsigned short int side;
};

std::unique_ptr<MeshBase>
UpdateNeighborElementPairs::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  const unsigned int elem_id = 0;
  const unsigned int neighbor_id = 1;
  const unsigned int elem_side = 2;
  const unsigned int neighbor_side = 0;

  RealVectorValue translation = getParam<RealVectorValue>("translation");

  std::vector<BoundaryName> prim_side(getParam<std::vector<BoundaryName>>("primary"));
  std::vector<BoundaryName> seco_side(getParam<std::vector<BoundaryName>>("secondary"));
  auto p_sideset_ids = MooseMeshUtils::getBoundaryIDs(*mesh, prim_side, true);
  std::set<boundary_id_type> p_sidesets(p_sideset_ids.begin(), p_sideset_ids.end());
  auto s_sideset_ids = MooseMeshUtils::getBoundaryIDs(*mesh, seco_side, true);
  std::set<boundary_id_type> s_sidesets(s_sideset_ids.begin(), s_sideset_ids.end());

  PeriodicBoundary p(translation);
  for (auto & bnd_id : p_sidesets)
  {
    p.myboundary = bnd_id;
  }
  for (auto & bnd_id : s_sidesets)
  {
    p.pairedboundary = bnd_id;
  }
  PeriodicBoundaries pbc;
  pbc.emplace(p.myboundary, &p);
  PeriodicBoundary pi(translation);
  for (auto & bnd_id : s_sidesets)
  {
    pi.myboundary = bnd_id;
  }
  for (auto & bnd_id : p_sidesets)
  {
    pi.pairedboundary = bnd_id;
  }
  PeriodicBoundaries pbci;
  pbci.emplace(pi.myboundary, &pi);
  std::unique_ptr<PointLocatorBase> pl = mesh->sub_point_locator();


  // Elem * current_elem = mesh->elem_ptr(elem_id);
  // Elem * neighbor_elem2 = current_elem->topological_neighbor(elem_side, *mesh, *pl, &pbc);
  // Elem * neighbor_elem = mesh->elem_ptr(neighbor_id);
  // Elem * current_elem2 = neighbor_elem->topological_neighbor(neighbor_side, *mesh, *pl, &pbci);
  // current_elem->set_neighbor(elem_side, neighbor_elem);
  // neighbor_elem->set_neighbor(neighbor_side, current_elem);


  // BoundaryInfo & boundary_info = mesh->get_boundary_info();
  auto side_list = mesh->get_boundary_info().build_side_list();

  std::vector<std::pair<dof_id_type, ElemSideDoubleUEN>> element_sides_on_boundaryP;
  dof_id_type counter = 0;
  for (const auto & triple : side_list)
    if (p_sidesets.count(std::get<2>(triple)))
    {
      if (auto elem = mesh->query_elem_ptr(std::get<0>(triple)))
        element_sides_on_boundaryP.push_back(
            std::make_pair(counter, ElemSideDoubleUEN(elem, std::get<1>(triple))));
      ++counter;
    }

  std::vector<std::pair<dof_id_type, ElemSideDoubleUEN>> element_sides_on_boundaryS;
  counter = 0;
  for (const auto & triple : side_list)
    if (s_sidesets.count(std::get<2>(triple)))
    {
      if (auto elem = mesh->query_elem_ptr(std::get<0>(triple)))
        element_sides_on_boundaryS.push_back(
            std::make_pair(counter, ElemSideDoubleUEN(elem, std::get<1>(triple))));
      ++counter;
    }
    
  for (auto & [i, elem_side] : element_sides_on_boundaryP)
  {
    Elem * elem = elem_side.elem;

    const auto side = elem_side.side;
    Elem * neighbor_elem = elem->topological_neighbor(side, *mesh, *pl, &pbc);
    elem->set_neighbor(side, neighbor_elem);
  }
    
  for (auto & [i, elem_side] : element_sides_on_boundaryS)
  {
    Elem * elem = elem_side.elem;

    const auto side = elem_side.side;
    Elem * neighbor_elem = elem->topological_neighbor(side, *mesh, *pl, &pbci);
    elem->set_neighbor(side, neighbor_elem);
  }

  // pbc.clear();

  Partitioner::set_node_processor_ids(*mesh);

  return dynamic_pointer_cast<MeshBase>(mesh);
}
