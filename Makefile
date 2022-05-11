SHELL = /bin/sh

srcdir = ./src/kernel
testdir = ./src/kernel

# TODO: add -M to CFLAGS?
# AS = riscv64-unknown-linux-as
CC = riscv64-unknown-linux-gnu-gcc
LD = riscv64-unknown-linux-gnu-ld
LDFLAGS = -pie # -M
CFLAGS = -c -g3 -O3 # -fno-tree-switch-conversion

SRCDIR = $(srcdir)
TESTDIR = $(testdir)

VPATH = $(SRCDIR)

SRCS_C = $(wildcard $(SRCDIR)/*.c)
SRCS_S = $(wildcard $(SRCDIR)/*.S)
SRCS := $(SRCS_C) $(SRCS_S)

ifneq ($(TESTDIR),$(SRCDIR))
CFLAGS += -DTEST
TESTS = $(wildcard $(TESTDIR)/*.c)
SRCS_WITH_TEST = $(subst $(TESTDIR),$(SRCDIR), $(TESTS))
SRCS := $(filter-out $(SRCS_WITH_TEST),$(SRCS)) $(TESTS)
endif

OBJS := $(SRCS)

OBJS := $(OBJS:.c=.o)
OBJS := $(OBJS:.S=.o)

# $(info OBJS is $(OBJS)) <-- useful for debugging

# -ffreestanding: Tell the compiler there may not be any standard libraries.
#  Otherwise it will complain about for example printf not matching the printf
#  in glibc.
CFLAGS += -ffreestanding

# -march=rv64ima: Specifies the ISA and extensions (I, M, and A). The default is
#  rv64gc which uses compressed instructions, something we don't want. And rv64g
#  uses problematic double-precision floating point instructions (fsd), that's
#  why we use ima instead (implies soft float)
# -mabi=lp64: See https://gcc.gnu.org/onlinedocs/gcc/RISC-V-Options.html
CFLAGS += -march=rv64ima -mabi=lp64

# -mcmodel=medany: Need it because we have a custom .ld script
CFLAGS += -mcmodel=medany

# -falign-functions=4: Interrupt handlers need to be 4-byte aligned
CFLAGS += -falign-functions=4

# -Wall: all warnings! Makes you a good programmer!
# CFLAGS += -Wall

# -S: Do not start CPU at startup (you must type 'c' in the monitor)
# -s: Shorthand for -gdb tcp::1234, i.e. open a gdbserver on TCP port 1234.
# -nographic: Implies -serial stdio which is what we want (and no GUI!)
# -monitor none: Allows us to Ctrl-C qemu itself (instead of forw. to guest)
QEMUFLAGS := -bios none -s -m 32M -machine virt -nographic -monitor none

.PHONY : grade
grade : test qemu

# TODO: get rid of src/kernel here
src/kernel/stack.o src/kernel/ns16550a.o : %.o: %.S
	$(CC) $(CFLAGS) $^ -o $@

jake : clean main.ld $(OBJS)
	$(LD) $(LDFLAGS) -o $@ -T main.ld -L$(SRCDIR) -L$(TESTDIR) $(OBJS)

# All targets below require either test or jake, for example
# 	make jake gdb
# 	make test gdb

.PHONY : test
test :
	$(MAKE) testdir=./src/test jake

.PHONY : dis
dis :
	riscv64-unknown-linux-gnu-objdump -D jake

.PHONY : dis
gdb :
	qemu-system-riscv64 $(QEMUFLAGS) -S -kernel jake >/dev/null &
	riscv64-unknown-linux-gnu-gdb -ex "target remote localhost:1234"
	pkill qemu-system-ris

.PHONY : qemu
qemu :
	qemu-system-riscv64 $(QEMUFLAGS) -kernel jake

.PHONY : cscope
cscope :
	cscope -bR

.PHONY : tar
tar :
	make clean && tar -cvzf jake.tar.gz ./src/kernel/

.PHONY : clean
clean :
	rm -f $(SRCDIR)/*.o ./src/test/*.o jake

# Taken from
# http://cdn.kernel.org/pub/linux/kernel/people/will/docs/qemu/qemu-arm64-howto.htmlen
.PHONY : dtb
dtb:
	qemu-system-riscv64 $(QEMUFLAGS) -machine dumpdtb=virt.dtb
	dtc -o virt.dts -O dts -I dtb virt.dtb