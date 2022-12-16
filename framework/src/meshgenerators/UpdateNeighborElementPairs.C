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

registerMooseObject("MooseApp", UpdateNeighborElementPairs);

InputParameters
UpdateNeighborElementPairs::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addClassDescription("This is a hard-code adding of a pair.");
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");

  return params;
}

UpdateNeighborElementPairs::UpdateNeighborElementPairs(const InputParameters & parameters)
  : MeshGenerator(parameters), _input(getMesh("input"))
{
}

std::unique_ptr<MeshBase>
UpdateNeighborElementPairs::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  const unsigned int elem_id = 0;
  const unsigned int neighbor_id = 1;
  const unsigned int elem_side = 2;
  const unsigned int neighbor_side = 0;

  Elem * current_elem = mesh->elem_ptr(elem_id);
  Elem * neighbor_elem = mesh->elem_ptr(neighbor_id);
  current_elem->set_neighbor(elem_side, neighbor_elem);
  neighbor_elem->set_neighbor(neighbor_side, current_elem);

  Partitioner::set_node_processor_ids(*mesh);

  return dynamic_pointer_cast<MeshBase>(mesh);
}
