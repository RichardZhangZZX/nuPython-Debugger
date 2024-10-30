/*debugger.cpp*/

//
// Debugger for nuPython
//
// Richard Zhang
// Northwestern University
// CS 211
//

#include <iostream>

#include "debugger.h"

using namespace std;

//
// Constructor
//
Debugger::Debugger(struct STMT* program)
  : Program(program), State("Loaded"), Memory(ram_init()), Breakpoint(0), 
  Prev(nullptr), Cur(nullptr), TemptCur(nullptr), ClearRun(false), BreakpointSet(false)
{}


//
// Destructor
//
Debugger::~Debugger()
{
  ram_destroy(this->Memory);
}


// 
// showState()
// Show the current state of the debugger
//
void Debugger::showState()
{
  cout << this->State << endl;
}


//
// runProgram()
// Function for running the program
// 
void Debugger::runProgram()
{
  //struct RAM* memory = ram_init();
  struct ExecuteResult execution = {false, nullptr};

  // TODO: different input for execution if restart after breakpoint
  if (this->State == "Loaded")
  { // If state is loaded, program will be ran from the head
    this->State = "Running";
    execution = execute(this->Program, this->Memory); 
  }
  else if (this->State == "Completed")
  {
    cout << "program has completed" << endl;
    repairGraph();
  }
  // TODO: Figure out the differences between this two cases
  else if (this->State == "Running" && this->ClearRun && this->BreakpointSet)
  { // Special case, clear and set new, this will run to the end of the program
    execution = execute(this->TemptCur, this->Memory); 
    ClearRun = false;
  }
  else
  { // Breakpoint is set, just finish executing the rest of the program
    // State is Running
    execution = execute(this->Cur, this->Memory);
    this->State = "Completed";
  }


  // Check if breakpoint is reached by checking if the line number
  // of the last execution statment is the same as the line number of Prev. 
  if (this->Prev != nullptr)
  {
    if (execution.LastStmt != nullptr && execution.LastStmt->line == this->Prev->line)
    {
      // If breakpoint is reached, automatically repair the program graph
      if (this->Prev->stmt_type == STMT_ASSIGNMENT)
        this->Prev->types.assignment->next_stmt = this->Cur; 
      else if (this->Prev->stmt_type == STMT_FUNCTION_CALL) 
        this->Prev->types.function_call->next_stmt = this->Cur;  
      else if (this->Prev->stmt_type == STMT_WHILE_LOOP) 
        this->Prev->types.while_loop->next_stmt = this->Cur;  
      else if (this->Prev->stmt_type == STMT_PASS)
        this->Prev->types.pass->next_stmt = this->Cur;  
      return; 
    }
  }

  this->State = "Completed";
}



// 
// p_Varname
// pinrts information about a variable
//
void Debugger::p_Varname()
{
  string varname;
  cin >> varname;

  const char* name = varname.c_str();
  struct RAM_VALUE* value = ram_read_cell_by_name(this->Memory, (char*) name);

  if (!value)
  {
    cout << "no such variable" << endl; 
    return; 
  }

  // int type = value->value_type;
  if (value->value_type == RAM_TYPE_INT)
    cout << varname << " (int): " << value->types.i << endl;
  else if (value->value_type == RAM_TYPE_REAL)
    cout << varname << " (real): " << value->types.d << endl;
  else if (value->value_type == RAM_TYPE_STR)
    cout << varname << " (str): " << value->types.s << endl;
  else if (value->value_type == RAM_TYPE_PTR)
    cout << varname << " (ptr): " << value->types.i << endl;
  else if (value->value_type == RAM_TYPE_BOOLEAN)
    cout << varname << " (bool): " << value->types.i << endl;
  else if (value->value_type == RAM_TYPE_NONE)
    cout << varname << " (none): None" << endl;

  ram_free_value(value);
}

//
// traverseProgram()
// check to see if the line number is in the program
// Traverse the program graph and set the two pointers: Prev and Cur
//
bool Debugger::traverseProgram(STMT* head, int lineNumber)
{
  STMT* prev = nullptr; 
  STMT* cur = head; //this->Program is the head of the program graph
  // This cur is local variable, as opposed to the class attribute Cur

  // Traverse the list until the end
  while (cur != nullptr)
  {
    if (cur->line == lineNumber)
    {
      // Only set Private attribute Prev and Cur when line is found
      this->Prev = prev; 
      this->Cur = cur;
      return true;
    } 

    // Move prev to cur and cur to the next statement
    prev = cur; 

    // Determine the next statment
    if (cur->stmt_type == STMT_ASSIGNMENT)
      cur = cur->types.assignment->next_stmt;
    else if (cur->stmt_type == STMT_FUNCTION_CALL)
      cur = cur->types.function_call->next_stmt;
    // else if (cur->stmt_type == STMT_IF_THEN_ELSE)
    // Don't need to worry about this case now for project 3
    else if (cur->stmt_type == STMT_WHILE_LOOP)
      cur = cur->types.while_loop->next_stmt;
    else if (cur->stmt_type == STMT_PASS)
      cur = cur->types.pass->next_stmt;
    else
    {
      cout << "Error: Unkown statment type!" << endl;
      return false; 
    }
  }
  return false;
}


//
// repairGraph()
//
void Debugger::repairGraph()
{
  if (this->Prev != nullptr)
  {
    if (this->Prev->stmt_type == STMT_ASSIGNMENT)
      this->Prev->types.assignment->next_stmt = this->Cur; 
    else if (this->Prev->stmt_type == STMT_FUNCTION_CALL) 
      this->Prev->types.function_call->next_stmt = this->Cur;  
    else if (this->Prev->stmt_type == STMT_WHILE_LOOP) 
      this->Prev->types.while_loop->next_stmt = this->Cur;  
    else if (this->Prev->stmt_type == STMT_PASS)
      this->Prev->types.pass->next_stmt = this->Cur;   
    return;
  }
}


//
// setBreakpoint()
// Setting the breakpoint, and breaking the link between Prev and Cur
//
void Debugger::setBreakpoint()
{
  // Warning: this gets into an infinite loop when the input is not an integer
  int line = -1;
  cin >> line; 
  if (line == this->Breakpoint && this->BreakpointSet == true)
    cout << "breakpoint already set" << endl;
  else
  {
    // TODO: If no such line, else set breakpoint...
    if (!traverseProgram(this->Program, line))
      cout << "no such line" << endl;
    else
    {
      if (this->Prev != nullptr) 
      {
        if (this->Prev->stmt_type == STMT_ASSIGNMENT)
          this->Prev->types.assignment->next_stmt = nullptr;  
        else if (this->Prev->stmt_type == STMT_FUNCTION_CALL) 
          this->Prev->types.function_call->next_stmt = nullptr;  
        else if (this->Prev->stmt_type == STMT_WHILE_LOOP) 
          this->Prev->types.while_loop->next_stmt = nullptr;  
        else if (this->Prev->stmt_type == STMT_PASS)
          this->Prev->types.pass->next_stmt = nullptr;  
        else
        {            
          cout << "Unknown statement type. Cannot set breakpoint." << endl;
          return;
        }

        this->Breakpoint = line;  // Save the line number where the breakpoint is set
        BreakpointSet = true; 
      } 
      else 
      { // TODO: Implement edge case when breakpoint is 1
        cout << "Cannot set a breakpoint at the start of the program" << endl;
      }
    }
  }
}

//
// removeBreakpoint()
//
void Debugger::removeBreakpoint()
{
  int line = -1; 
  cin >> line; 

  if (line == this->Breakpoint)
  {
    this->BreakpointSet = false;
    repairGraph();
    cout << "breakpoint removed" << endl;
  }
  else
    cout << "no such breakpoint" << endl; 
}

//
// clearBreakpoints()
//
void Debugger::clearBreakpoints()
{
  this->BreakpointSet = false;
  repairGraph();
  cout << "breakpoints cleared" << endl;

  // Special Case: Clear Breakpoint in the middle of the execution
  if (this->State == "Running") 
  {
    if (this->ClearRun == true) 
    // ClearRun == true means break mid-execution, but already clear breakpoint once, 
    // but haven't ran again. So the program will still run from the current line at break. 
      {}
    else
    {
      this->ClearRun = true;
      this->TemptCur = this->Cur;
    }
  }
}


//
// listBreakpoints()
//
void Debugger::listBreakpoints()
{
  if (this->BreakpointSet == false)
    cout << "no breakpoints" << endl;
  else
    cout << "breakpoints on lines: " << this->Breakpoint << endl;
}

//
// where()
//
void Debugger::where()
{
  if (this->State == "Loaded")
    cout << "line 1" << endl;
  else if (this->State == "Completed")
    cout << "completed execution" << endl;
  else
  { // State == "running"
    cout << "line " << this->Breakpoint << endl; 
  }
}


//
// run()
//
void Debugger::run()
{
  string cmd;

  while (true)
  {
    cout << "Enter a command, type h for help. Type r to run. > "; 

    cin >> cmd; 

    if (cmd == "q")
      break;

    else if (cmd == "h")
    {
      cout << "Available commands:" << endl;
      cout << "r -> Run the program / continue from a breakpoint" << endl;
      cout << "b n -> Breakpoint at line n" << endl;
      cout << "rb n -> Remove breakpoint at line n" << endl;
      cout << "lb -> List all breakpoints" << endl;
      cout << "cb -> Clear all breakpoints" << endl;
      cout << "p varname -> Print variable" << endl;
      cout << "sm -> Show memory contents" << endl;
      cout << "ss -> Show state of debugger" << endl;
      cout << "w -> What line are we on?" << endl;
      cout << "q -> Quit the debugger" << endl;
    }

    else if (cmd == "ss")
      showState();

    else if (cmd == "sm")
      ram_print(this->Memory); 

    else if (cmd == "p")
      p_Varname();

    else if (cmd == "b")
      setBreakpoint(); 

    else if (cmd == "rb")
      removeBreakpoint();

    else if (cmd == "cb")
      clearBreakpoints();

    else if (cmd == "r")
      runProgram();

    else if (cmd == "lb")
      listBreakpoints();

    else if (cmd == "w")
      where();

    else
      cout << "unkown command" << endl; 
  }
  if (this->BreakpointSet)
    repairGraph();
}
