/*debugger.h*/

//
// Debugger for nuPython
//
// Richard Zhang
// Northwestern University
// CS 211
//

#pragma once

#include <algorithm>
#include <vector>
#include "programgraph.h"
#include "execute.h"
#include "ram.h"

using namespace std;

class Debugger {
private:
  //const struct STMT* Program; //To store the program graph, a ptr to STMT.
  //struct STMT* iterator; 
  struct STMT* Program;
  string State;
  struct RAM* Memory; 
  vector<int> Breakpoints;
  struct STMT* Prev; 
  struct STMT* Cur;
  struct STMT* TemptCur;  
  bool ClearRun; 
  struct STMT* Current;

public:
  // 
  // Constructor
  //
  Debugger(struct STMT* program);

  //
  // Destructor
  //
  ~Debugger();

  // 
  // showState()
  // Show the current state of the debugger
  //
  void showState();

  //
  // step()
  //
  void step();

  //
  // runProgram()
  // Function for running the program
  // 
  void runProgram();

  // 
  // p_Varname
  // pinrts information about a variable
  //
  void p_Varname();

  //
  // traverseProgram()
  // check to see if the line number is in the program
  // 
  bool traverseProgram(STMT* head, int);

  //
  // repairGraph()
  //
  void repairGraph();


  //
  // setBreakpoint()
  //
  void setBreakpoint();

  //
  // removeBreakpoint()
  //
  void removeBreakpoint();

  //
  // clearBreakpoints()
  //
  void clearBreakpoints();

  //
  // listBreakpoints()
  //
  void listBreakpoints(); 

  //
  // where()
  //
  void where();

  //
  // run()
  //
  void run();
};

