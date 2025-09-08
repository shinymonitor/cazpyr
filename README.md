# CAZPYR
A heap-free minimalist terminal-based text editor written in under 1000 lines of pure C.

## Why Use CAZPYR
- **Zero heap allocations**: Predictable memory usage with fixed-size buffers
- **Lightning fast**: No dynamic memory management overhead
- **Developer-focused**: Built-in build and run shortcuts
- **Zero dependencies**: Pure C with only standard libraries
- **Hackable**: Easy to modify codebase under 1000 lines

## Features

- Full text editing
- Copy, cut, and paste with visual selection
- Smart cursor movement with word jumping
- Advanced navigation (home, end, page up/down, file start/end, page scroll)
- Auto-adapting display (responds to terminal window resizing)
- Build shortcut
- Run shortcut
- tmux compatible
- Line numbers with smart padding
- Fixed memory footprint (no malloc/free, no memory leaks)
- Custom ANSI color scheme (easy to modify in source)
- Efficient rendering (minimal screen updates)

## Keybindings

| Action | Key |
|--------|-----|
| **File Operations** |
| Save | ctrl+s |
| Quit | ctrl+q |
| **Build & Run** |
| Build | ctrl+t |
| Run | ctrl+r |
| **Editing** |
| Copy | ctrl+c |
| Cut | ctrl+x |
| Paste | ctrl+v |
| **Navigation** |
| Move by word | ctrl + left/right arrow keys |
| Start/End of line | home/end |
| Start/End of file | ctrl+home/end |
| Page scroll | ctrl + up/down arrow keys |
| **Selection** |
| Select text | shift + navigation keys |

## Todo

Undo and redo

Find and Replace

Syntax Highlighting

Better config

In app build