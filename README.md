# CAZPYR
A heap-less minimalist terminal-based text editor within 1000 lines of C code.
## Features
Terminal window size adapting

Custom pre-compiled ANSI colors

Build and run shortcuts

Quality motions like home, end, page control, scroll control, and word jump

No external libraries

## Todo

Shift select

Cut, Copy, and Paste

Undo and redo

Syntax Highlighting

Better config

In app build

## User guide

### Compile and run
gcc ./main.c -o ./cazpyr

./cazpyr \<filename\>

### Keys
Insert and Delete control: characters, space, tab, enter, backspace, delete

Cursor control: arrow keys, home, end, page up, page down, ctrl+(home/end), ctrl+(arrow keys)

Save file: ctrl+s

Quit: ctrl+q

Initiate build: ctrl+b (make a ./BUILD file to be run or change INTIATE_BUILD_COMMAND)

Initiate run: ctrl+r (make a ./RUN file to be run or change INTIATE_RUN_COMMAND)
