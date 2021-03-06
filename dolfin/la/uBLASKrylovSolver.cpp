// Copyright (C) 2006-2009 Garth N. Wells
//
// This file is part of DOLFIN.
//
// DOLFIN is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// DOLFIN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with DOLFIN. If not, see <http://www.gnu.org/licenses/>.
//
// Modified by Anders Logg 2006-2012
//
// First added:  2006-05-31
// Last changed: 2012-05-07

#include <dolfin/common/NoDeleter.h>
#include <dolfin/log/LogStream.h>
#include "uBLASILUPreconditioner.h"
#include "uBLASDummyPreconditioner.h"
#include "uBLASKrylovSolver.h"
#include "KrylovSolver.h"

using namespace dolfin;

//-----------------------------------------------------------------------------
std::map<std::string, std::string> uBLASKrylovSolver::methods()
{
  return { {"default",  "default Krylov method"},
           {"cg",       "Conjugate gradient method"},
           {"gmres",    "Generalized minimal residual method"},
           {"bicgstab", "Biconjugate gradient stabilized method"} };
}
//-----------------------------------------------------------------------------
std::map<std::string, std::string> uBLASKrylovSolver::preconditioners()
{
  return { {"default", "default preconditioner"},
           {"none",    "No preconditioner"},
           {"ilu",     "Incomplete LU factorization"} };
}
//-----------------------------------------------------------------------------
Parameters uBLASKrylovSolver::default_parameters()
{
  Parameters p(KrylovSolver::default_parameters());
  p.rename("ublas_krylov_solver");
  return p;
}
//-----------------------------------------------------------------------------
uBLASKrylovSolver::uBLASKrylovSolver(std::string method,
                                     std::string preconditioner) :
  _method(method),
  rtol(0.0),
  atol(0.0),
  div_tol(0.0),
  max_it(0),
  restart(0),
  report(false)
{
  // Set parameter values
  parameters = default_parameters();

  // Select and create default preconditioner
  select_preconditioner(preconditioner);
}
//-----------------------------------------------------------------------------
uBLASKrylovSolver::uBLASKrylovSolver(uBLASPreconditioner& pc)
  : _method("default"), _pc(reference_to_no_delete_pointer(pc)),
  rtol(0.0), atol(0.0), div_tol(0.0), max_it(0), restart(0),
  report(false)
{
  // Set parameter values
  parameters = default_parameters();
}
//-----------------------------------------------------------------------------
uBLASKrylovSolver::uBLASKrylovSolver(std::string method,
                                     uBLASPreconditioner& pc)
  : _method(method), _pc(reference_to_no_delete_pointer(pc)),
  rtol(0.0), atol(0.0), div_tol(0.0), max_it(0), restart(0),
  report(false)
{
  // Set parameter values
  parameters = default_parameters();
}
//-----------------------------------------------------------------------------
uBLASKrylovSolver::~uBLASKrylovSolver()
{
  // Do nothing
}
//-----------------------------------------------------------------------------
std::size_t uBLASKrylovSolver::solve(GenericVector& x, const GenericVector& b)
{
  dolfin_assert(_matA);
  dolfin_assert(_matP);

  // Try to first use operator as a uBLAS matrix
  if (has_type<const uBLASMatrix<ublas_sparse_matrix>>(*_matA))
  {
    std::shared_ptr<const uBLASMatrix<ublas_sparse_matrix>> A
      = as_type<const uBLASMatrix<ublas_sparse_matrix>>(_matA);
    std::shared_ptr<const uBLASMatrix<ublas_sparse_matrix>> P
      = as_type<const uBLASMatrix<ublas_sparse_matrix>>(_matP);
    dolfin_assert(A);
    dolfin_assert(P);

    return solve_krylov(*A,
                        as_type<uBLASVector>(x),
                        as_type<const uBLASVector>(b),
                        *P);
  }

  // If that fails, try to use it as a uBLAS linear operator
  if (has_type<const uBLASLinearOperator>(*_matA))
  {
    std::shared_ptr<const uBLASLinearOperator> A
      =  as_type<const uBLASLinearOperator>(_matA);
    std::shared_ptr<const uBLASLinearOperator> P
      =  as_type<const uBLASLinearOperator>(_matP);

    dolfin_assert(A);
    dolfin_assert(P);

    return solve_krylov(*A,
                        as_type<uBLASVector>(x),
                        as_type<const uBLASVector>(b),
                        *P);
  }

  return 0;
}
//-----------------------------------------------------------------------------
std::size_t uBLASKrylovSolver::solve(const GenericLinearOperator& A,
                                     GenericVector& x,
                                     const GenericVector& b)
{
  // Set operator
  std::shared_ptr<const GenericLinearOperator> Atmp(&A, NoDeleter());
  set_operator(Atmp);
  return solve(as_type<uBLASVector>(x), as_type<const uBLASVector>(b));
}
//-----------------------------------------------------------------------------
void uBLASKrylovSolver::select_preconditioner(std::string preconditioner)
{
  if (preconditioner == "none")
    _pc.reset(new uBLASDummyPreconditioner());
  else if (preconditioner == "ilu")
    _pc.reset(new uBLASILUPreconditioner(parameters));
  else if (preconditioner == "default")
    _pc.reset(new uBLASILUPreconditioner(parameters));
  else
  {
    warning("Requested preconditioner is not available for uBLAS Krylov solver. Using ILU.");
    _pc.reset(new uBLASILUPreconditioner(parameters));
  }
}
//-----------------------------------------------------------------------------
void uBLASKrylovSolver::read_parameters()
{
  // Set tolerances and other parameters
  rtol    = parameters["relative_tolerance"];
  atol    = parameters["absolute_tolerance"];
  div_tol = parameters["divergence_limit"];
  max_it  = parameters["maximum_iterations"];
  restart = parameters("gmres")["restart"];
  report  = parameters["report"];
}
//-----------------------------------------------------------------------------
