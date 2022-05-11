# Serial UART (see https://www.lammertbies.nl/comm/info/serial-uart)

.extern uart

.globl put

.equ THR, 0x0
.equ LSR, 0x5

.section .text
put:
	# FIXME: check LSR before writing
	la t0, uart
	sb a0, THR(t0)
	ret
