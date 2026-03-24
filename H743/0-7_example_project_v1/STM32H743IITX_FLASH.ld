/*
******************************************************************************
**
**  File        : LinkerScript.ld
**
**  Author      : STM32CubeIDE
**
**  Abstract    : Linker script for STM32H7 series
**                2048Kbytes FLASH and 1056Kbytes RAM
**
**                Set heap size, stack size and stack location according
**                to application requirements.
**
**                Set memory bank area and size if external memory is used.
**
**  Target      : STMicroelectronics STM32
**
**  Distribution: The file is distributed as is, without any warranty
**                of any kind.
**
*****************************************************************************
** @attention
**
** Copyright (c) 2025 STMicroelectronics.
** All rights reserved.
**
** This software is licensed under terms that can be found in the LICENSE file
** in the root directory of this software component.
** If no LICENSE file comes with this software, it is provided AS-IS.
**
****************************************************************************
*/

/* Entry Point */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_estack = ORIGIN(RAM_D1) + LENGTH(RAM_D1);    /* end of RAM */
/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size = 0x2048;      /* required amount of heap  */
_Min_Stack_Size = 0x2048; /* required amount of stack */

/* Specify the memory areas */
MEMORY
{
  FLASH (rx)     : ORIGIN = 0x08000000, LENGTH = 2048K
  DTCMRAM (xrw)  : ORIGIN = 0x20000000, LENGTH = 128K
/* ↓ 系统SRAM ↓ = 864K */
  RAM_D1 (xrw)   : ORIGIN = 0x24000000, LENGTH = 512K
  RAM_D2 (xrw)   : ORIGIN = 0x30000000, LENGTH = 288K	/* 128K + 128K + 32K */
  RAM_D3 (xrw)   : ORIGIN = 0x38000000, LENGTH = 64K
/* ↑ 系统SRAM ↑ */ 
  ITCMRAM (xrw)  : ORIGIN = 0x00000000, LENGTH = 64K
}

/* Define output sections */
SECTIONS
{
  /* The startup code goes first into FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >FLASH

  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH

  /* Constant data goes into FLASH */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >FLASH

  .ARM.extab (READONLY) : /* The READONLY keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } >FLASH
  .ARM (READONLY) : /* The READONLY keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

  .preinit_array (READONLY) : /* The READONLY keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH

  .init_array (READONLY) : /* The READONLY keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH

  .fini_array (READONLY) : /* The READONLY keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH

  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */
    *(.RamFunc)        /* .RamFunc sections */
    *(.RamFunc*)       /* .RamFunc* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >RAM_D1 AT> FLASH

  /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss section */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >RAM_D1

  /* User_heap_stack section, used to check that there is enough RAM left */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >RAM_D1

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}

/* =============================== ↓ 用户定义 ↓ =============================== */

/* 定义外部内存区域 */
MEMORY {
  SDRAM (xrw) : ORIGIN = 0xC0000000, LENGTH = 32M  /* 定义SDRAM区域 */
}


SECTIONS {
	/* LTDC缓存 */
	.sdram_section (NOLOAD) : {
		. = ALIGN(16);
		*(.sdram_section)  /* 将变量放入此段 */
		. = ALIGN(16);	
	} > SDRAM
	
	/* ---------------------- ↓ 外部SDRAM ↓ ----------------------- */
	/* NOLOAD 属性告诉加载器不要在烧录时初始化这片内存 */
	.sdram_pool 0XC01F4000 (NOLOAD) :
	{
		. = ALIGN(64);
		KEEP(*(.sdram_pool))
		. = ALIGN(64);
	} >SDRAM
	
		.sdram_map (ADDR(.sdram_pool) + SIZEOF(.sdram_pool)) (NOLOAD) :
	{
		. = ALIGN(4);
		KEEP(*(.sdram_map))
	} >SDRAM

	/* ------------------------ ↓ ITCM ↓ ------------------------ */
	/* ITCM内存池 */
	.itcm_pool 0x00000000 :
	{
	    . = ALIGN(64);
	    *(.itcm_pool)
	    . = ALIGN(64);
	} >ITCMRAM
	/* ITCM内存管理表 */
	.itcm_map (ADDR(.itcm_pool) + SIZEOF(.itcm_pool)) :
	{
		. = ALIGN(4); /* MT_TYPE 通常是 4 字节对齐 */
		*(.itcm_map)
	} >ITCMRAM
	/* --------------------------- ↓ DTCM ↓ --------------------------- */
	/* DTCM内存池 */
	.dtcm_pool 0x20000000 :
	{
	    . = ALIGN(64);
	    *(.dtcm_pool)
	    . = ALIGN(64);
	} >DTCMRAM
	/* DTCM内存管理表 */
	.dtcm_map (ADDR(.dtcm_pool) + SIZEOF(.dtcm_pool)) :
	{
		. = ALIGN(4);
		*(.dtcm_map)
	} >DTCMRAM
	
	/* --------------------------- ↓ D1域 ↓ --------------------------- */
	/* 没有指定的默认放于D1域 */
	
	/* ---------------- ↓ D2域(AHB SRAM1,AHB SRAM2) ↓ ----------------- */
	/* D2域内存池 */
	.sram12_pool 0x30000000 :
	{
	    . = ALIGN(64);
	    *(.sram12_pool)
	    . = ALIGN(64);
	} >RAM_D2
	/* D2域内存管理表 */
	.sram12_map (ADDR(.sram12_pool) + SIZEOF(.sram12_pool)) :
	{
	    . = ALIGN(4);
	    *(.sram12_map)
	} >RAM_D2
	
	/* --------------------- ↓ D3域(AHB SRAM4) ↓ --------------------- */
	/* D3域内存池 */
	.sram4_pool 0x38000000 :
	{
	    . = ALIGN(64);
	    *(.sram4_pool)
	    . = ALIGN(64);
	} >RAM_D3
	/* D3域内存管理表 */
	.sram4_map (ADDR(.sram4_pool) + SIZEOF(.sram4_pool)) :
	{
	    . = ALIGN(4);
	    *(.sram4_map)
	} >RAM_D3

}
/* =============================== ↑ 用户定义 ↑ =============================== */
