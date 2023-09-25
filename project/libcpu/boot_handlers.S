/**
 ****************************************************************************************
 *
 * @file boot_handlers.s
 *
 * @brief ARM Exception Vector handler functions.
 *
 * Copyright (C) RivieraWaves 2011-2016
 *
 ****************************************************************************************
 */
    .section ".rom1", "ax"

    .globl entry_main
    .globl intc_irq
    .globl intc_fiq
    .globl boot_reset
    .globl boot_swi
    .globl boot_undefined
    .globl boot_pabort
    .globl boot_dabort
    .globl boot_reserved
    .globl irq_handler
    .globl fiq_handler
	.globl do_irq
	.globl do_fiq
	.globl do_swi
	.globl do_undefined
	.globl do_pabort
	.globl do_dabort
	.globl do_reserved
    
/* ========================================================================
 *                                Macros
 * ======================================================================== */
#define _FIQ_STACK_SIZE_                  0xFF0
#define _IRQ_STACK_SIZE_                  0xFF0
#define _SVC_STACK_SIZE_                  0x3F0
#define _SYS_STACK_SIZE_                  0x3F0
#define _UND_STACK_SIZE_                  0x010
#define _ABT_STACK_SIZE_                  0x010

#define BOOT_MODE_MASK                    0x1F
#define BOOT_MODE_USR                     0x10
#define BOOT_MODE_FIQ                     0x11
#define BOOT_MODE_IRQ                     0x12
#define BOOT_MODE_SVC                     0x13
#define BOOT_MODE_ABT                     0x17
#define BOOT_MODE_UND                     0x1B
#define BOOT_MODE_SYS                     0x1F
#define BOOT_FIQ_IRQ_MASK                 0xC0
#define BOOT_IRQ_MASK                     0x80

#define BOOT_COLOR_UNUSED                 0xAAAAAAAA      //Pattern to fill UNUSED stack
#define BOOT_COLOR_SVC                    0xBBBBBBBB      //Pattern to fill SVC stack
#define BOOT_COLOR_IRQ                    0xCCCCCCCC      //Pattern to fill IRQ stack
#define BOOT_COLOR_FIQ                    0xDDDDDDDD      //Pattern to fill FIQ stack
#define BOOT_COLOR_SYS                    0xEEEEEEEE      //Pattern to fill SYS stack

/* ========================================================================
                    restore macro definitions
 * ======================================================================== */

/* ========================================================================
/**
 * Macro for switching ARM mode
 */
    .macro  BOOT_CHANGE_MODE, mode, mode_mask
    MRS   R0, CPSR
    BIC   R0, R0, #\mode_mask
    ORR   R0, R0, #\mode
    MSR   CPSR_c, R0
    .endm

/* ========================================================================
/**
 * Macro for setting the stack
 */
    .macro  BOOT_SET_STACK, stackStart, stackLen, color
    LDR   R1, \stackStart
    LDR   R0, \stackLen

    SUB   R0, R1, R0
    MOV   SP, R1        //Set stack pointer

    LDR   R2, =\color

3:
    CMP   R0, R1        //End of stack?
    STRLT R2, [r0]      //Colorize stack word
    ADDLT R0, R0, #4
    BLT   3b           //branch to previous local label
    .endm

/* ========================================================================
/**
 * Push SVC reg
 */
    .macro PUSH_SVC_REG
        SUB     SP, SP, #17 * 4
        STMIA   SP, {R0 - R12}
        MOV     R0, SP
        MRS     R6, SPSR
        STR     LR, [R0, #15*4]
        STR     R6, [R0, #16*4]
        STR     SP, [R0, #13*4]
        STR     LR, [R0, #14*4]
    .endm

/* ========================================================================
 *                      Stack and Heap Definitions
 * ========================================================================
 */
    .section .data
    .space _UND_STACK_SIZE_
    .align 3
    .global und_stack_start
und_stack_start:

    .space _ABT_STACK_SIZE_
    .align 3
    .global abt_stack_start
abt_stack_start:

    .space _FIQ_STACK_SIZE_
    .align 3
    .global fiq_stack_start
fiq_stack_start:

    .space _IRQ_STACK_SIZE_
    .align 3
    .global irq_stack_start
irq_stack_start:

    .skip _SYS_STACK_SIZE_
    .align 3
    .global sys_stack_start
sys_stack_start:

    .space _SVC_STACK_SIZE_
    .align 3
    .global svc_stack_start
svc_stack_start:

/* ========================================================================
 *                                Functions
 * ========================================================================

/* ========================================================================
 * Function to handle reset vector
 */
.section ".rom1", "ax"

boot_reset:
    //Disable IRQ and FIQ before starting anything
    MRS   R0, CPSR
    ORR   R0, R0, #0xC0
    MSR   CPSR_c, R0

    //Setup all stacks //Note: Abt and Usr mode are not used
    BOOT_CHANGE_MODE BOOT_MODE_SYS BOOT_MODE_MASK
    BOOT_SET_STACK   boot_stack_base_SYS boot_stack_len_SYS BOOT_COLOR_SYS

    BOOT_CHANGE_MODE BOOT_MODE_ABT BOOT_MODE_MASK
    BOOT_SET_STACK   boot_stack_base_UNUSED boot_stack_len_UNUSED BOOT_COLOR_UNUSED

    BOOT_CHANGE_MODE BOOT_MODE_UND BOOT_MODE_MASK
    BOOT_SET_STACK   boot_stack_base_UNUSED boot_stack_len_UNUSED BOOT_COLOR_UNUSED

    BOOT_CHANGE_MODE BOOT_MODE_IRQ BOOT_MODE_MASK
    BOOT_SET_STACK   boot_stack_base_IRQ boot_stack_len_IRQ BOOT_COLOR_IRQ

    BOOT_CHANGE_MODE BOOT_MODE_FIQ BOOT_MODE_MASK
    BOOT_SET_STACK   boot_stack_base_FIQ boot_stack_len_FIQ BOOT_COLOR_FIQ

    //Clear FIQ banked registers while in FIQ mode
    MOV     R8,  #0
    MOV     R9,  #0
    MOV     R10, #0
    MOV     R11, #0
    MOV     R12, #0

    BOOT_CHANGE_MODE BOOT_MODE_SVC BOOT_MODE_MASK
    BOOT_SET_STACK   boot_stack_base_SVC boot_stack_len_SVC BOOT_COLOR_SVC

    //Stay in Supervisor Mode
    //copy data from binary to ram
    BL _sysboot_copy_data_to_ram

    ///*Init the BSS section*/
    BL _sysboot_zi_init

    //==================
    //Clear Registers
    MOV R0,  #0
    MOV R1,  #0
    MOV R2,  #0
    MOV R3,  #0
    MOV R4,  #0
    MOV R5,  #0
    MOV R6,  #0
    MOV R7,  #0
    MOV R8,  #0
    MOV R9,  #0
    MOV R10, #0
    MOV R11, #0
    MOV R12, #0

	/* start RT-Thread Kernel */
    ldr     pc, _rtthread_startup
_rtthread_startup:
    .word  rtthread_startup

/*FUNCTION:     _sysboot_copy_data_to_ram*/
/*DESCRIPTION:  copy main stack code from FLASH/ROM to SRAM*/
_sysboot_copy_data_to_ram:
    LDR     R0, =_data_flash_begin
    LDR     R1, =_data_ram_begin
    LDR     R2, =_data_ram_end

4: CMP R1, R2
    LDRLO   R4, [R0], #4
    STRLO   R4, [R1], #4
    BLO     4b
    BX LR

/*FUNCTION:     _sysboot_zi_init*/
/*DESCRIPTION:  Initialise Zero-Init Data Segment*/
_sysboot_zi_init:
    LDR     R0, =_bss_start
    LDR     R1, =_bss_end

    MOV R3, R1
    MOV R4, R0
    MOV R2, #0
5: CMP R4, R3
    STRLO R2, [R4], #4
    BLO 5b
    BX LR

/* ========================================================================
 *                       exception handlers
 * ========================================================================
 */
    .global rt_hw_trap_udef
    .global rt_hw_trap_swi
    .global rt_hw_trap_pabt
    .global rt_hw_trap_dabt
    .global rt_hw_trap_resv
    .global rt_hw_trap_irq
    .global rt_hw_trap_fiq

    .global rt_interrupt_enter
    .global rt_interrupt_leave
    .global rt_thread_switch_interrupt_flag
    .global rt_interrupt_from_thread
    .global rt_interrupt_to_thread

    /* Interrupt */
        .align  5
do_undefined:
    LDMFD   SP!, {R0-R1}
	PUSH_SVC_REG
    BL rt_hw_trap_udef
	B       .

        .align  5
do_swi:
    LDMFD   SP!, {R0-R1}
	PUSH_SVC_REG
    BL rt_hw_trap_swi
	B       .

        .align  5
do_pabort:
    LDMFD   SP!, {R0-R1}
	PUSH_SVC_REG
    BL rt_hw_trap_pabt
	B       .

        .align  5
do_dabort:
    LDMFD   SP!, {R0-R1}
	PUSH_SVC_REG
    BL rt_hw_trap_dabt
	B       .

        .align  5
do_reserved:
    LDMFD   SP!, {R0-R1}
	PUSH_SVC_REG
    BL rt_hw_trap_resv
	B       .

        .align  5
boot_undefined:
	STMFD  sp!,{r0-r1}
	LDR    R1, =0x40000c
	LDR    r0, [R1]
	BX     r0

        .align  5
boot_pabort:
	STMFD  sp!,{r0-r1}
	LDR    R1, =0x400010
	LDR    r0, [R1]
	BX     r0

        .align  5
boot_dabort:
	STMFD  sp!,{r0-r1}
	LDR    R1, =0x400014
	LDR    r0, [R1]
	BX     r0

        .align  5
boot_reserved:
	STMFD  sp!,{r0-r1}
	LDR    R1, =0x400018
	LDR    r0, [R1]
	BX     r0


		.align	5
boot_swi:
	STMFD  sp!,{r0-r1}
	LDR    R1, =0x400008
	LDR    r0, [R1]
	BX     r0

		.align	5
irq_handler:
	STMFD  sp!,{r0-r1}
	LDR    R1, =0x400000
	LDR    r0, [R1]
	BX     r0


		.align	5
fiq_handler:
	STMFD  sp!,{r0-r1}
	LDR    R1, =0x400004
	LDR    r0, [R1]
	BX     r0


        .align  5
do_irq:
    LDMFD   SP!, {R0-R1}
    STMFD   SP!, {R0-R12,LR}

    mrs r4, cpsr
    orr r1, r4, #0xC0          @; disable interrupt
    msr cpsr_c, r1

    BL      rt_interrupt_enter
    BL      rt_irq_dispatch
    BL      rt_interrupt_leave

    LDR     R0, =rt_thread_switch_interrupt_flag
    LDR     R1, [R0]
    CMP     R1, #1
    BEQ     rt_hw_context_switch_interrupt_do

    LDMFD   SP!, {R0-R12,LR}
    SUBS    PC,  LR, #4

rt_hw_context_switch_interrupt_do:
    MOV     R1,  #0
    STR     R1,  [R0]

    MOV     R1, SP
    ADD     SP, SP, #4*4
    LDMFD   SP!, {R4-R12,LR}

    MRS     R0,  SPSR
    SUB     R2,  LR, #4

    MSR     CPSR_c, #BOOT_FIQ_IRQ_MASK|BOOT_MODE_SVC

    STMFD   SP!, {R2}
    STMFD   SP!, {R4-R12,LR}
    LDMFD   R1,  {R1-R4}
    STMFD   SP!, {R1-R4}
    STMFD   SP!, {R0}

    LDR     R4,  =rt_interrupt_from_thread
    LDR     R5,  [R4]
    STR     SP,  [R5]

    LDR     R6,  =rt_interrupt_to_thread
    LDR     R6,  [R6]
    LDR     SP,  [R6]

    LDMFD   SP!, {R4}
    MSR     SPSR_cxsf, R4

    LDMFD   SP!, {R0-R12,LR,PC}^

        .align  5
do_fiq:
    LDMFD   SP!, {R0-R1}
    STMFD   SP!,{R0-R7,LR}

    BL      rt_interrupt_enter
    BL      rt_fiq_dispatch
    BL      rt_interrupt_leave

    MRS R3, spsr
    AND R2, R3, #0x1F
    CMP R2, #0x12            @; fiq from irq(0x12)
    BEQ fiq_handler_return

    LDR     R0, =rt_thread_switch_interrupt_flag
    LDR     R1, [R0]
    CMP     R1, #1
    BEQ     rt_hw_context_switch_interrupt_fiq_do

fiq_handler_return:
    LDMFD   SP!,{R0-R7,LR}
    SUBS    PC, LR, #4

rt_hw_context_switch_interrupt_fiq_do:
    MOV     R1,  #0
    STR     R1,  [R0]

    MOV     R1, SP             @; pop {R0-R7,LR} but skip R0-R3
    ADD     SP, SP, #4*4
    LDMFD   SP!, {R4-R7,LR}

    MRS     R0,  SPSR
    SUB     R2,  LR, #4        @; Save old task's PC to R2

    MSR     CPSR_c, #BOOT_FIQ_IRQ_MASK|BOOT_MODE_SVC

    STMFD   SP!, {R2}          @; Push old task's PC
    STMFD   SP!, {R4-R12,LR}   @; Push old task's LR,R12-R4
    LDMFD   R1,  {R1-R4}       @; pop old thread R0-R3 to R1-R4
    STMFD   SP!, {R1-R4}       @; Push old thread R0-R3
    STMFD   SP!, {R0}          @; Push old task's CPSR

    LDR     R4,  =rt_interrupt_from_thread
    LDR     R5,  [R4]
    STR     SP,  [R5]

    LDR     R6,  =rt_interrupt_to_thread
    LDR     R6,  [R6]
    LDR     SP,  [R6]

    LDMFD   SP!, {R4}
    MSR     SPSR_cxsf, R4

    LDMFD   SP!, {R0-R12,LR,PC}^


/* ========================================================================
 *                                Globals
 * ======================================================================== */
boot_stack_base_UNUSED:
    .word und_stack_start

boot_stack_len_UNUSED:
    .word _UND_STACK_SIZE_

boot_stack_base_IRQ:
    .word irq_stack_start

boot_stack_len_IRQ:
    .word _IRQ_STACK_SIZE_

boot_stack_base_SVC:
    .word svc_stack_start

boot_stack_len_SVC:
     .word _SVC_STACK_SIZE_

boot_stack_base_FIQ:
     .word fiq_stack_start

boot_stack_len_FIQ:
    .word _FIQ_STACK_SIZE_

boot_stack_base_SYS:
     .word sys_stack_start

boot_stack_len_SYS:
     .word _SYS_STACK_SIZE_


/*EOF*/

