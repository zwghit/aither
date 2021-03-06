/*  This file is part of aither.
    Copyright (C) 2015-17  Michael Nucci (michael.nucci@gmail.com)

    Aither is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Aither is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <iostream>     // cout
#include <cstdlib>      // exit()
#include "eos.hpp"

using std::cout;
using std::endl;
using std::cerr;

// Member functions for idealGas class
// These functions calculate values using the ideal gas equation of state
// P = rho * R * T (for Navier-Stokes) or P = (g-1) * rho * e (for Euler)
double idealGas::Pressure(const double &rho, const double &specEn) const {
  return (gamma_ - 1.0) * rho * specEn;
}

double idealGas::PressFromEnergy(const double &rho, const double &energy,
                                 const double &vel) const {
  return (gamma_ - 1.0) * rho * (energy - 0.5 * vel * vel);
}

double idealGas::Density(const double &pressure,
                         const double &specEn) const {
  return pressure / ((gamma_ - 1.0) * specEn);
}

double idealGas::SpecEnergy(const double &pressure,
                            const double &rho) const {
  return pressure / ((gamma_ - 1.0) * rho);
}

double idealGas::Energy(const double &specEn, const double &vel) const {
  return specEn + 0.5 * vel * vel;
}

double idealGas::Enthalpy(const double &energy, const double &pressure,
                          const double &rho) const {
  return energy + pressure / rho;
}

double idealGas::SoS(const double &pressure, const double &rho) const {
  return sqrt(gamma_ * pressure / rho);
}

double idealGas::Temperature(const double &pressure,
                             const double &rho) const {
  return pressure * gamma_ / rho;
}

// Functions for sutherland class
double sutherland::Viscosity(const double &t) const {
  // Dimensionalize temperature
  const auto temp = t * tRef_;

  // Calculate viscosity
  const auto mu = (cOne_ * pow(temp, 1.5)) / (temp + S_);

  // Nondimensionalize viscosity
  return (mu / muRef_);
}

double sutherland::EffectiveViscosity(const double &t) const {
  // Get viscosity and scale
  return this->Viscosity(t) * scaling_;
}

double sutherland::Lambda(const double &mu) const {
  // Calculate lambda (2nd coeff of viscosity)
  return bulkVisc_ - (2.0 / 3.0) * mu;
}
