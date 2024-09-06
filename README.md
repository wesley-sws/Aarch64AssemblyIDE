# Aarch64 Assembly IDE

AArch64_Assembly_IDE is an integrated development environment (IDE) specifically designed for AArch64 assembly language programming.
Note the Emulator and Assembler codebases are private due to Imperial College's projects policies but can be shown on request. Code making use of them in the IDE have been commented.
Any warnings about GTimeVal are due to its usage in GtkSourceView3.

## Features

- **Command Shortcuts**:
  - Note: Use `Cmd` for macOS and `Ctrl` for other operating systems.
  - `Cmd/Ctrl + O`: Open file
  - `Cmd/Ctrl + S`: Save file
  - `Cmd/Ctrl + W`: Close assembly code tab
  - `Cmd/Ctrl + T`: Add assembly code tab
  - `Cmd/Ctrl + 1-9`: Navigate through different tabs (9 for rightmost tab)
  - `Esc`: Close autocomplete window or file dialog
- **Themes**: Options include light theme, dark theme, and catppuccin (default).
- **Autocomplete**: Code completion based on the mnemonics of the AArch64 Assembly Language. Use the arrow keys to navigate through suggestions and press Enter to select an option.
- **Syntax Highlighting**: Different colors for different syntax elements.
- **Error Detection**: Underlines and shows errors in your code as you type.
- **Build**: Can build assembly file to binary file as well as emulating its execution on a Raspberry Pi (text file will be produced).

## Prerequisites

Ensure you have make, gtk3, and gtksourceview3 installed on your system.

On Ubuntu, you can install these with:

```sudo apt-get install build-essential libgtk-3-dev libgtksourceview-3.0-dev```

On macOS, you can install these using Homebrew:

```brew install gtk+3 gtksourceview3```

## Clone the Repository

```git clone https://github.com/wesley-sws/AArch64_Assembly_IDE.git```

```cd AArch64_Assembly_IDE```

## Build and Run

```make```

```./assembly_ide```



