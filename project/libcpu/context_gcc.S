;/*
; * File      : context_iar.S
; * This file is part of RT-Thread RTOS
; * COPYRIGHT (C) 2006, RT-Thread Development Team
; */

#define NOINT   0xC0

;/*
; * rt_base_t rt_hw_interrupt_disable();
; */
    .globl rt_hw_interrupt_disable
    .type rt_hw_interrupt_disable, %function
rt_hw_interrupt_disable:
    MRS     R0, CPSR
    ORR     R1, R0, #NOINT
    MSR     CPSR_c, R1
    BX      LR

/*
 * void rt_hw_interrupt_enable(rt_base_t level);
 */
    .globl rt_hw_interrupt_enable
    .type rt_hw_interrupt_enable, %function
rt_hw_interrupt_enable:
    MSR     CPSR_c, R0
    BX      LR

/*
 * void rt_hw_context_switch(rt_uint32 from, rt_uint32 to);
 * r0 --> from
 * r1 --> to
 */
    .globl rt_hw_context_switch
    .type rt_hw_context_switch, %function
rt_hw_context_switch:
    STMFD   SP!, {LR}               @; push pc (lr should be pushed in place of pc)
    STMFD   SP!, {R0-R12, LR}       @; push lr & register file
    MRS     R4, CPSR
    tst     lr, #0x01
    beq     _ARM_MODE
    orr     r4, r4, #0x20           @; it's thumb code set T=1

_ARM_MODE:
    STMFD   SP!, {R4}               @; push cpsr
    STR     SP, [R0]                @; store sp in preempted tasks tcb
    LDR     SP, [R1]                @; get new task stack pointer
    LDMFD   SP!, {R4}               @; pop new task spsr
    MSR     SPSR_cxsf, R4
    LDMFD   SP!, {R0-R12, LR, PC}^  @; pop new task r0-r12, lr & pc

/*
 * void rt_hw_context_switch_to(rt_uint32 to);
 * r0 --> to
 */
    .globl rt_hw_context_switch_to
    .type rt_hw_context_switch_to, %function
rt_hw_context_switch_to:
    LDR     SP, [R0]                @; get new task stack pointer
    LDMFD   SP!, {R4}               @; pop new task cpsr
    MSR     SPSR_cxsf, R4
    LDMFD   SP!, {R0-R12, LR, PC}^  @; pop new task r0-r12, lr & pc

/*
 * void rt_hw_context_switch_interrupt(rt_uint32 from, rt_uint32 to);
 */
    .globl rt_thread_switch_interrupt_flag
    .globl rt_interrupt_from_thread
    .globl rt_interrupt_to_thread
    .globl rt_hw_context_switch_interrupt
    .type rt_hw_context_switch_interrupt, %function
rt_hw_context_switch_interrupt:
    LDR     R2, =rt_thread_switch_interrupt_flag
    LDR     R3, [R2]
    CMP     R3, #1
    BEQ     _reswitch
    MOV     R3, #1                          @; set flag to 1
    STR     R3, [R2]
    LDR     R2, =rt_interrupt_from_thread   @; set rt_interrupt_from_thread
    STR     R0, [R2]
_reswitch:
    LDR     R2, =rt_interrupt_to_thread     @; set rt_interrupt_to_thread
    STR     R1, [R2]
    BX      LR
