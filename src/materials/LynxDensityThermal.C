/******************************************************************************/
/*                            This file is part of                            */
/*                       LYNX, a MOOSE-based application                      */
/*                    Lithosphere dYnamic Numerical toolboX                   */
/*                                                                            */
/*          Copyright (C) 2017 by Antoine B. Jacquey and Mauro Cacace         */
/*             GFZ Potsdam, German Research Centre for Geosciences            */
/*                                                                            */
/*                Licensed under GNU General Public License 3,                */
/*                       please see LICENSE for details                       */
/*                  or http://www.gnu.org/licenses/gpl.html                   */
/******************************************************************************/

#include "LynxDensityThermal.h"

registerMooseObject("LynxApp", LynxDensityThermal);

template <>
InputParameters
validParams<LynxDensityThermal>()
{
  InputParameters params = validParams<LynxDensityBase>();
  params.addClassDescription(
      "Material calculating densities as a simple linear function of temperature.");
  params.addRequiredCoupledVar("temperature", "The temperature variable.");
  params.addParam<std::vector<Real>>("beta_fluid", "The fluid thermal expansion coefficient.");
  params.addParam<std::vector<Real>>("beta_solid", "The solid thermal expansion coefficient.");
  params.addParam<Real>("reference_temperature", 0.0, "The reference temperature.");
  params.addParam<FunctionName>("reference_temperature_fct",
                                "The reference temperature given by a function.");
  return params;
}

LynxDensityThermal::LynxDensityThermal(const InputParameters & parameters)
  : LynxDensityBase(parameters),
    _temperature(coupledValue("temperature")),
    _drho_dtemp(declareProperty<Real>("drho_dtemp")),
    _dinvrho_dtemp(declareProperty<Real>("dinvrho_dtemp")),
    _beta_fluid(isParamValid("beta_fluid") ? getLynxParam<Real>("beta_fluid")
                                           : std::vector<Real>(_n_composition, 0.0)),
    _beta_solid(isParamValid("beta_solid") ? getLynxParam<Real>("beta_solid")
                                           : std::vector<Real>(_n_composition, 0.0)),
    _temp_ref(getParam<Real>("reference_temperature")),
    _temp_ref_fct(isParamValid("reference_temperature_fct")
                      ? &getFunction("reference_temperature_fct")
                      : NULL)
{
}

void
LynxDensityThermal::computeQpProperties()
{
  computeQpGravity();

  Real temp_ref = _temp_ref;
  if (_temp_ref_fct)
    temp_ref = _temp_ref_fct->value(_t, _q_point[_qp]);

  _rho_f[_qp] = averageProperty(_fluid_density) *
                (1.0 - averageProperty(_beta_fluid) * (_temperature[_qp] - temp_ref));
  _rho_s[_qp] = averageProperty(_solid_density) *
                (1.0 - averageProperty(_beta_solid) * (_temperature[_qp] - temp_ref));

  _rho_b[_qp] = _porosity[_qp] * _rho_f[_qp] + (1.0 - _porosity[_qp]) * _rho_s[_qp];
  _reference_rho_b[_qp] = _rho_b[_qp];

  Real drho_f_dtemp = -1.0 * averageProperty(_beta_fluid) * averageProperty(_fluid_density);
  Real drho_s_dtemp = -1.0 * averageProperty(_beta_solid) * averageProperty(_solid_density);

  _drho_dtemp[_qp] = _porosity[_qp] * drho_f_dtemp + (1.0 - _porosity[_qp]) * drho_s_dtemp;

  _dinvrho_dtemp[_qp] =
      (_rho_b[_qp] > 0.0) ? -1.0 / std::pow(_rho_b[_qp], 2.0) * _drho_dtemp[_qp] : 0.0;
}
