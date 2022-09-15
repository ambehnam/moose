//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "UserObject.h"
#include "NeighborCoupleableMooseVariableDependencyIntermediateInterface.h"
#include "MortarConsumerInterface.h"
#include "TwoMaterialPropertyInterface.h"
#include "MooseMesh.h"
#include "TransientInterface.h"
#include "RandomInterface.h"
#include "ElementIDInterface.h"
#include "MooseError.h"

class MooseVariableFieldBase;

namespace libMesh
{
class QBase;
}

/**
 * This user object allows related evaluations on elements, boundaries, internal sides,
 * interfaces in one single place. MortarUserObject is still block restrictable.
 * While evaluations on elements, boundaries and internal sides always happen,
 * a parameter 'interface_boundaries' needs to be set to invoke evaluations on interfaces.
 * We require this parameter for interface evaluations because we want to enforce sanity
 * checks on coupling variables that are not defined on the domain this user object works on
 * but only are available on the other side of the interfaces. All sides of an interface must
 * connect with the subdomain of MortarUserObject.
 * With this user object, evaluations that would have to be put into ElementUserObject,
 * SideUserObject, InternalSideUserObject, and InterfaceUserObject separately may be combined.
 */
class MortarUserObject : public UserObject,
                         public NeighborCoupleableMooseVariableDependencyIntermediateInterface,
                         public MortarConsumerInterface,
                         public TwoMaterialPropertyInterface,
                         public TransientInterface,
                         public RandomInterface,
                         public ElementIDInterface
{
public:
  static InputParameters validParams();

  MortarUserObject(const InputParameters & parameters);

protected:

  /// Reference to the EquationSystem object
  SystemBase & _sys;

  /// the Moose mesh
  MooseMesh & _mesh;

  /// the higher-dimensional secondary face element
  const Elem * const & _interior_secondary_elem;

  /// the higher-dimensional primary face element
  const Elem * const & _interior_primary_elem;

  /// the lower-dimensional secondary element
  const Elem * const & _lower_secondary_elem;

  /// The primary face lower dimensional element (not the mortar element!). The mortar element
  /// lives on the secondary side of the mortar interface and *may* correspond to \p
  /// _lower_secondary_elem under the very specific circumstance that the nodes on the primary side
  /// of the mortar interface exactly project onto the secondary side of the mortar interface. In
  /// general projection of primary nodes will split the face elements on the secondary side of the
  /// interface. It is these split elements that are the mortar segment mesh elements
  Elem const * const & _lower_primary_elem;

  /// The current element volume (available during all execute functions)
  const Real & _interior_secondary_volume;

  /// Current side of the current element (available during executeOnInternalSide() and
  /// executeOnBoundary() and executeOnInterface())
  const unsigned int & _current_side;

  /// Current side of the current element (available during executeOnInternalSide() and
  /// executeOnBoundary() and executeOnInterface())
  const Elem * const & _current_side_elem;

  /// Current side volume (available during executeOnInternalSide() and executeOnBoundary() and
  /// executeOnInterface())
  const Real & _current_side_volume;

  /// the neighboring element's volume (available during executeOnInternalSide() and
  /// executeOnInterface())
  const Real & _interior_primary_volume;

  /// The boundary ID (available during executeOnBoundary() and executeOnInterface())
  const BoundaryID & _current_boundary_id;

  /// the normals along the primary face
  const MooseArray<Point> & _normals_primary;

  /// Tangent vectors on the secondary faces (libmesh)
  const MooseArray<std::vector<Point>> & _tangents;

  /// Member for handling change of coordinate systems (xyz, rz, spherical)
  const MooseArray<Real> & _coord;

  /// The quadrature points in physical space
  const std::vector<Point> & _q_point;

};
