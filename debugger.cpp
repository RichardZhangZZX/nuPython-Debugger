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
  : Program(program), State("Loaded"), Memory(ram_init()), 
  Prev(nullptr), Cur(nullptr), TemptCur(nullptr), ClearRun(false), Current(program),
  Already_at_break(false)
{}


//
// Destructor
//
Debugger::~Debugger()
{
  ram_destroy(this->Memory);
}


/*
breakProgram()
*/



// 
// showState()
// Show the current state of the debugger
//
void Debugger::showState()
{
  cout << this->State << endl;
}

//
// step()
// Assume this play with Prev and Current directly
// Step to next stmt by executing current stmt
// This is to break the program, where as repair is to repair the program
// 
void Debugger::step()
{
  // Move Prev to Current
  Prev = Current;

  // Move Current to the next statement
  if (Current != nullptr)
  { 
    if (Current->stmt_type == STMT_ASSIGNMENT)
      Current = Current->types.assignment->next_stmt;
    else if (Current->stmt_type == STMT_FUNCTION_CALL)
      Current = Current->types.function_call->next_stmt;
    else if (Current->stmt_type == STMT_WHILE_LOOP)
      Current = Current->types.while_loop->next_stmt;
    else if (Current->stmt_type == STMT_PASS)
      Current = Current->types.pass->next_stmt;
    else
      cout << "Unkown statement type for Current" << endl;

    // If the next statement became nullptr, the program is completed
    if (Current == nullptr)
      this->State = "Completed";
  }
  else
    cout << "Current is nullptr" << endl; 

  // Break the program graph by setting Prev->Next to Nullptr
  if (Prev != nullptr) 
  {
    if (Prev->stmt_type == STMT_ASSIGNMENT)
      Prev->types.assignment->next_stmt = nullptr;  
    else if (Prev->stmt_type == STMT_FUNCTION_CALL) 
      Prev->types.function_call->next_stmt = nullptr;  
    else if (Prev->stmt_type == STMT_WHILE_LOOP) 
      Prev->types.while_loop->next_stmt = nullptr;  
    else if (this->Prev->stmt_type == STMT_PASS)
      Prev->types.pass->next_stmt = nullptr;  
    else   
      cout << "Unknown statement type for Prev" << endl;
  }
  else 
    cout << "Prev is nullptr 2" << endl;

  if (Prev != nullptr)
  {
    struct ExecuteResult execution = {false, nullptr};
    execution = execute(Prev, Memory);
    if (execution.Success == false)
      this->State = "Completed";
  }
  else
    cout << "Prev is nullptr 1" << endl;
  
  repairGraph();
}


// OLD RUN PROGRAM, NEED TO BE DISCARDED


//
// runProgram()
// Function for running the program, specifically for "r" command 
//
void Debugger::runProgram()
{
  while (true)
  {
    // Just hit breakpoint
    if (Current != nullptr && find(Breakpoints.begin(), Breakpoints.end(), Current->line) != Breakpoints.end() && Already_at_break == false)
    {
      cout << "hit breakpoint on line " << Current->line << endl;
      Already_at_break = true; 
      printNext();
      break;
    }
    else
    {
      if (this->State == "Completed")
      {
        cout << "program has completed" << endl;
        break;
      }
      else if (this->State == "Loaded")
        this->State = "Running";
      step();
      Already_at_break = false; 
    }
  }
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
//
bool Debugger::traverseProgram(STMT* head, int lineNumber)
{
  STMT* cur = head; //this->Program is the head of the program graph

  // Traverse the list until the end
  while (cur != nullptr)
  {
    if (cur->line == lineNumber)
    {
      return true;
    } 

    // Determine the next statment
    if (cur->stmt_type == STMT_ASSIGNMENT)
      cur = cur->types.assignment->next_stmt;
    else if (cur->stmt_type == STMT_FUNCTION_CALL)
      cur = cur->types.function_call->next_stmt;
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
  if (Prev != nullptr)
  {
    if (Prev->stmt_type == STMT_ASSIGNMENT)
      Prev->types.assignment->next_stmt = Current; 
    else if (Prev->stmt_type == STMT_FUNCTION_CALL) 
      Prev->types.function_call->next_stmt = Current;  
    else if (this->Prev->stmt_type == STMT_WHILE_LOOP) 
      this->Prev->types.while_loop->next_stmt = Current;  
    else if (this->Prev->stmt_type == STMT_PASS)
      Prev->types.pass->next_stmt = Current;   
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

  // If breakpoint already exist, output the message, else set breakpoint 
  // but don't break the function. Also check if the line exist
  if (find(this->Breakpoints.begin(), this->Breakpoints.end(), line) != this->Breakpoints.end())
    cout << "breakpoint already set" << endl;
  else
  {
    if (!traverseProgram(this->Program, line))
      cout << "no such line" << endl; 
    else
    {
      this->Breakpoints.push_back(line); 
      sort(this->Breakpoints.begin(), this->Breakpoints.end()); 
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

  auto it = find(this->Breakpoints.begin(), this->Breakpoints.end(), line);

  if (it != this->Breakpoints.end())
  {
    this->Breakpoints.erase(it); 
    repairGraph();
    cout << "breakpoint removed" << endl;
  }
  else
  {
    cout << "no such breakpoint" << endl; 
  }
}

//
// clearBreakpoints()
//
void Debugger::clearBreakpoints()
{
  this->Breakpoints.clear();
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
  if (this->Breakpoints.empty())
    cout << "no breakpoints" << endl;
  else
  {
    cout << "breakpoints on lines: ";
    for (int BP: this->Breakpoints)
      cout << BP << " ";
  }
}

//
// printNext()
//
void Debugger::printNext()
{
  struct STMT* next; 
  // Set the temporary next pointer for repairing the graph later
  // Simultaneously temporarily break the graph, treat Current here like Prev
  if (Current->stmt_type == STMT_ASSIGNMENT)
  {
    next = Current->types.assignment->next_stmt;
    Current->types.assignment->next_stmt = nullptr;
  }
  else if (Current->stmt_type == STMT_FUNCTION_CALL)
  {
    next = Current->types.function_call->next_stmt;
    Current->types.function_call->next_stmt = nullptr;
  }
  else if (Current->stmt_type == STMT_WHILE_LOOP)
  {
    next = Current->types.while_loop->next_stmt;
    Current->types.while_loop->next_stmt = nullptr;
  }
  else if (Current->stmt_type == STMT_PASS)
  {
    next = Current->types.pass->next_stmt;
    Current->types.pass->next_stmt = nullptr;
  }
  else
    cout << "Unknown statement type for Current" << endl;

  programgraph_print(Current);

  // Repair the program graph
  if (Current->stmt_type == STMT_ASSIGNMENT)
    Current->types.assignment->next_stmt = next; 
  else if (Current->stmt_type == STMT_FUNCTION_CALL) 
    Current->types.function_call->next_stmt = next;  
  else if (Current->stmt_type == STMT_WHILE_LOOP) 
    Current->types.while_loop->next_stmt = next;  
  else if (Current->stmt_type == STMT_PASS)
    Current->types.pass->next_stmt = Current;   
  return;
}



//
// where()
//
void Debugger::where()
{ // Assume for now that "Loaded" and "Running is the same case"
  if (this->State == "Completed")
    cout << "completed execution" << endl; 
  else
  {
    cout << "line " << Current->line << endl;
    printNext();
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
    cout << endl;
    cout << "Enter a command, type h for help. Type r to run. > " << endl; 
    cin >> cmd; 

    if (cmd == "q")
      break;

    else if (cmd == "h")
    {
      cout << "Available commands:" << endl;
      cout << "r -> Run the program / continue from a breakpoint" << endl;
      cout << "s -> Step to next stmt by executing current stmt" << endl; 
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

    else if (cmd == "s")
    {
      if (this->State == "Completed")
      {
        cout << "program has completed" << endl;
        continue; 
      }
      else if (this->State == "Loaded")
        this->State = "Running";
      step();
    }

    else if (cmd == "lb")
      listBreakpoints();

    else if (cmd == "w")
      where();

    else
      cout << "unknown command" << endl; 
  }
  if (!this->Breakpoints.empty())
    repairGraph();
}
