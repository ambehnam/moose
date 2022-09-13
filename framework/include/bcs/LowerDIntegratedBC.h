//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"

class LowerDIntegratedBC : public IntegratedBC
{
public:
  static InputParameters validParams();

  LowerDIntegratedBC(const InputParameters & parameters);

  virtual const MooseVariable & variable() const override { return _var; }
  const MooseVariable & lowerDVariable() const { return _lowerd_var; }

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  virtual void computeOffDiagJacobianScalar(unsigned int jvar) override;

protected:
  /**
   * Method for computing the Lower part of residual at quadrature points.
   */
  virtual Real computeLowerDQpResidual() = 0;

  /**
   * Method for computing the LowerLower, PrimaryLower and LowerPrimary parts of Jacobian
   */
  virtual void computeLowerDJacobian(Moose::ConstraintJacobianType type);

  /**
   * Method for computing the LowerLower, PrimaryLower and LowerPrimary parts of Jacobian at
   * quadrature points
   */
  virtual Real computeLowerDQpJacobian(Moose::ConstraintJacobianType) = 0;

  /**
   * Method for computing an off-diagonal jacobian component
   */
  void computeLowerDOffDiagJacobian(Moose::ConstraintJacobianType type,
                                    const unsigned int jvar_num);

  /**
   * Method for computing an off-diagonal jacobian component at quadrature points.
   */
  virtual Real computeLowerDQpOffDiagJacobian(Moose::ConstraintJacobianType /*type*/,
                                             const MooseVariableFEBase & /*jvar*/)
  {
    return 0;
  }

  /**
   * Method for computing an off-diagonal jacobian scalar component at quadrature points.
   */
  virtual Real computeLowerDQpOffDiagJacobianScalar(const unsigned int /*jvar*/)
  {
    return 0;
  }

  /**
   * Put necessary evaluations depending on qp but independent of test functions here
   */
  virtual void initLowerDQpResidual() {}

  /**
   * Put necessary evaluations depending on qp but independent of test and shape functions here
   */
  virtual void initLowerDQpJacobian(Moose::ConstraintJacobianType /*type*/) {}

  /**
   * Put necessary evaluations depending on qp but independent of lower test and shape functions here for
   * off-diagonal Jacobian assembly
   */
  virtual void initLowerDQpOffDiagJacobian(Moose::ConstraintJacobianType /*type*/,
                                           const MooseVariableFEBase & /*jvar*/)
  {
  }

  /**
   * Put necessary evaluations depending on qp but independent of lower test and shape functions here for
   * off-diagonal Jacobian scalar assembly
   */
  virtual void initLowerDQpOffDiagJacobianScalar(const unsigned int /*jvar*/)
  {
  }

  /// Variable this kernel operates on
  const MooseVariable & _lowerd_var;
  /// Holds the current solution at the current quadrature point on the face.
  const VariableValue & _lambda;
  /// Shape functions
  const VariablePhiValue & _phi_lambda;
  /// test functions
  const VariableTestValue & _test_lambda;
};
