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

#include <chrono>     // timing capability
#include <iostream>   // cout
#include <iomanip>    // put_time
#include <fstream>    // ifstream
#include <cstdlib>    // exit()
#include <sstream>    // istringstream
#include <iterator>   // istring_iterator
#include <memory>     // make_unique
#include <algorithm>  // min
#include <string>
#include <vector>
#include "input.hpp"
#include "turbulence.hpp"
#include "inputStates.hpp"
#include "eos.hpp"
#include "macros.hpp"

using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::cerr;
using std::istringstream;
using std::istream_iterator;
using std::unique_ptr;
using std::stoi;
using std::stod;

// constructor for input class
// initialize vector to have length of number of acceptable inputs to the code
input::input(const string &name, const string &resName) : simName_(name),
                                                          restartName_(resName) {
  // default values for each variable
  gName_ = "";
  dt_ = -1.0;
  iterations_ = 1;
  pRef_ = -1.0;
  rRef_ = -1.0;
  lRef_ = 1.0;
  vRef_ = {1.0, 0.0, 0.0};
  gamma_ = 1.4;
  gasConst_ = 287.058;
  bc_ = vector<boundaryConditions>(1);
  timeIntegration_ = "explicitEuler";
  cfl_ = -1.0;
  faceReconstruction_ = "constant";
  viscousFaceReconstruction_ = "central";
  kappa_ = -2.0;  // default to value outside of range to tell if higher
                  // order or constant method should be used
  limiter_ = "none";
  outputFrequency_ = 1;
  equationSet_ = "euler";
  tRef_ = -1.0;
  matrixSolver_ = "lusgs";
  matrixSweeps_ = 1;
  matrixRelaxation_ = 1.0;  // default is symmetric Gauss-Seidel
                            // with no overrelaxation
  timeIntTheta_ = 1.0;  // default results in implicit euler
  timeIntZeta_ = 0.0;  // default results in implicit euler
  nonlinearIterations_ = 1;  // default is 1 (steady)
  cflMax_ = 1.0;
  cflStep_ = 0.0;
  cflStart_ = 1.0;
  invFluxJac_ = "rusanov";  // default is approximate rusanov which is used
                            // with lusgs
  dualTimeCFL_ = -1.0;  // default value of -1; negative value means dual time
                       // stepping is not used
  inviscidFlux_ = "roe";  // default value is roe flux
  decompMethod_ = "cubic";  // default is cubic decomposition
  turbModel_ = "none";  // default turbulence model is none
  restartFrequency_ = 0;  // default to not write restarts
  iterationStart_ = 0;  // default to start from iteration zero

  // default to primative variables
  outputVariables_ = {"density", "vel_x", "vel_y", "vel_z", "pressure"};

  // keywords in the input file that the parser is looking for to define
  // variables
  vars_ = {"gridName",
           "timeStep",
           "iterations",
           "pressureRef",
           "densityRef",
           "lengthRef",
           "velocityRef",
           "gamma",
           "gasConstant",
           "timeIntegration",
           "faceReconstruction",
           "viscousFaceReconstruction",
           "limiter",
           "outputFrequency",
           "restartFrequency",
           "equationSet",
           "temperatureRef",
           "matrixSolver",
           "matrixSweeps",
           "matrixRelaxation",
           "nonlinearIterations",
           "cflMax",
           "cflStep",
           "cflStart",
           "inviscidFluxJacobian",
           "dualTimeCFL",
           "inviscidFlux",
           "decompositionMethod",
           "turbulenceModel",
           "outputVariables",
           "initialConditions",
           "boundaryStates",
           "boundaryConditions"};
}

// function to print the time
void PrintTime() {
  auto now = std::chrono::system_clock::now();
  auto nowOut = std::chrono::system_clock::to_time_t(now);
  cout << std::put_time(std::localtime(&nowOut), "%c") << endl;
}

// function to read the input file and return the data as a member of the input
// class
// read the input file
void input::ReadInput(const int &rank) {
  // rank -- rank of processor

  if (rank == ROOTP) {
    cout << "##################################################################"
            "#########################################################" << endl;
    PrintTime();
    cout << endl;
    cout << "Parsing input file " << simName_ << endl << endl;
    cout << "Solver Inputs" << endl;
  }

  // open input file
  ifstream inFile(simName_, ios::in);
  if (inFile.fail()) {
    cerr << "ERROR: Error in input::ReadInput(). Input file " << simName_
         << " did not open correctly!!!" << endl;
    exit(EXIT_FAILURE);
  }

  string line = "";
  auto readingBCs = 0;
  auto blk = 0;
  auto surfCounter = 0;
  auto numBCBlks = 0;
  vector<boundaryConditions> tempBC(1);
  auto lEnd = 0;
  auto numSurf = 0;

  while (getline(inFile, line)) {  // while there are still lines in the input
    // file, execute loop

    // remove leading and trailing whitespace and ignore comments
    line = Trim(line);

    if (line.length() > 0) {  // only proceed if line has data
      // split line at variable separator
      auto tokens = Tokenize(line, ":", 2);

      // search to see if first token corresponds to any keywords
      auto key = tokens[0];

      // if first token matches a keyword or reading boundary condtions
      if (vars_.find(key) != vars_.end() || readingBCs > 0) {
        // if not yet reading BCs (readingBCs == 0), set variable in input
        // class to corresponding value and print assignment to std out
        if (key == "gridName") {
          gName_ = tokens[1];
          if (rank == ROOTP) {
            cout << key << ": " << this->GridName() << endl;
          }
        } else if (key == "timeStep") {
          dt_ = stod(tokens[1]);  // double variable (stod)
          if (rank == ROOTP) {
            cout << key << ": " << this->Dt() << endl;
          }
        } else if (key == "iterations") {
          iterations_ = stoi(tokens[1]);
          if (rank == ROOTP) {
            cout << key << ": " << this->Iterations() << endl;
          }
        } else if (key == "pressureRef") {
          pRef_ = stod(tokens[1]);  // double variable (stod)
          if (rank == ROOTP) {
            cout << key << ": " << this->PRef() << endl;
          }
        } else if (key == "densityRef") {
          rRef_ = stod(tokens[1]);  // double variable (stod)
          if (rank == ROOTP) {
            cout << key << ": " << this->RRef() << endl;
          }
        } else if (key == "lengthRef") {
          lRef_ = stod(tokens[1]);  // double variable (stod)
          if (rank == ROOTP) {
            cout << key << ": " << this->LRef() << endl;
          }
        } else if (key == "velocityRef") {
          vRef_ = ReadVector(tokens[1]);
          if (rank == ROOTP) {
            cout << key << ": [" << this->VelRef() << "]" << endl;
          }
        } else if (key == "gamma") {
          gamma_ = stod(tokens[1]);  // double variable (stod)
          if (rank == ROOTP) {
            cout << key << ": " << this->Gamma() << endl;
          }
        } else if (key == "gasConstant") {
          gasConst_ = stod(tokens[1]);  // double variable (stod)
          if (rank == ROOTP) {
            cout << key << ": " << this->R() << endl;
          }
        } else if (key == "timeIntegration") {
          timeIntegration_ = tokens[1];
          if (this->TimeIntegration() == "implicitEuler") {
            timeIntTheta_ = 1.0;
            timeIntZeta_ = 0.0;
          } else if (this->TimeIntegration() == "crankNicholson") {
            timeIntTheta_ = 0.5;
            timeIntZeta_ = 0.0;
          } else if (this->TimeIntegration() == "bdf2") {
            timeIntTheta_ = 1.0;
            timeIntZeta_ = 0.5;
          }

          if (rank == ROOTP) {
            cout << key << ": " << this->TimeIntegration() << endl;
          }
        } else if (key == "faceReconstruction") {
          if (tokens[1] == "upwind") {
            faceReconstruction_ = tokens[1];
            kappa_ = -1.0;
          } else if (tokens[1] == "fromm") {
            faceReconstruction_ = tokens[1];
            kappa_ = 0.0;
          } else if (tokens[1] == "quick") {
            faceReconstruction_ = tokens[1];
            kappa_ = 0.5;
          } else if (tokens[1] == "central") {
            faceReconstruction_ = tokens[1];
            kappa_ = 1.0;
          } else if (tokens[1] == "thirdOrder") {
            faceReconstruction_ = tokens[1];
            kappa_ = 1.0 / 3.0;
          } else if (tokens[1] == "constant" || tokens[1] == "weno" ||
                     tokens[1] == "wenoZ") {
            faceReconstruction_ = tokens[1];
          } else {
            // face reconstruction method not recognized
            cerr << "ERROR: Error in input::ReadInput(). Face reconstruction "
                 << "method " << tokens[1] << " is not recognized!";
                exit(EXIT_FAILURE);
          }

          if (rank == ROOTP) {
            cout << key << ": " << this->FaceReconstruction() << endl;
          }
        } else if (key == "viscousFaceReconstruction") {
          if (tokens[1] == "central") {
            viscousFaceReconstruction_ = tokens[1];
          } else if (tokens[1] == "centralFourth") {
            viscousFaceReconstruction_ = tokens[1];
          } else {
            // face reconstruction method not recognized
            cerr << "ERROR: Error in input::ReadInput(). Viscous face "
                 << "reconstruction method " << tokens[1] << " is not recognized!";
                exit(EXIT_FAILURE);
          }

          if (rank == ROOTP) {
            cout << key << ": " << this->ViscousFaceReconstruction() << endl;
          }
        } else if (key == "limiter") {
          limiter_ = tokens[1];
          if (rank == ROOTP) {
            cout << key << ": " << this->Limiter() << endl;
          }
        } else if (key == "outputFrequency") {
          outputFrequency_ = stoi(tokens[1]);
          if (rank == ROOTP) {
            cout << key << ": " << this->OutputFrequency() << endl;
          }
        } else if (key == "restartFrequency") {
          restartFrequency_ = stoi(tokens[1]);
          if (rank == ROOTP) {
            cout << key << ": " << this->RestartFrequency() << endl;
          }
        } else if (key == "equationSet") {
          equationSet_ = tokens[1];
          if (rank == ROOTP) {
            cout << key << ": " << this->EquationSet() << endl;
          }
        } else if (key == "temperatureRef") {
          tRef_ = stod(tokens[1]);  // double variable (stod)
          if (rank == ROOTP) {
            cout << key << ": " << this->TRef() << endl;
          }
        } else if (key == "matrixSolver") {
          matrixSolver_ = tokens[1];
          if (rank == ROOTP) {
            cout << key << ": " << this->MatrixSolver() << endl;
          }
        } else if (key == "matrixSweeps") {
          matrixSweeps_ = stoi(tokens[1]);
          if (rank == ROOTP) {
            cout << key << ": " << this->MatrixSweeps() << endl;
          }
        } else if (key == "matrixRelaxation") {
          matrixRelaxation_ =
              stod(tokens[1]);  // double variable (stod)
          if (rank == ROOTP) {
            cout << key << ": " << this->MatrixRelaxation() << endl;
          }
        } else if (key == "nonlinearIterations") {
          nonlinearIterations_ = stoi(tokens[1]);
          if (rank == ROOTP) {
            cout << key << ": " << this->NonlinearIterations() << endl;
          }
        } else if (key == "cflMax") {
          cflMax_ = stod(tokens[1]);  // double  (stod)
          if (rank == ROOTP) {
            cout << key << ": " << this->CFLMax() << endl;
          }
        } else if (key == "cflStep") {
          cflStep_ = stod(tokens[1]);  // double (stod)
          if (rank == ROOTP) {
            cout << key << ": " << this->CFLStep() << endl;
          }
        } else if (key == "cflStart") {
          cflStart_ = stod(tokens[1]);  // double variable (stod)
          if (rank == ROOTP) {
            cout << key << ": " << this->CFLStart() << endl;
          }
        } else if (key == "inviscidFluxJacobian") {
          invFluxJac_ = tokens[1];
          if (rank == ROOTP) {
            cout << key << ": " << this->InvFluxJac() << endl;
          }
        } else if (key == "dualTimeCFL") {
          dualTimeCFL_ = stod(tokens[1]);  // double variable (stod)
          if (rank == ROOTP) {
            cout << key << ": " << this->DualTimeCFL() << endl;
          }
        } else if (key == "inviscidFlux") {
          inviscidFlux_ = tokens[1];
          if (rank == ROOTP) {
            cout << key << ": " << this->InviscidFlux() << endl;
          }
        } else if (key == "decompositionMethod") {
          decompMethod_ = tokens[1];
          if (rank == ROOTP) {
            cout << key << ": " << this->DecompMethod() << endl;
          }
        } else if (key == "turbulenceModel") {
          turbModel_ = tokens[1];
          if (rank == ROOTP) {
            cout << key << ": " << this->TurbulenceModel() << endl;
          }
        } else if (key == "outputVariables") {
          // clear default variables from set
          outputVariables_.clear();
          auto specifiedVars = ReadStringList(inFile, tokens[1]);
          for (auto &vars : specifiedVars) {
            outputVariables_.insert(vars);
          }
          if (rank == ROOTP) {
            cout << key << ": <";
            auto count = 0U;
            auto numChars = 0U;
            for (auto &vars : outputVariables_) {
              if (count == outputVariables_.size() - 1) {
                cout << vars << ">" << endl;
              } else {
                cout << vars << ", ";
                numChars += vars.length();
                if (numChars >= 50) {  // if more than 50 chars, go to next line
                  cout << endl << "                  ";
                }
              }
              count++;
            }
            cout << endl;
          }
        } else if (key == "initialConditions") {
          ics_ = ReadICList(inFile, tokens[1]);
          if (rank == ROOTP) {
            cout << key << ": <";
            for (auto ii = 0U; ii < ics_.size(); ++ii) {
              cout << ics_[ii];
              if (ii == ics_.size() - 1) {
                cout << ">" << endl;
              } else {
                cout << "," << endl << "                    ";
              }
            }
          }
        } else if (key == "boundaryStates") {
          bcStates_ = ReadBCList(inFile, tokens[1]);
          if (rank == ROOTP) {
            cout << key << ": <";
            for (auto ii = 0U; ii < bcStates_.size(); ++ii) {
              cout << *bcStates_[ii];
              if (ii == bcStates_.size() - 1) {
                cout << ">" << endl;
              } else {
                cout << "," << endl << "                 ";
              }
            }
          }

          // reading BCs
          // -------------------------------------------------------------
        } else if (key == "boundaryConditions" || readingBCs > 0) {
          // read in boundary conditions and assign to boundaryConditions class
          if (readingBCs == 0) {  // variable read must be number of blocks if
            // first line of BCs
            numBCBlks = stoi(tokens[1]);
            tempBC.resize(numBCBlks);  // resize vector holding BCs to correct
            // number of blocks
            readingBCs++;
            lEnd++;
          } else if (readingBCs == lEnd) {  // variables must be number of i,
            // j, k surfaces for each block
            // set number of i, j, k surfaces and resize vectors
            // boundary conditions are space delimited
            tokens = Tokenize(line, " ");
            tempBC[blk].ResizeVecs(stoi(tokens[0]), stoi(tokens[1]),
                                   stoi(tokens[2]));

            // set total number of surfaces
            numSurf = stoi(tokens[0]) + stoi(tokens[1]) + stoi(tokens[2]);

            // set new target for when block data is finished
            lEnd += (numSurf + 1);
            readingBCs++;

          } else {  // assign BC block variables
            // boundary conditions are space delimited
            tokens = Tokenize(line, " ");
            tempBC[blk].AssignFromInput(surfCounter, tokens);

            surfCounter++;
            if (surfCounter == numSurf) {  // at end of block data, increment
              // block index, reset surface
              // counter
              blk++;
              surfCounter = 0;
            }

            readingBCs++;
          }

          // if block counter reaches number of blocks, BCs are finished (b/c
          // counter starts at 0), so assign BCs and write them out
          if (blk == numBCBlks) {
            bc_ = tempBC;
            if (rank == ROOTP) {
              cout << "boundaryConditions: " << this->NumBC() << endl;
              for (auto ll = 0; ll < this->NumBC(); ll++) {
                cout << "Block: " << ll << endl;
                cout << this->BC(ll) << endl;
              }
            }
          }
        }
      }
    }
  }


  // input file sanity checks
  this->CheckNonlinearIterations();
  this->CheckOutputVariables();
  this->CheckTurbulenceModel();

  if (rank == ROOTP) {
    cout << endl;
    cout << "Input file parse complete" << endl;
    cout << "##################################################################"
            "#########################################################" << endl
         << endl;
  }
}

// member function to calculate the cfl value for the step from the starting,
// ending, and step values
void input::CalcCFL(const int &ii) {
  cfl_ = std::min(cflStart_ + ii * cflStep_, cflMax_);
}

// member function to determine number of turbulence equations
int input::NumTurbEquations() const {
  auto numEqns = 0;
  if (this->IsTurbulent()) {
    numEqns = 2;
  }
  return numEqns;
}

// member function to determine number of equations to solver for
int input::NumEquations() const {
  auto numEqns = 0;
  if ((equationSet_ == "euler") ||
      (equationSet_ == "navierStokes")) {
    numEqns = this->NumFlowEquations();
  } else if (equationSet_ == "rans") {
    numEqns = this->NumFlowEquations() + this->NumTurbEquations();
  } else {
    cerr << "ERROR: Equations set is not recognized. Cannot determine number "
            "of equations!" << endl;
  }

  return numEqns;
}

// member function to determine of method is implicit or explicit
bool input::IsImplicit() const {
  if (timeIntegration_ == "implicitEuler" ||
      timeIntegration_ == "crankNicholson" ||
      timeIntegration_ == "bdf2") {
    return true;
  } else {
    return false;
  }
}

// member function to determine of method is vicous or inviscid
bool input::IsViscous() const {
  if (equationSet_ == "navierStokes" ||
      equationSet_ == "rans") {
    return true;
  } else {
    return false;
  }
}

// member function to determine of method is turbulent
bool input::IsTurbulent() const {
  if (equationSet_ == "rans") {
    return true;
  } else {
    return false;
  }
}

// member function to determine if solution should use a block matrix
bool input::IsBlockMatrix() const {
  if (this->IsImplicit() && (matrixSolver_ == "bdplur" ||
                             matrixSolver_ == "blusgs")) {
    return true;
  } else {
    return false;
  }
}

string input::OrderOfAccuracy() const {
  if (this->UsingConstantReconstruction()) {
    return "first";
  } else {
    return "second";
  }
}

bool input::UsingMUSCLReconstruction() const {
  auto muscl = false;
  if (faceReconstruction_ == "upwind" || faceReconstruction_ == "fromm" ||
      faceReconstruction_ == "quick" || faceReconstruction_ == "central" ||
      faceReconstruction_ == "thirdOrder") {
    muscl = true;
  }
  return muscl;
}

unique_ptr<turbModel> input::AssignTurbulenceModel() const {
  // define turbulence model
  unique_ptr<turbModel> turb(nullptr);
  if (turbModel_ == "none") {
    turb = unique_ptr<turbModel>{std::make_unique<turbNone>()};
  } else if (turbModel_ == "kOmegaWilcox2006") {
    turb = unique_ptr<turbModel>{std::make_unique<turbKWWilcox>()};
  } else if (turbModel_ == "sst2003") {
    turb = unique_ptr<turbModel>{std::make_unique<turbKWSst>()};
  } else {
    cerr << "ERROR: Error in input::AssignTurbulenceModel(). Turbulence model "
         << turbModel_ << " is not recognized!" << endl;
    exit(EXIT_FAILURE);
  }
  return turb;
}

// member function to return the name of the simulation without the file
// extension i.e. "myInput.inp" would return "myInput"
string input::SimNameRoot() const {
  return simName_.substr(0, simName_.find("."));
}

// member function to check validity of nonlinear iterations
void input::CheckNonlinearIterations() {
  if (timeIntegration_ == "rk4" && nonlinearIterations_ != 4) {
    cerr << "WARNING: For RK4 method, nonlinear iterations should be set to "
         << 4 << " changing value from " << nonlinearIterations_ << " to "
         << 4 << endl;
    nonlinearIterations_ = 4;
  }

  if (timeIntegration_ == "euler" && nonlinearIterations_ != 1) {
    cerr << "WARNING: For euler method, nonlinear iterations should be set to "
         << 1 << " changing value from " << nonlinearIterations_ << " to " << 1
         << endl;
    nonlinearIterations_ = 1;
  }
}

// member function to check validity of the requested output variables
void input::CheckOutputVariables() {
  for (auto var : outputVariables_) {
    if (!this->IsTurbulent()) {  // can't have turbulent varibles output
      if (var == "tke" || var == "sdr" || var == "viscosityRatio" ||
          var.find("tkeGrad_") != string::npos ||
          var.find("sdrGrad_") != string::npos || var == "resid_tke" ||
          var == "resid_sdr") {
        cerr << "WARNING: Variable " << var <<
            " is not available for laminar simulations." << endl;
        outputVariables_.erase(var);
      }

      if (!this->IsViscous()) {  // can't have viscous variables output
        if (var.find("velGrad_") != string::npos
            || var.find("tempGrad_") != string::npos) {
          cerr << "WARNING: Variable " << var <<
              " is not available for inviscid simulations." << endl;
          outputVariables_.erase(var);
        }
      }
    }
  }
}

// member function to check that turbulence model makes sense with equation set
void input::CheckTurbulenceModel() const {
  if (equationSet_ == "rans" && turbModel_ == "none") {
    cerr << "ERROR: If solving RANS equations, must specify turbulence model!"
         << endl;
    exit(EXIT_FAILURE);
  }
  if (equationSet_ != "rans" && turbModel_ != "none") {
    cerr << "ERROR: Turbulence models are only valid for the RANS equation set!"
         << endl;
    exit(EXIT_FAILURE);
  }
}


// member function to calculate the coefficient used to scale the viscous
// spectral radius in the time step calculation
double input::ViscousCFLCoefficient() const {
  auto coeff = 1.0;
  if (this->Kappa() == 1.0) {  // central
    coeff = 4.0;
  } else if (this->Kappa() == -2.0) {  // first order upwind
    coeff = 2.0;
  }
  return coeff;
}

bool input::MatrixRequiresInitialization() const {
  // initialize matrix if using DPLUR / BDPLUR, or if using LUSGS / BLUSGS with
  // more than one sweep
  return (matrixSolver_ == "dplur" || matrixSolver_ == "bdplur" ||
          matrixSweeps_ > 1) ? true : false;
}

int input::NumberGhostLayers() const {
  if (this->UsingConstantReconstruction()) {
    return 1;
  } else if (this->UsingMUSCLReconstruction()) {
    return 2;
  } else if (this->UsingHigherOrderReconstruction()) {
    return 3;
  } else {
    cerr << "ERROR: Problem with face reconstruction. Not using any of the "
         << "supported methods!" << endl;
    exit(EXIT_FAILURE);
  }
}

// member function to get the initial condition state for a given parent block
icState input::ICStateForBlock(const int &block) const {
  icState blockIC;
  auto foundExactMatch = false;
  auto foundDefaultMatch = false;

  // exact match trumps default
  for (auto &ic : ics_) {
    if (ic.Tag() == -1 && !foundExactMatch) {
      blockIC = ic;
      foundDefaultMatch = true;
    } else if (ic.Tag() == block) {
      blockIC = ic;
      foundExactMatch = true;
    }
  }

  // sanity check -- must specify default or exact match
  if (!foundDefaultMatch && !foundExactMatch) {
    cerr << "ERROR. Did not find default or specified initial condition state "
         << "for block " << block << endl;
    exit(EXIT_FAILURE);
  }

  return blockIC;
}

// member function to return boundary condition data index for a given tag
const unique_ptr<inputState> & input::BCData(const int &tag) const {
  for (auto &bcData : bcStates_) {
    if (bcData->Tag() == tag) {
      return bcData;
    }
  }

  // if function doesn't return, tag not found
  cerr << "ERROR. Could not find data for boundary condition tag " << tag << endl;
  exit(EXIT_FAILURE);
}

// member function to return reference speed of sound
double input::ARef(const idealGas &eos) const {
  return eos.SoS(pRef_, rRef_);
}
