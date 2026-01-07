/*  
    Define the physical memory regions available on the MCU.
    These numbers come from the STM32F103RB datasheet / RM0008 memory map:
    - Flash starts at 0x08000000 and is 128 KB for STM32F103xB
    - SRAM starts at 0x20000000 and is 20 KB for STM32F103RB
*/
MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 128K
    RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}
/*
    Linker symbol __reset_stack_pointer for top of RAM
    On Cortex-M, the CPU loads SP from vector table entry 0 on reset.
    Convention is to start SP at the top of RAM so the stack grows downwards.
*/
__reset_stack_pointer = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
    /* Put the program code into Flash */
    .text : {
        /* Set the initial address to 0 */
        /* '.' is the location counter */
        . = 0; 
        /* First entry in vector table is the initial SP value, the last RAM address */
        LONG(__reset_stack_pointer);
        /* Next entry in the vector table is initial PC value */
        /* OR with 1 to set LSB to 1, for Thumb mode */
        LONG(main | 1);
        /* 
            The whole interrupt table is 332 bytes long. 
            Cortex-M3 Core: 16 entries * 4 bytes = 64 bytes
            STM32F103RB External Interrupts: 60 entries * 4 bytes = 240 bytes
            Total: 64 + 240 = 304 bytes
            Advance to that position. 
        */
        __vector_table_size = (16 + 60) * 4;
        . = __vector_table_size;
        /* The rest of the code */
        *(.text*)
    } > FLASH /* Put the code in the Flash memory region */
}
