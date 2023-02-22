//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"

/**
 *
 */
class UpdateNeighborElementPairs : public MeshGenerator
{
public:
  static InputParameters validParams();

  UpdateNeighborElementPairs(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// The mesh to modify
  std::unique_ptr<MeshBase> & _input;
  /// A vector of the names of the element sidesets
  const std::vector<BoundaryName> _adj_sidesets;
  /// A vector of the names of the neighbor sidesets
  const std::vector<BoundaryName> _sep_sidesets;
};
