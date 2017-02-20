# Accurate Implicit Three-dimensional Efficient RANS (AITHER)

Master: [![Build Status](https://travis-ci.org/mnucci32/aither.svg?branch=master)](https://travis-ci.org/mnucci32/aither)
<br>
Develop: [![Build Status](https://travis-ci.org/mnucci32/aither.svg?branch=develop)](https://travis-ci.org/mnucci32/aither)

### About The code
This code is for a 3D Navier-Stokes computational fluid dynamics solver. It is a cell centered, structured solver,
using mulit-block structured grids in Plot3D format. It uses explicit and implicit time integration methods.
It uses MUSCL extrapolation to reconstruct the primative variables from the cell centers to the cell faces for 2nd
order accuracy. Higher order reconstruction is acheived with a 5th order WENO reconstruction for the inviscid fluxes,
and a 4th order central reconstruction for the viscous fluxes. The code uses the Roe flux difference splitting scheme
for the inviscid fluxes, and a central scheme for the viscous fluxes. It is second order accurate in both space and time.

### Current Status
The code is 2nd order accurate in space and time. Available explicit time integration methods are forward euler
(1st order) and a minimum storage four stage Runge-Kutta method (2nd order). The implicit solver (LU-SGS,
BLU-SGS, DPLUR, BDPLUR) is implemented for implicit time integration. Dual time stepping is implemented for
time accuracy in the implicit solver. Available implicit time integrations methods come from the Beam and
Warming family of methods and are the implicit euler (1st order), Crank-Nicholson (2nd order), and BDF2
(2nd order) methods. The code has been thoroughly commented. It has been made parallel using MPI. For RANS simulations 
the Wilcox K-Omega 2006 and SST 2003 turbulence models are available. For detatched eddy simulations, the SST-DES
turbulence model is available. 

### To Do List
* Add WALE and Smagorinsky subgrid scale models for LES
* Add Couette flow regression test for isothermal wall, moving wall, periodic boundary conditions
* Add wall functions for turbulence models
* Add multigrid scheme for improved convergence

### Dependencies
* MPI - OpenMPI and MPICH have both been used in the past. Aither is currently developed with OpenMPI
* C++ compiler with C++14 support
* Cmake - Cmake only depends on a C++ compiler

### How To compile
Aither is compiled and installed with the standard cmake process.

```bash
cmake -DCMAKE_INSTALL_PREFIX=/path/to/installation -DCMAKE_BUILD_TYPE=release /path/to/source
make
make install
```

Cmake will automatically look for an MPI package. To specify a specific installation, set *-DMPI_DIR* to the
MPI installation directory. In addition to *release*, other supported build types are *debug*, *profile*,
*relwithdebinfo*, and *minsizerel*.

### How To Run
```bash
mpirun -np 1 aither inputFile.inp [restartFile.rst] > outputFile.out 2> errorFile.err &
```
The restart file argument is optional.

### Visualizing Results
Aither writes out Plot3D function files (\*.fun), as well as a Plot3D meta files (\*.p3d) that can be visualized
in [ParaView](www.paraview.org). 