/*
 * Script for GNU linker.
 * Describes layout of sections, location of stack.
 *
 * In this case vectors are at location 0 (reset @ 0x08)
 *
 * +------------+  0x00400000
 * data         |
 * end
 * |(heap)      |
 * .            .
 * .            .
 * |(heap limit)|
 *
 * |- - - - - - |
 * stack bottom    256k
 * +------------+
 *
 * +------------+  0x00000000
 * |vectors     |
 * |            |
 * |------------+
 * |text        |
 * |data        |
 * |            |  1024k
 * +------------+
 */


/* Split memory into area for vectors and ram */
MEMORY
{
        flash  (rx) : ORIGIN = 0x00010000, LENGTH = 1215k /* 1216KB - 96B */
        ram   (rw!x): ORIGIN = 0x00400020, LENGTH = 262112
}

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_vector_start);
_vector_start = ORIGIN(flash);

SECTIONS
{
    /* vectors go to vectors region */
    . = ORIGIN(flash);
    .vectors :
    {
        KEEP(*(*.vectors))
        KEEP( *(*.rom1))
    } > flash

    /* instructions go to the text region*/

    . = ALIGN(0x8);
    /* code, instructions.for example: i=i+1; */
    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text.*)
        *(.stub)
        /* .gnu.warning sections are handled specially by elf32.em.  */
        *(.gnu.warning)
        *(.gnu.linkonce.t*)
        *(.glue_7t) *(.glue_7)

        KEEP(*(.fini))

        /* section information for finsh shell */
        . = ALIGN(4);
        __fsymtab_start = .;
        KEEP(*(FSymTab))
        __fsymtab_end = .;
        . = ALIGN(4);
        __vsymtab_start = .;
        KEEP(*(VSymTab))
        __vsymtab_end = .;
        . = ALIGN(4);

        /* section information for modules */
        . = ALIGN(4);
        __rtmsymtab_start = .;
        KEEP(*(RTMSymTab))
        __rtmsymtab_end = .;

        /* section information for initialization */
        . = ALIGN(4);
        __rt_init_start = .;
        KEEP(*(SORT(.rti_fn*)))
        __rt_init_end = .;
    } > flash

    /* read only data.for example: const int rom_data[3]={1,2,3}; */
    .rodata ALIGN(8) :
    {
        *(.rodata)
        *(.rodata.*)
        *(.gnu.linkonce.r*)
        *(.eh_frame)
    } > flash

    . = ALIGN(4);
    .ctors :
    {
        PROVIDE(__ctors_start__ = .);
        *crtbegin.o(.ctors)
        *crtbegin?.o(.ctors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
        *(SORT(.ctors.*))
        *(.ctors)
        PROVIDE(__ctors_end__ = .);
    } > flash

    .dtors :
    {
        PROVIDE(__dtors_start__ = .);
        *crtbegin.o(.dtors)
        *crtbegin?.o(.dtors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
        *(SORT(.dtors.*))
        *(.dtors)
        PROVIDE(__dtors_end__ = .);
    } > flash

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > flash

    /* The .ARM.exidx section is used for C++ exception handling. */
    /* .ARM.exidx is sorted, so has to go in its own output section.  */
    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx*)
        *(.gnu.linkonce.armexidx.*)
    } > flash

    .rwdata ALIGN(8) :
    {
        /* globals.for example: int ram_data[3]={4,5,6}; VMA in RAM, but keep LMA in flash */
        _begin_data = .;
        . = . + SIZEOF(.data);
        . = ALIGN(4);
    } > flash

    __exidx_end = .;

    .data : AT ( _begin_data )
    {
        *(.data .data.*)
        *(.sdata)

        . = ALIGN(4);
        /* preinit data */
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);

        . = ALIGN(4);
        /* init data */
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE_HIDDEN (__init_array_end = .);

        . = ALIGN(4);
        /* finit data */
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE_HIDDEN (__fini_array_end = .);

        KEEP(*(.jcr*))
        . = ALIGN(4);
        /* All data end */

        *(.gnu.linkonce.d*)
        SORT(CONSTRUCTORS)
    } >ram
    _end_data = .;

    /* Loader will copy data from _flash_begin to _ram_begin..ram_end */
    _data_flash_begin = LOADADDR(.data);
    _data_ram_begin = ADDR(.data);
    _data_ram_end = .;

    /* uninitialized data section - global   int i; */
    .bss ALIGN(8):
    {
        _bss_start = .;
        *(.bss .bss.*)
        *(.scommon)
        *(.sbss)
        *(.dynbss)
        *(COMMON)
        /* Align here to ensure that the .bss section occupies space up to
           _end.  Align after .bss to ensure correct alignment even if the
           .bss section disappears because there are no input sections.  */
        . = ALIGN(32 / 8);
        _bss_end = .;
    } > ram

    . = ALIGN (8);
    _empty_ram = .;

    /* This symbol defines end of code/data sections. Heap starts here. */
    PROVIDE(end = .);

    /* _stack symbol defines initial stack bottom addres. Stack grows to lower addresses.
       Typically you set this to be top of your RAM. Note: code never checks, if stack
       grows into heap area!
    */
    PROVIDE(_stack_unused      = 0x440000 - 0x3F0 - 0x7F0 - 0xFF0 - 0x3F0 - 0x10); /* 0x10*/
    PROVIDE(_stack_svc         = 0x440000 - 0x3F0 - 0x7F0 - 0xFF0 - 0x3F0); /* 0x3F0*/
    PROVIDE(_stack_irq         = 0x440000 - 0x3F0 - 0x7F0 - 0xFF0); /* 0xFF0*/
    PROVIDE(_stack_fiq         = 0x440000 - 0x3F0 - 0x7F0); /* 0x7F0*/
    PROVIDE(_stack_sys         = 0x440000 - 0x3F0); /* 0x3F0*/
}

GROUP(
   libgcc.a
   libg.a
   libc.a
   libm.a
   libnosys.a
 )