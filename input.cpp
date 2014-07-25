#include <iostream>     //cout
#include <fstream>      //ifstream
#include <cstdlib>      //exit()
#include <sstream>      //istringstream
#include <iterator>     //istring_iterator
#include <time.h>       //strftime
#include "input.h"

using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::cerr;
using std::istringstream;
using std::istream_iterator;

//constructor for input class
//initialize vector to have length of number of acceptable inputs to the code
input::input(): vars(17){
  //default values for each variable
  gName = "";
  dt = -1.0;
  iterations = 1;
  pRef = -1.0;
  rRef = -1.0;
  lRef = 1.0;
  gamma = 1.4;
  gasConst = 287.058;
  velRef.SetX(1.0);
  velRef.SetY(0.0);
  velRef.SetZ(0.0);
  vector<boundaryConditions> dummy(1);
  bc = dummy;
  timeIntegration = "explicitEuler";
  cfl = -1.0;
  kappa = -2.0;         //default to value outside of range to tell if higher order or constant method should be used
  limiter = "none";
  outputFrequency = 1;
  equationSet = "euler";
  tRef = -1.0;

  //keywords in the input file that the parser is looking for to define variables
  vars[0] = "gridName:";
  vars[1] = "timeStep:";
  vars[2] = "iterations:";
  vars[3] = "pressureRef:";
  vars[4] = "densityRef:";
  vars[5] = "lengthRef:";
  vars[6] = "gamma:";
  vars[7] = "gasConstant:";
  vars[8] = "velocity:";
  vars[9] = "timeIntegration:";
  vars[10] = "cfl:";
  vars[11] = "faceReconstruction:";
  vars[12] = "limiter:";
  vars[13] = "outputFrequency:";
  vars[14] = "equationSet:";
  vars[15] = "temperatureRef:";

  vars[16] = "boundaryConditions:";  //boundary conditions should be listed last
}

//member function to set vector holding boundary conditions for each block
void input::SetBCVec(const int &a){
  bc.resize(a);
}


//function to trim leading and trailing whitespace from a string, and also remove data after a comment
string trim(const string &s, const string &whitespace = " \t"){

  const string comment = "#";                                  //# is comment character for input file
  const unsigned int sBegin = s.find_first_not_of(whitespace);          //find index of first non whitespace character

  if (sBegin == string::npos){
    return ""; //string is empty
  }

  const int sEnd = s.find_last_not_of(whitespace);            //find index of last non whitespace character
  const int sRange = sEnd - sBegin +1;                        //range to trim string to
  string temp = s.substr(sBegin,sRange);

  const int tempComment = temp.find(comment);                 //find index of first comment character
  const int tempRange = tempComment - 0;                      //find range of string to return

  return temp.substr(0,tempRange);
}

//function to print the time
void PrintTime(){

  time_t rawtime;
  struct tm * timeinfo;

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  int bufLen = 100;
  char buffer [bufLen];
  strftime(buffer, bufLen, "%c", timeinfo);
  cout << buffer << endl;

}

//function to read the input file and return the data as a member of the input class
//read the input file
input ReadInput(const string &inputName){

  cout << "###########################################################################################################################" << endl;
  PrintTime();
  cout << endl;
  cout << "Parsing input file " << inputName << endl << endl;
  cout << "Solver Inputs" << endl;

  input inputVars;

  ifstream inFile;
  inFile.open(inputName.c_str(), ios::in);

  if (inFile.fail()) {
    cerr << "ERROR: Input file " << inputName << " did not open correctly!!!" << endl;
  }

  string line;
  unsigned int ii = 0;
  vector3d<double> temp;
  int readingBCs = 0;
  int jj = 0;
  int kk = 0;
  int surfCounter = 0;
  int numBCBlks = 0;
  vector<boundaryConditions> tempBC(1);

  while(getline(inFile,line)){

    line = trim(line);  //remove leading and trailing whitespace

    if (line[0] == '#') continue;  //skip line if first non-whitespace character is comment character #

    //split line into words
    istringstream buf(line);
    istream_iterator<string> beg(buf), end;
    vector<string> tokens(beg,end);
   
    //search to see if first token corresponds to any keywords
    if (tokens.size() >= 2) {
      for( ii=0; ii < inputVars.Vars().size(); ii++){

        if (tokens[0] == inputVars.Vars()[ii] || readingBCs > 0){
          if (ii==0 && readingBCs == 0){
            inputVars.SetGridName(tokens[1]);
            cout << inputVars.Vars()[ii] << " " << inputVars.GridName() << endl;
            continue;
          }
          else if (ii==1 && readingBCs == 0){
            inputVars.SetDt(atof(tokens[1].c_str()));                          //double variable (atof)
            cout << inputVars.Vars()[ii] << " " << inputVars.Dt() << endl;
            continue;
	  }
          else if (ii==2 && readingBCs == 0){
            inputVars.SetIterations(atoi(tokens[1].c_str()));
            cout << inputVars.Vars()[ii] << " " << inputVars.Iterations() << endl;
            continue;
	  }
          else if (ii==3 && readingBCs == 0){
            inputVars.SetPRef(atof(tokens[1].c_str()));                       //double variable (atof)
            cout << inputVars.Vars()[ii] << " " << inputVars.PRef() << endl;
            continue;
	  }
          else if (ii==4 && readingBCs == 0){
            inputVars.SetRRef(atof(tokens[1].c_str()));                       //double variable (atof)
            cout << inputVars.Vars()[ii] << " " << inputVars.RRef() << endl;
            continue;
	  }
          else if (ii==5 && readingBCs == 0){
            inputVars.SetLRef(atof(tokens[1].c_str()));                       //double variable (atof)
            cout << inputVars.Vars()[ii] << " " << inputVars.LRef() << endl;
            continue;
	  }
          else if (ii==6 && readingBCs == 0){
            inputVars.SetGamma(atof(tokens[1].c_str()));                      //double variable (atof)
            cout << inputVars.Vars()[ii] << " " << inputVars.Gamma() << endl;
            continue;
	  }
          else if (ii==7 && readingBCs == 0){
            inputVars.SetR(atof(tokens[1].c_str()));                         //double variable (atof)
            cout << inputVars.Vars()[ii] << " " << inputVars.R() << endl;
            continue;
	  }
          else if (ii==8 && readingBCs == 0){
            temp.SetX(atof(tokens[1].c_str()));                              //double variable (atof)
            temp.SetY(atof(tokens[2].c_str()));
            temp.SetZ(atof(tokens[3].c_str()));
            inputVars.SetVelRef(temp);
            cout << inputVars.Vars()[ii] << " " << inputVars.VelRef() << endl;
            continue;
	  }
          else if (ii==9 && readingBCs == 0){
            inputVars.SetTimeIntegration(tokens[1]);
            cout << inputVars.Vars()[ii] << " " << inputVars.TimeIntegration() << endl;
            continue;
          }
          else if (ii==10 && readingBCs == 0){
            inputVars.SetCFL(atof(tokens[1].c_str()));                       //double variable (atof)
            cout << inputVars.Vars()[ii] << " " << inputVars.CFL() << endl;
            continue;
          }
          else if (ii==11 && readingBCs == 0){
	    if (tokens[1] == "upwind"){
	      inputVars.SetKappa(-1.0);
	    }
	    else if (tokens[1] == "fromm"){
	      inputVars.SetKappa(0.0);
	    }
	    else if (tokens[1] == "quick"){
	      inputVars.SetKappa(0.5);
	    }
	    else if (tokens[1] == "central"){
	      inputVars.SetKappa(1.0);
	    }
	    else if (tokens[1] == "thirdOrder"){
	      inputVars.SetKappa(1.0/3.0);
	    }
	    else if (tokens[1] == "constant"){
	      inputVars.SetKappa(-2.0);
	    }
	    else{
	      inputVars.SetKappa(atof(tokens[1].c_str()));         //if string is not recognized, set kappa to number input
	    }

            cout << inputVars.Vars()[ii] << " " << tokens[1] << " kappa = " << inputVars.Kappa() << endl;
	    if ( (inputVars.Kappa() < -1.0) || (inputVars.Kappa() > 1.0) ){
	      cerr << "ERROR: Kappa value of " << inputVars.Kappa() << " is not valid! Choose a value between -1.0 and 1.0." << endl;
	      exit(0);
	    }
            continue;
          }
          else if (ii==12 && readingBCs == 0){
            inputVars.SetLimiter(tokens[1]);
            cout << inputVars.Vars()[ii] << " " << inputVars.Limiter() << endl;
            continue;
          }
          else if (ii==13 && readingBCs == 0){
            inputVars.SetOutputFrequency(atoi(tokens[1].c_str()));
            cout << inputVars.Vars()[ii] << " " << inputVars.OutputFrequency() << endl;
            continue;
	  }
          else if (ii==14 && readingBCs == 0){
            inputVars.SetEquationSet(tokens[1]);
            cout << inputVars.Vars()[ii] << " " << inputVars.EquationSet() << endl;
            continue;
          }
          else if (ii==15 && readingBCs == 0){
            inputVars.SetTRef(atof(tokens[1].c_str()));                       //double variable (atof)
            cout << inputVars.Vars()[ii] << " " << inputVars.TRef() << endl;
            continue;
	  }
          else if (ii==inputVars.Vars().size()-1 || readingBCs > 0){
	    //read in boundary conditions and assign to boundaryConditions class
	    if (readingBCs == 0) {  //variable read must be number of blocks
	      numBCBlks = atoi(tokens[1].c_str());
	      tempBC.resize(numBCBlks);
	      readingBCs++;
	    }	    
	    else if (readingBCs > 0 && readingBCs <= numBCBlks){ //variables must be number of i, j, k surfaces for each block
	      tempBC[jj].SetNumSurfI(atoi(tokens[0].c_str()));
	      tempBC[jj].SetNumSurfJ(atoi(tokens[1].c_str()));
	      tempBC[jj].SetNumSurfK(atoi(tokens[2].c_str()));

	      tempBC[jj].ResizeVecs(atoi(tokens[0].c_str()) + atoi(tokens[1].c_str()) + atoi(tokens[2].c_str()));
	      jj++;
	      readingBCs++;
	    }
	    else {  //assign block variables
	      int numSurf = tempBC[kk].NumSurfI() + tempBC[kk].NumSurfJ() + tempBC[kk].NumSurfK(); 

	      tempBC[kk].SetBCTypes(tokens[0], surfCounter);
	      tempBC[kk].SetIMin(atoi(tokens[1].c_str()), surfCounter);
	      tempBC[kk].SetIMax(atoi(tokens[2].c_str()), surfCounter);
	      tempBC[kk].SetJMin(atoi(tokens[3].c_str()), surfCounter);
	      tempBC[kk].SetJMax(atoi(tokens[4].c_str()), surfCounter);
	      tempBC[kk].SetKMin(atoi(tokens[5].c_str()), surfCounter);
	      tempBC[kk].SetKMax(atoi(tokens[6].c_str()), surfCounter);
	      tempBC[kk].SetTag(atoi(tokens[7].c_str()), surfCounter);

	      surfCounter++;
	      if (surfCounter == numSurf){
		kk++;
		surfCounter = 0;
	      }

	      readingBCs++;
	    }

	    if (kk == numBCBlks){
	      inputVars.SetBC(tempBC);
	      cout << inputVars.Vars()[inputVars.Vars().size()-1] << " " << inputVars.BC().size() << endl << endl;;
	      unsigned int ll = 0;
	      for ( ll = 0; ll < inputVars.BC().size(); ll++ ){
		cout << inputVars.BC()[ll] << endl;
	      }
	    }

	    break;
	  }

	}
      }
    }
    else{
      continue;
    }
  }

  cout << endl;
  cout << "Input file parse complete" << endl;
  cout << "###########################################################################################################################" << endl << endl;

  return inputVars;

}