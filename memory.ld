/* 
Define physical memory ranges of MCU (from STM32F103xB datasheet) 

Origins:
- Memory map figure/table shows:
    - Flash starts at 0x08000000
    - SRAM starts at 0x20000000

Lengths:
- Flash: Section 2.3.2 shows 64 or 128 KB flash options (B in name specifies 128 KB)
- RAM: Section 2.3.4 shows 20 KB of Embedded SRAM

This is a minimal  and simplified memory linker script, missing much of
what standard scripts have. 
*/
MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 128K
    RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}
/* Linker symbol = top of RAM. On reset, CPU loads SP from vector table entry 0 */
__reset_stack_pointer = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
    /* Place code in FLASH. This section begins at the FLASH ORIGIN. */
    .text : {
        /* '.' is the location counter inside this output section */
        . = 0;

        /* Vector table entry 0: initial SP */
        LONG(__reset_stack_pointer);

        /* 
        Vector table entry 1: reset handler address using main as reset handler.
        '| 1' sets Thumb-state bit (the LSB) (Cortex-M uses Thumb instruction set)
        */
        LONG(main | 1);


        /* 
        Reserve space for 83 entries 
            - up to 16 core exceptions defined by (ARM) 
            - up to 67 external interrupt entries (defined by MCU)
        
        83 entries * 4 bytes per entry = 332 bytes 
        */
        . = 332;

        /* Place all compiled .text (instructions) here */
        *(.text*)
    } > FLASH
}
