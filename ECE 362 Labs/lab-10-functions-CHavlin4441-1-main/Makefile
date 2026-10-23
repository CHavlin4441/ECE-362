PREFIX := riscv64-unknown-elf-
CC := $(PREFIX)g++
OBJDUMP := $(PREFIX)objdump
CFLAGS := -march=rv32im -mabi=ilp32 -g3
QEMU := qemu-riscv32-static
QEMU_FLAGS := 
INC := ./include
EXE := lab

.PHONY: clean
.PHONY: help
.PHONY: port
.PHONY: kill

# Run the lab, kill to ensure only one instance of qemu running
run: lab kill
	@rm -f .autograde_debug_mode
	@$(QEMU) $(QEMU_FLAGS) $(EXE)

# Debug the lab: creates .autograde_debug_mode so the binary disables its 1s timeout
debug: lab port kill
	@echo "target remote localhost:`sed -n '1 p' port.env`" > ./qemu_riscv32.gdbinit
	@echo "Debug session started"
	@echo "Waiting for gdb connection on port `sed -n '1 p' port.env`"
	@touch .autograde_debug_mode
	@$(QEMU) $(QEMU_FLAGS) -g `sed -n '1 p' port.env` $(EXE); rm -f .autograde_debug_mode

# Kill qemu, ignore if none is presented
kill:
	@killall -q -u ${USER} $(QEMU) || :

# Obtain an unused port
port:
	@shuf -i 1024-65535 -n 1 > port.env

%.o: %.cpp
	$(CC) -o $@ -c $^ -I$(INC) $(CFLAGS)

lab: lab.S autotest.o
	$(CC) -o $(EXE) $^ -I$(INC) $(CFLAGS)

lab.dump: $(EXE)
	@$(OBJDUMP) -D $^ > $@

######################################################
# Dynamic targets
quicksort strconcat fib bsearch: lab
	@rm -f .autograde_debug_mode
	$(QEMU) $(QEMU_FLAGS) $(EXE) --$@ || true
######################################################

clean:
	@rm -f $(EXE) port.env qemu_riscv32.gdbinit .autograde_debug_mode

help:
	@echo "-----------------------------------------------------"
	@echo "| Help section for ECE 362 RISCV QEMU               |"
	@echo "-----------------------------------------------------"
	@echo "| make: compile and run the lab on qemu             |"
	@echo "|                                                   |"
	@echo "| make run: same as 'make'                          |"
	@echo "|                                                   |"
	@echo "| make debug: compile and launch executable waiting |"
	@echo "|             for VSCode connection                 |"
	@echo "|                                                   |"
	@echo "| make kill: kill all qemu instance belong to       |"
	@echo "|            the user. Automatically executed with  |"
	@echo "|            'make run' and 'make debug'            |"
	@echo "|                                                   |"
	@echo "| make port: get an unused port for gdb connection  |"
	@echo "|            used internally for 'debug'            |"
	@echo "|                                                   |"
	@echo "| make lab: compile lab with autotest               |"
	@echo "|                                                   |"
	@echo "| make clean: clean up object file and executable   |"
	@echo "|                                                   |"
	@echo "| make help: print this message                     |"
	@echo "|                                                   |"
	@echo "-----------------------------------------------------"
