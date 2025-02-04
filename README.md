# nuPython Debugger

This is a source-level debugger for `nuPython`, developed as part of CS 211 at Northwestern University. The debugger allows setting breakpoints, stepping through execution, and inspecting program state.

## How to Use

### Running the Debugger
You can run the debugger in two ways:

1. Provide a `nuPython` script as a command-line argument:

   ```sh
   ./a.out test.py
   ```

2. Run the debugger without arguments and enter the program manually. Use `$` to denote the end of the input program.

### Debugger Commands
Once inside the debugger, you can use the following commands:

- `r` → Run the program / continue from a breakpoint
- `s` → Step to the next statement
- `b <line>` → Set a breakpoint at a specific line
- `rb <line>` → Remove a breakpoint
- `lb` → List all breakpoints
- `cb` → Clear all breakpoints
- `p <varname>` → Print a variable’s value
- `ss` → Show debugger state
- `w` → Show the current execution line
- `q` → Quit the debugger

## Code Acknowledgment
Some code in this project was provided as part of the course, including `main.c` and core program execution functions. However, I wrote `debugger.cpp` and `debugger.h`, which implement the full debugger logic.

If you're reviewing this repo, I recommend starting with `debugger.cpp` to see the core implementation.
