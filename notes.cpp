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


breakProgram()
{
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
      this->Breakpoints.push_back(line); 
    } 
    else 
    { // TODO: Implement edge case when breakpoint is 1
      cout << "Cannot set a breakpoint at the start of the program" << endl;
    }
  }
}


// OLD RUN PROGRAM, NEED TO BE DISCARDED
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
  else if (this->State == "Running" && this->ClearRun && !this->Breakpoints.empty())
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




void Debugger::step()
{
  // Move Prev to Current
  Prev = Current;

  // Move Current to the next statement
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
    cout << "Prev is nullptr" << endl;

  execute(Prev, Memory);
  
  repairGraph();
}