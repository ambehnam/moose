# KernelScalarBase

describe the purpose and the philosophy of the scalar augmentation class, repeating from the scalar kernel page.
A "Kernel" is a piece of physics. It can represent one or more operators or terms in the weak form of
a partial differential equation.  With all terms on the left-hand-side, their sum is referred to as
the "residual". The residual is evaluated at several integration quadrature points over the problem
domain. To implement your own physics in MOOSE, you create your own kernel by subclassing the MOOSE
`Kernel` class.

The kernel scalar augmentation system supports the use of [!ac](AD) for residual calculations, as such
there are two options for creating field-scalar coupling objects:
`KernelScalarBase` and `ADKernelScalarBase`. To further understand
automatic differentiation, please refer to the [automatic_differentiation/index.md] page for more
information.

##

mention how it gets implemented, through the list of method calls.
Say that these things are implemented similar to integrated BC lower and DG lower; and link to those
https://mooseframework.inl.gov/source/dgkernels/HFEMDiffusion.html
https://mooseframework.inl.gov/source/bcs/HFEMDirichletBC.html

mention jvar and svar loops. over all coupled objects
This means the philosophy beside how you use it, and then listing out the methods that you call to contribute different parts to the matrix comment and I also would be listing out the different variable types probably within a chart like I did in OneNote, so I can refer to each of them.
So for these guys you want to show about the rows of The matrix and so I need to think about it if it's a chart for that that lists the function name or what. I was thinking about using the weak form and using the the variable. Maybe maybe the variables could be the headers, but still what goes into the table. Maybe the weak form term goes into the table with an integration symbol. And then in a list you list out the variable types. And I guess for the mortar method then you need to indicate is it primary or secondary.
this is going to require some drafting and seeing what looks good and getting some feedback from them and looking at how well they have described the mortar method previously.

In a `Kernel` subclass the `computeQpResidual()` function +must+ be overridden.  This is where you
implement your PDE weak form terms.  For non-AD objects the following member functions can
optionally be overridden:

- `computeQpJacobian()`
- `computeQpOffDiagJacobian()`

These two functions provide extra information that can help the numerical solver(s) converge faster
and better.


As mentioned, the `computeQpResidual` method must be overridden for both flavors of kernels non-AD
and AD. The `computeQpResidual` method for the non-AD version, [`Diffusion`](/Diffusion.md), is
provided in [non-ad-residual].

!listing framework/src/kernels/Diffusion.C id=non-ad-residual
         re=Real\nDiffusion::computeQpResidual.*?}
         caption=The C++ weak-form residual statement of [weak-form] as implemented in the Diffusion kernel.

This object also overrides the `computeQpJacobian` method to define Jacobian term of [jacobian] as
shown in [non-ad-jacobian].


!listing framework/src/kernels/Diffusion.C id=non-ad-jacobian
         re=Real\nDiffusion::computeQpJacobian.*?}
         caption=The C++ weak-form Jacobian statement of [jacobian] as implemented in the Diffusion kernel.


mention that the test function of a scalar is "1", so it doesn't appear in the listing. Similarly, the trial function is "1", so you see that in the penalty term.

Mention kappa is the variable in the code, but it is renamed in the input file


The AD version of this object, [`ADDiffusion`](/ADDiffusion.md), relies on an optimized kernel object, as such it overrides `precomputeQpResidual` as follows.

!listing framework/src/kernels/ADDiffusion.C id=ad-residual
         re=ADDiffusion::precomputeQpResidual.*?}
         caption=The C++ pre-computed portions of the weak-form residual statement of [weak-form] as implemented in the ADDiffusion kernel.


including automatic differentiation description for those augmentation classes that I describe.
Mention AD version provided, and with warning for global only.


use this to talk about loops
Inside your Kernel class, you have access to several member variables for computing the
residual and Jacobian values in the above mentioned functions:

- `_i`, `_j`: indices for the current test and trial shape functions respectively.
- `_qp`: current quadrature point index.
- `_u`, `_grad_u`: value and gradient of the variable this Kernel operates on;
  indexed by `_qp` (i.e. `_u[_qp]`).
- `_test`, `_grad_test`: value ($\psi$) and gradient ($\nabla \psi$) of the
  test functions at the q-points; indexed by `_i` and then `_qp` (i.e., `_test[_i][_qp]`).
- `_phi`, `_grad_phi`: value ($\phi$) and gradient ($\nabla \phi$) of the
    trial functions at the q-points; indexed by `_j` and then `_qp` (i.e., `_phi[_j][_qp]`).
- `_q_point`: XYZ coordinates of the current quadrature point.
- `_current_elem`: pointer to the current element being operated on.




say that the user object version is probably faster because you're not accessing the global variables so often. But for objects that need the coupling, then this would be the easy way to go.
maybe for timing comparison, I should give an option in that kernel thing to in the drive class that does that constraint, to not do the scalar variable residual. Then you can run it on a big problem and compare the timing of doing the user object and the assembly locally.
and then you can let them discuss that, that would be my thought about asking if this is the most optimal way.
I would need to add this test case as well.
look and see if I can find what the heavy tests are and how you can skip that. So then we could have the timing test be stored in the framework.

Note:
Mention that displaced mesh features are not yet tested with this

Note:
Mention the coverage of the other scalar coupling is checked with tensor mechanics test file, and a developer could look in the header file for descriptions. List the test name, the location.

## Parameters

There are two required parameters the user must supply for a kernel derived
from `KernelScalarBase`:

- `scalar_variable`: the primary scalar variable of the kernel, for which assembly
  of the residual and Jacobian contributions will occur. It must be a `MooseScalarVariable`.
  This parameter may be renamed in a derived class to be more physically meaningful
- `coupled_scalar`: the name of the primary scalar variable, provided a second time
  to ensure that the dependency of the `Kernel` on this variable is detected. This
  parameter cannot be renamed.

If the `scalar_variable` parameter is not specified, then the derived class will behave
identically to a regular `Kernel`, namely without any scalar functionality. This feature
is useful if the scalar augmentation in inserted into a class structure with several
levels and not all derived members use scalar variables.

The duplicate parameter listing is shown below for the `ScalarLMKernel` object
with the `scalar_variable` parameter renamed to `kappa`:

!listing test/tests/kernels/scalar_kernel_constraint/scalar_constraint_kernel.i block=Kernels

There are also some optional parameters that can be supplied to
`KernelScalarBase` classes. They are:

- `compute_scalar_residuals`: Whether to compute scalar residuals. This
  will automatically be set to false if a `scalar_variable` parameter is not
  supplied. Other cases where the user may want to set this to false is during
  testing when the scalar variable is an `AuxVariable` and not a solution variable
  in the system.
- `compute_field_residuals`: Whether to compute residuals for the primal field
  variable. If several `KernelScalarBase` objects are used in the input file
  to compute different rows (i.e. different variables) of the global residual,
  then some objects can be targeted to field variable rows and others to scalar
  variable rows.

At present, either the `secondary_variable` or `primary_variable` parameter must be supplied.
