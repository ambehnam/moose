//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MortarConstraint.h"
#include "DerivativeMaterialInterface.h"
#include "MooseVariableScalar.h"
#include "Assembly.h"

/// Base class for implementing Variational Multiscale Nitsche (VMN)
/// Thermal (T) mechanics periodic boundary conditions 2D, and 3D.
/// Constructed in the spirit of the "Heat Diffusion" kernel.
///
/// This class provides the base for computing the residual that couples
/// the macro heat flux and the boundary temperature jump, and the off-
/// diagonal Jacobian terms
///

class TestPeriodic : public DerivativeMaterialInterface<MortarConstraint>
{
public:
  static InputParameters validParams();
  TestPeriodic(const InputParameters & parameters);

protected:
  /**
   * compute the scalar residual
   */
  virtual void computeResidualScalar() override;

  /**
   * Method for computing the scalar Jacobian
   */
  virtual void computeJacobianScalar() override;

  virtual void precalculateResidual() override;
  virtual void precalculateJacobian() override;

  using MortarConstraint::computeOffDiagJacobianScalar;
  /**
   * compute jacobian block with respect to a scalar variable for primary/secondary/lower
   */
  void computeOffDiagJacobianScalar(Moose::MortarType mortar_type, unsigned int jvar) override;

  Real computeQpResidual(const Moose::MortarType mortar_type) override;
  Real computeQpResidualScalar();
  Real computeQpResidualScalarScalar();
  Real computeQpJacobian(Moose::ConstraintJacobianType /*jacobian_type*/,
                         unsigned int /*jvar*/) override
  {
    return 0;
  };
  Real computeQpJacobianScalarScalar();

  /**
   * compute the jacobian at the quadrature points with respect to a scalar variable
   */
  virtual Real computeQpBaseJacobian(Moose::MortarType mortar_type, unsigned int jvar);
  virtual Real computeQpConstraintJacobian(Moose::MortarType mortar_type, unsigned int jvar);

  // Compute T jump and heat flux average/jump
  void precalculateMaterial();
  // Compute four stability tensors
  void precalculateStability();

protected:
  /// the temperature jump in global and interface coordiantes;
  /// TM-analogy: _displacement_jump_global, _interface_displacement_jump
  ///@{
  Real _temp_jump_global;
  ///@}

  /// The four stability parameters from the VMDG method
  ///@{
  Real _tau_s;
  ///@}

  /// The unknown scalar variable ID
  const unsigned int _kappa_var;

  /// Order of the homogenization variable, used in several places
  const unsigned int _k_order;

  /// Pointer to kappa variable instance
  const MooseVariableScalar * const _kappa_var_ptr;

  /// The unknownscalar variable
  const VariableValue & _kappa;

  /// The controlled scalar variable ID
  const unsigned int _kappa_aux_var;

  /// Order of the homogenization variable, used in several places
  const unsigned int _ka_order;

  /// The controlled scalar variable
  const VariableValue & _kappa_aux;

  /// Used internally to iterate over each scalar component
  unsigned int _h;
  unsigned int _l;

  const Real & _current_elem_volume;

  const Real & _current_side_volume;

  /// Input property to allow user modifying penalty parameter
  const Real _pen_scale;

private:
  /// hard code the penalty for now
  const Real pencoef = 1.0;
};
