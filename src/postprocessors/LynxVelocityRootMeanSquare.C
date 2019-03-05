/******************************************************************************/
/*                       LYNX, a MOOSE-based application                      */
/*                                                                            */
/*          Copyright (C) 2017 by Antoine B. Jacquey and Mauro Cacace         */
/*             GFZ Potsdam, German Research Centre for Geosciences            */
/*                                                                            */
/*    This program is free software: you can redistribute it and/or modify    */
/*    it under the terms of the GNU General Public License as published by    */
/*      the Free Software Foundation, either version 3 of the License, or     */
/*                     (at your option) any later version.                    */
/*                                                                            */
/*       This program is distributed in the hope that it will be useful,      */
/*       but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/*        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the       */
/*                GNU General Public License for more details.                */
/*                                                                            */
/*      You should have received a copy of the GNU General Public License     */
/*    along with this program. If not, see <http://www.gnu.org/licenses/>     */
/******************************************************************************/

#include "LynxVelocityRootMeanSquare.h"
#include "Function.h"

registerMooseObject("MooseApp", LynxVelocityRootMeanSquare);

template <>
InputParameters
validParams<LynxVelocityRootMeanSquare>()
{
  InputParameters params = validParams<ElementIntegralPostprocessor>();
  params.addClassDescription("Calculates the mean square root of the velocity vector.");
  params.addRequiredCoupledVar("velocities", "The string of velocities that advect the problem.");
  return params;
}

LynxVelocityRootMeanSquare::LynxVelocityRootMeanSquare(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters), _n_vel(coupledComponents("velocities")), _vel(3)
{
  // Checking for consistency between mesh size and length of the provided displacements vector
  if (_n_vel != _mesh.dimension())
    mooseError("The number of variables supplied in 'velocities' must match the mesh dimension.");

  // Fetch coupled variables and gradients (as stateful properties if necessary)
  for (unsigned int i = 0; i < _n_vel; ++i)
    _vel[i] = &coupledValue("velocities", i);

  // Set unused dimensions to zero
  for (unsigned i = _n_vel; i < 3; ++i)
    _vel[i] = &_zero;
}

Real
LynxVelocityRootMeanSquare::getValue()
{
  return std::sqrt(ElementIntegralPostprocessor::getValue());
}

Real
LynxVelocityRootMeanSquare::computeQpIntegral()
{
  RealVectorValue vel((*_vel[0])[_qp], (*_vel[1])[_qp], (*_vel[2])[_qp]);

  return vel.norm_sq();
}