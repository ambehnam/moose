//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarUserObject.h"
#include "MooseVariableFE.h"
#include "SubProblem.h"
#include "Assembly.h"

#include "libmesh/string_to_enum.h"

InputParameters
MortarUserObject::validParams()
{
  InputParameters params = UserObject::validParams();
  params += MortarConsumerInterface::validParams();
  params += TwoMaterialPropertyInterface::validParams();
  params += TransientInterface::validParams();
  params += RandomInterface::validParams();

  // Whether on a displaced or undisplaced mesh, coupling ghosting will only happen for
  // cross-interface elements
  params.addRelationshipManager("AugmentSparsityOnInterface",
                                Moose::RelationshipManagerType::COUPLING,
                                [](const InputParameters & obj_params, InputParameters & rm_params)
                                {
                                  rm_params.set<bool>("use_displaced_mesh") =
                                      obj_params.get<bool>("use_displaced_mesh");
                                  rm_params.set<BoundaryName>("secondary_boundary") =
                                      obj_params.get<BoundaryName>("secondary_boundary");
                                  rm_params.set<BoundaryName>("primary_boundary") =
                                      obj_params.get<BoundaryName>("primary_boundary");
                                  rm_params.set<SubdomainName>("secondary_subdomain") =
                                      obj_params.get<SubdomainName>("secondary_subdomain");
                                  rm_params.set<SubdomainName>("primary_subdomain") =
                                      obj_params.get<SubdomainName>("primary_subdomain");
                                  rm_params.set<bool>("ghost_higher_d_neighbors") =
                                      obj_params.get<bool>("ghost_higher_d_neighbors");
                                });
                                
  // Note that in my experience it is only important for the higher-d lower-d point neighbors to be
  // ghosted when forming sparsity patterns and so I'm putting this here instead of at the
  // MortarConsumerInterface level
  params.addRelationshipManager("GhostHigherDLowerDPointNeighbors",
                                Moose::RelationshipManagerType::GEOMETRIC);

  params.addParam<MooseEnum>(
      "quadrature",
      MooseEnum("DEFAULT FIRST SECOND THIRD FOURTH", "DEFAULT"),
      "Quadrature rule to use on mortar segments. For 2D mortar DEFAULT is recommended. "
      "For 3D mortar, QUAD meshes are integrated using triangle mortar segments. "
      "While DEFAULT quadrature order is typically sufficiently accurate, exact integration of "
      "QUAD mortar faces requires SECOND order quadrature for FIRST variables and FOURTH order "
      "quadrature for SECOND order variables.");
  return params;
}

MortarUserObject::MortarUserObject(const InputParameters & parameters)
  : UserObject(parameters),
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(this, false, false),
    MortarConsumerInterface(this),
    TwoMaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, getBoundaryIDs()),
    TransientInterface(this),
    RandomInterface(parameters, _fe_problem, _tid, false),
    ElementIDInterface(this),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _mesh(_subproblem.mesh()),
    _interior_secondary_elem(_assembly.elem()),
    _interior_primary_elem(_assembly.neighbor()),
    _lower_secondary_elem(_assembly.lowerDElem()),
    _lower_primary_elem(_assembly.neighborLowerDElem()),
    _interior_secondary_volume(_assembly.elemVolume()),
    _current_side(_assembly.side()),
    _current_side_elem(_assembly.sideElem()),
    _current_side_volume(_assembly.sideElemVolume()),
    _interior_primary_volume(_assembly.neighborVolume()),
    _current_boundary_id(_assembly.currentBoundaryID()),
    _normals_primary(_assembly.neighborNormals()),
    _tangents(_assembly.tangents()),
    _coord(_assembly.mortarCoordTransformation()),
    _q_point(_assembly.qPointsMortar())
{

  // Note parameter is discretization order, we then convert to quadrature order
  const MooseEnum p_order = getParam<MooseEnum>("quadrature");
  // If quadrature not DEFAULT, set mortar qrule
  if (p_order != "DEFAULT")
  {
    Order q_order = static_cast<Order>(2 * Utility::string_to_enum<Order>(p_order) + 1);
    _assembly.setMortarQRule(q_order);
  }

}
