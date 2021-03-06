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

#ifndef INPUTHEADERDEF  // only if the macro INPUTHEADERDEF is not defined
                        // execute these lines of code
#define INPUTHEADERDEF  // define the macro

#include <vector>                   // vector
#include <string>                   // string
#include <set>                      // set
#include <memory>                   // unique_ptr
#include "vector3d.hpp"
#include "boundaryConditions.hpp"
#include "inputStates.hpp"
#include "macros.hpp"

using std::vector;
using std::string;
using std::set;
using std::unique_ptr;

// forward class declaration
class turbModel;
class idealGas;

class input {
  string simName_;  // simulation name
  string restartName_;  // restart file name
  string gName_;  // grid file name
  double dt_;  // time step
  int iterations_;  // number of iterations

  // variable names that are regcognized by the input file parser
  set<string> vars_;

  double pRef_;  // reference pressure
  double rRef_;  // reference density
  double lRef_;  // reference length
  vector3d<double> vRef_;  // reference velocity
  double gamma_;  // ratio of specific heats
  double gasConst_;  // gas constant of fluid
  vector<boundaryConditions> bc_;  // vector of boundary conditions for each
                                  // block
  string timeIntegration_;  // time integration method
  double cfl_;  // cfl number for local time stepping
  string faceReconstruction_;  // face reconstruction method
  string viscousFaceReconstruction_;  // face reconstruction for viscous flux
  double kappa_;  // kappa paramenter for MUSCL face reconstruction
  string limiter_;  // limiter to use in higher order calculations
  int outputFrequency_;  // how often to output results
  string equationSet_;  // which set of equations to solver Euler/Navier-Stokes
  double tRef_;  // reference temperature
  string matrixSolver_;  // matrix solver to solve Ax=b
  int matrixSweeps_;  // number of sweeps for matrix solver
  double matrixRelaxation_;  // relaxation parameter for matrix solver
  double timeIntTheta_;  // beam and warming time integration parameter
  double timeIntZeta_;  // beam and warming time integration parameter
  int nonlinearIterations_;  // number of nonlinear iterations for time accurate
                            // scheme
  double cflMax_;  // maximum cfl_ value
  double cflStep_;  // cfl_ step size for ramp
  double cflStart_;  // starting cfl_ number
  string invFluxJac_;  // inviscid flux jacobian
  double dualTimeCFL_;  // cfl_ number for dual time
  string inviscidFlux_;  // scheme for inviscid flux calculation
  string decompMethod_;  // method of decomposition for parallel problems
  string turbModel_;  // turbulence model
  int restartFrequency_;  // how often to output restart data
  int iterationStart_;  // starting number for iterations

  set<string> outputVariables_;  // variables to output

  vector<icState> ics_;  // initial conditions
  vector<unique_ptr<inputState>> bcStates_;  // information for boundary conditions

 public:
  // constructor
  input(const string &, const string &);

  // move constructor and assignment operator
  input(input&&) noexcept = default;
  input& operator=(input&&) noexcept = default;

  // copy constructor and assignment operator
  input(const input&) = default;
  input& operator=(const input&) = default;

  // member functions
  string SimName() const {return simName_;}
  string SimNameRoot() const;
  string RestartName() const {return restartName_;}
  bool IsRestart() const {return restartName_ != "none";}
  string GridName() const {return gName_;}

  double Dt() const {return dt_;}

  int Iterations() const {return iterations_;}
  void SetIterationStart(const int &nn) {iterationStart_ = nn;}
  int IterationStart() const {return iterationStart_;}

  double PRef() const {return pRef_;}
  double RRef() const {return rRef_;}
  double LRef() const {return lRef_;}
  double TRef() const {return tRef_;}
  vector3d<double> VelRef() const {return vRef_;}
  double ARef(const idealGas &) const;

  double Gamma() const {return gamma_;}
  double R() const {return gasConst_;}

  boundaryConditions BC(const int &ind) const {return bc_[ind];}
  vector<boundaryConditions> AllBC() const {return bc_;}
  int NumBC() const {return bc_.size();}

  string TimeIntegration() const {return timeIntegration_;}
  bool IsMultilevelInTime() const {return timeIntegration_ == "bdf2";}

  double CFL() const {return cfl_;}
  void CalcCFL(const int &i);

  double Kappa() const {return kappa_;}
  string FaceReconstruction() const {return faceReconstruction_;}
  string ViscousFaceReconstruction() const {return viscousFaceReconstruction_;}
  bool UsingConstantReconstruction() const {
    return faceReconstruction_ == "constant";
  }
  bool UsingMUSCLReconstruction() const;
  bool UsingHigherOrderReconstruction() const {
    return faceReconstruction_ == "weno" || faceReconstruction_ == "wenoZ";
  }

  string Limiter() const {return limiter_;}

  int OutputFrequency() const {return outputFrequency_;}
  int RestartFrequency() const {return restartFrequency_;}
  set<string> OutputVariables() const {return outputVariables_;}

  bool WriteOutput(const int &nn) const {return (nn + 1) % outputFrequency_ == 0;}
  bool WriteRestart(const int &nn) const {
    return (restartFrequency_ == 0) ? false : (nn + 1) % restartFrequency_ == 0;
  }

  string EquationSet() const {return equationSet_;}

  string MatrixSolver() const {return matrixSolver_;}
  int MatrixSweeps() const {return matrixSweeps_;}
  double MatrixRelaxation() const {return matrixRelaxation_;}
  bool MatrixRequiresInitialization() const;

  double Theta() const {return timeIntTheta_;}
  double Zeta() const {return timeIntZeta_;}

  int NonlinearIterations() const {return nonlinearIterations_;}

  double CFLMax() const {return cflMax_;}
  double CFLStep() const {return cflStep_;}
  double CFLStart() const {return cflStart_;}

  string InvFluxJac() const {return invFluxJac_;}

  double DualTimeCFL() const {return dualTimeCFL_;}

  string InviscidFlux() const {return inviscidFlux_;}

  string DecompMethod() const {return decompMethod_;}
  string TurbulenceModel() const {return turbModel_;}

  int NumVars() const {return vars_.size();}
  int NumVarsOutput() const {return outputVariables_.size();}
  int NumEquations() const;
  int NumFlowEquations() const {return NUMFLOWVARS;}
  int NumTurbEquations() const;

  void ReadInput(const int &);

  bool IsImplicit() const;
  bool IsViscous() const;
  bool IsTurbulent() const;
  bool IsBlockMatrix() const;

  string OrderOfAccuracy() const;

  unique_ptr<turbModel> AssignTurbulenceModel() const;

  void CheckNonlinearIterations();
  void CheckOutputVariables();
  void CheckTurbulenceModel() const;

  double ViscousCFLCoefficient() const;

  int NumberGhostLayers() const;

  icState ICStateForBlock(const int &) const;
  const unique_ptr<inputState> & BCData(const int &) const;

  bool IsWenoZ() const {return this->FaceReconstruction() == "wenoZ";}

  // destructor
  ~input() noexcept {}
};

// function declarations
void PrintTime();

#endif
