PROVIDE(uart = 0x10000000);

SECTIONS
{
	. = 0x80000000;
	.text : { stack.o(.text) *(.text) }
	.data : { *(.data) 
		_testing = .;
		app.o(.testing)
		_hello_jr = .;
		app.o(.hello_jr)
	}

	.bss : { *(.bss) *(COMMON) }
	
	. = . + SIZEOF(.bss); 
	phys_base = ALIGN(0x1000);
}
