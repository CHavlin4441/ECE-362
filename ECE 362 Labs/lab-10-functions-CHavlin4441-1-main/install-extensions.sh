#! /bin/bash

# Run this file as "./install-extensions.sh"
# to install debug/C++ extensions inside the 
# dev container.

code --install-extension /opt/riscv/riscv-debug.vsix
code --install-extension zhwu95.riscv
code --install-extension ms-vscode.cpptools
