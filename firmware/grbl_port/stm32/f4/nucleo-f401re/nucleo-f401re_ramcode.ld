/*
 * This file is part of the grbl_port_opencm3 project.
 *
 * Copyright (c) 2017-2020 The Ant Team
 *
 * Grbl_port_opencm3 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Grbl_port_opencm3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with grbl_port_opencm3. If not, see <http://www.gnu.org/licenses/>.
 */
 
/* Linker script for Nucleo F401RE (STM32F401RE, 512K flash, 96K RAM). 
 * with a section reserved for code to be fetched by RAM.
 */

/* Define memory regions. */
MEMORY
{
	rom (rx) : ORIGIN = 0x08000000, LENGTH = 32K
	rom2 (rx) : ORIGIN = 0x08010000, LENGTH = 448K
	ram (rwx) : ORIGIN = 0x20000000, LENGTH = 10K
	ram_txt (rwx) : ORIGIN = 0x20002800, LENGTH = 86K
}

/* Enforce emmition of the vector table. */
EXTERN (vector_table)

/* Define the entry point of the output file. */
ENTRY(reset_handler)

/* Define sections. */
SECTIONS
{
	.ramcode : {
		    _ramcode = .;
		. = ALIGN(4);
	        *(.vectors)	/* Vector table */
			*flash.o
			*serial.o
		. = ALIGN(4);
			_eramcode = .;
	}  >ram_txt AT >rom
	_ramcode_loadaddr = LOADADDR(.ramcode);
	
	.ramcode2 : {
		. = ALIGN(4);
		    _ramcode2 = .;
			*protocol.o
			*report.o
			*settings.o
			*system.o
		. = ALIGN(4);
	}  >ram_txt AT >rom2
	_ramcode2_loadaddr = LOADADDR(.ramcode2);

	. = ALIGN(4);
	_eramcode2 = .;
	
	.text : {
		*(.text*)	/* Program code */
		. = ALIGN(4);
		*(.rodata*)	/* Read-only data */
		. = ALIGN(4);
	} >rom2
	
	/*
	 * Another section used by C++ stuff, appears when using newlib with
	 * 64bit (long long) printf support
	 */
	.ARM.extab : {
		*(.ARM.extab*)
	} >rom
	.ARM.exidx : {
		__exidx_start = .;
		*(.ARM.exidx*)
		__exidx_end = .;
	} >rom

	. = ALIGN(4);
	_etext = .;

	.data : {
		_data = .;
		*(.data*)	/* Read-write initialized data */
		. = ALIGN(4);
		_edata = .;
	} >ram AT >rom2
	_data_loadaddr = LOADADDR(.data);

	.bss : {
		_bss = .;
		*(.bss*)	/* Read-write zero initialized data */
		*(COMMON)
		. = ALIGN(4);
		_ebss = .;
	} >ram

	/*
	 * The .eh_frame section appears to be used for C++ exception handling.
	 * You may need to fix this if you're using C++.
	 */
	/DISCARD/ : { *(.eh_frame) }

	. = ALIGN(4);
	end = .;
}

PROVIDE(_stack = ORIGIN(ram) + LENGTH(ram));



