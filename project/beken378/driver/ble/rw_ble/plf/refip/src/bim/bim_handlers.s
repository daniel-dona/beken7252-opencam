;/**
; ****************************************************************************************
; *
; * @file boot_handlers.s
; *
; * @brief ARM Exception Vector handler functions.
; *
; * Copyright (C) BEKEN 2009-2015
; *
; * $Rev:  $
; *
; ****************************************************************************************
; */

	
    PRESERVE8
    AREA SYS_BOOT, CODE, READONLY
	
	EXPORT sys_Reset
	
		
	EXPORT boot_stack_base_UNUSED
	EXPORT boot_stack_len_UNUSED
	EXPORT boot_stack_base_SVC
	EXPORT boot_stack_len_SVC
	EXPORT boot_stack_base_IRQ
	EXPORT boot_stack_len_IRQ
	EXPORT boot_stack_base_FIQ
	EXPORT boot_stack_len_FIQ		
	;EXPORT _sysboot_copy_data_to_ram
	;EXPORT _sysboot_zi_init
	
	IMPORT ||Load$$RAM_DATA$$Base||  
	
	IMPORT ||Image$$RAM_DATA$$Base||               ;//ram_data_base
	IMPORT ||Image$$RAM_DATA$$Length||             ;//ram_data_length
	
	IMPORT ||Image$$RAM_BSS$$Base||                ;//bss_base
	IMPORT ||Image$$RAM_BSS$$Length||              ;//image_bss_length
	IMPORT ||Image$$RAM_BSS$$ZI$$Length||          ;//bss_length
	
	IMPORT ||Image$$RAM_STACK_UNUSED$$Base||       ;//stack_base_unused
	IMPORT ||Image$$RAM_STACK_UNUSED$$ZI$$Length|| ;//stack_len_unused	
	
	IMPORT ||Image$$RAM_STACK_IRQ$$Base||          ;//stack_base_irq
	IMPORT ||Image$$RAM_STACK_IRQ$$ZI$$Length||    ;//stack_len_irq	
	
	IMPORT ||Image$$RAM_STACK_SVC$$Base||          ;//stack_base_svc
	IMPORT ||Image$$RAM_STACK_SVC$$ZI$$Length||    ;//stack_len_svc	
	
	IMPORT ||Image$$RAM_STACK_FIQ$$Base||          ;//stack_base_fiq
	IMPORT ||Image$$RAM_STACK_FIQ$$ZI$$Length||    ;//stack_len_fiq
	
	IMPORT bim_main
		
	
			

_FIQ_STACK_SIZE_ 	        EQU   0x1F0
_IRQ_STACK_SIZE_ 	        EQU   0x1F0
_SVC_STACK_SIZE_ 	        EQU   0x1F0
_UNUSED_STACK_SIZE_ 	    EQU   0x010
;/* ========================================================================
; *                                Constants
; * ======================================================================== */

BOOT_MODE_MASK  	EQU 0x1F

BOOT_MODE_USR   	EQU 0x10
BOOT_MODE_FIQ   	EQU 0x11
BOOT_MODE_IRQ   	EQU 0x12
BOOT_MODE_SVC   	EQU 0x13
BOOT_MODE_ABT   	EQU 0x17
BOOT_MODE_UND   	EQU 0x1B
BOOT_MODE_SYS    	EQU 0x1F
I_BIT               EQU    0x80
F_BIT               EQU    0x40

BOOT_FIQ_IRQ_MASK 	EQU 0xC0
BOOT_IRQ_MASK   	EQU 0x80
	
	

BOOT_PATTERN_UNUSED  EQU 0xAAAAAAAA      ;// Pattern to fill UNUSED stack
BOOT_PATTERN_SVC     EQU 0xBBBBBBBB      ;// Pattern to fill SVC stack
BOOT_PATTERN_IRQ     EQU 0xCCCCCCCC      ;// Pattern to fill IRQ stack
BOOT_PATTERN_FIQ     EQU 0xDDDDDDDD      ;// Pattern to fill FIQ stack


;/* ========================================================================
; *                                Macros
; * ======================================================================== */

;/* ========================================================================
;/**
; * Macro for switching ARM mode
; */
	MACRO
	BOOT_CHANGE_MODE $newmode
        ;MRS   R0, CPSR
        ;BIC   R0, R0, #BOOT_MODE_MASK
        ;ORR   R0, R0, #BOOT_MODE_$newmode:OR:I_BIT:OR:F_BIT
		MOV   R0, #BOOT_MODE_$newmode:OR:I_BIT:OR:F_BIT
        MSR   CPSR_c, R0
    MEND

;/* ========================================================================
;/**
; * Macro for setting the stack
; 
   MACRO
	BOOT_SET_STACK $stackname


        LDR   R0, boot_stack_base_$stackname		
		LDR   R1, boot_stack_len_$stackname
		ADD   R3, R1, R0
		MOV   R4, R0	
		MOV   SP, R3		
		LDR   R2, =BOOT_PATTERN_$stackname
		
100    
		CMP   R4, R3
        STRLO R2, [R4], #4
        BLO %b100  
	
    MEND


	

;/* ========================================================================
; *                                Globals
; * ======================================================================== */

;/* ========================================================================


;/* ========================================================================
;/**
; * RAM_BSS
; */

ram_bss_base DCD ||Image$$RAM_BSS$$Base||


ram_bss_length DCD ||Image$$RAM_BSS$$ZI$$Length||

;/* ========================================================================
;/**
; * Unused (ABT, UNDEFINED, SYSUSR) Mode
; */

boot_stack_base_UNUSED DCD ||Image$$RAM_STACK_UNUSED$$Base||


boot_stack_len_UNUSED DCD ||Image$$RAM_STACK_UNUSED$$ZI$$Length||

;/* ========================================================================
;/**
; * IRQ Mode
; */

boot_stack_base_IRQ DCD ||Image$$RAM_STACK_IRQ$$Base||


boot_stack_len_IRQ DCD ||Image$$RAM_STACK_IRQ$$ZI$$Length||



;/* ========================================================================
;/**
; * Supervisor Mode
; */

boot_stack_base_SVC DCD ||Image$$RAM_STACK_SVC$$Base||

boot_stack_len_SVC  DCD ||Image$$RAM_STACK_SVC$$ZI$$Length||

;/* ========================================================================
;/**
; * FIQ Mode
; */

boot_stack_base_FIQ DCD ||Image$$RAM_STACK_FIQ$$Base||


boot_stack_len_FIQ DCD ||Image$$RAM_STACK_FIQ$$ZI$$Length||

;/* ========================================================================
; *                                Functions
; * ========================================================================

;/* ========================================================================
;/**
; * Function to handle reset vector
; */
sys_Reset
    ; Disable IRQ and FIQ before starting anything
  ;  MRS   R0, CPSR
  ;  ORR   R0, R0, #0xC0
  ;  MSR   CPSR_c, R0

    ; ==================
    ; Setup all stacks

    ; Note: Sys and Usr mode are not used
		
    BOOT_CHANGE_MODE SYS
    BOOT_SET_STACK   UNUSED
    BOOT_CHANGE_MODE ABT
    BOOT_SET_STACK   UNUSED
    BOOT_CHANGE_MODE UND
    BOOT_SET_STACK   UNUSED
    BOOT_CHANGE_MODE IRQ
    BOOT_SET_STACK   IRQ
    BOOT_CHANGE_MODE FIQ
    BOOT_SET_STACK   FIQ

    ; Clear FIQ banked registers while in FIQ mode
    MOV     R8, #0
    MOV     R9, #0
    MOV     R10, #0
    MOV     R11, #0
    MOV     R12, #0

    BOOT_CHANGE_MODE SVC
    BOOT_SET_STACK   SVC
	
	

    ; Stay in Supervisor Mode
    ;copy data from binary to ram
    BL _sysboot_copy_data_to_ram
    
	; init bss section
    BL _sysboot_zi_init
	 
    ; Clear Registers
    MOV R0, #0
    MOV R1, #0
    MOV R2, #0
    MOV R3, #0
    MOV R4, #0
    MOV R5, #0
    MOV R6, #0
    MOV R7, #0
    MOV R8, #0
    MOV R9, #0
    MOV R10, #0
    MOV R11, #0
    MOV R12, #0
	

    B bim_main

	 
	; /*FUNCTION:     _sysboot_copy_data_to_ram*/
; /*DESCRIPTION:  copy main stack code from FLASH/ROM to SRAM*/
_sysboot_copy_data_to_ram
        LDR     R0, =||Load$$RAM_DATA$$Base||
        LDR     R1, =||Image$$RAM_DATA$$Base||
        LDR     R2, =||Image$$RAM_DATA$$Length||
		
		
        MOV     R3, R1
        ADD     R3, R3, R2
_rw_copy                                
        CMP R1, R3
        LDRLO   R4, [R0], #4
        STRLO   R4, [R1], #4
        BLO     _rw_copy
        BX LR
        
; /*FUNCTION:     _sysboot_zi_init*/
; /*DESCRIPTION:  Initialise Zero-Init Data Segment*/;
_sysboot_zi_init
        LDR     R0, =||Image$$RAM_BSS$$Base||
        LDR     R1, =||Image$$RAM_BSS$$ZI$$Length||
        
        ADD R3, R1, R0
        MOV R4, R0
        MOV R2, #0
_zi_loop
        CMP R4, R3
        STRLO R2, [R4], #4
        BLO _zi_loop
        BX LR

	END