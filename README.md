# Building Your Vim-like Text Editor

**Hongli SHEN** && **Ximing ZHENG**

## Description

This project initializes the core functionalities of Vim on a Linux system using C++. It leverages <ncurses.h> to enhance its usability and interface.

- **Basic parts**
  - Normal mode: file and mode commands, cursor movement, line navigation, basic editing.
  - Insert mode: direct text insertion, new line creation, backspace support, real-time display update, cursor movement with typed text, exit to normal mode.
- **Bonus parts**
  - undo and redo
  - line numbers and jumping
  - Search and replace
  - UI improvements: 
    - better color
    - mode display

#### Contributions

- **Hongli SHEN**: Basic parts without overflow content, undo and redo, line numbers and jumping, background color.
- **Ximing ZHENG**: overflow content, 

## Project structure

```
src/              #source code
include/          #head file
build/            #compile
text.txt          #text to edit
README.md.        #project doucument
```

The project initialize the vim functions through a C++ class `MiniVim`. It include a head file `<ncurses.h>` for terminal-based capabilities.  [http://tldp.org/HOWTO/NCURSES-Programming-HOWTO]()



## Usage

#### Build

```

```

Should work for Windows, macOS and Linux.

#### Arguments

We start with normal mode when enter mini-vim.

**Normal mode**

- `h,j,k,l`: move the cursor(`left,down,up,right`)
- Arrow keys to move the cursor
- `0`: jump the cursor to the beginning of the current line
- `$`: jump the cursor to the end of the current line
- `gg`: move the cursor to the first line at the beginning
- `G`: move the cursor to the last line at the beginning
- `dd`: delete the current line
- `yy`: copy the line
- `p`: paste the copied line below the cursor
- `u` for undo and `Ctrl+R` for redo
- `i`: move to the insert mode

**Insert mode**

- type characters for insertion
- `enter`: create a new line
- `backspace`: delete characters
- move cursor operations are the same as in normal mode
- `esc`: exit to normal mode

we press `:` to enter command line mode when we are in the normal mode.

**command line mode**

- `:q`: exit the program
- `:w`: save the file
- `:wq`: save the file and enter the program
- `:+number`: jump the cursor to the specific line at the beginning