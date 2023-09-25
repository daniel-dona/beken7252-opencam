;/**
 ;****************************************************************************************
 ;*
 ;* @file boot_vectors.s
 ;*
 ;* @brief ARM Exception Vectors table.
 ;*
 ;* Copyright (C) RivieraWaves 2009-2015
 ;*
 ;* $Rev:  $
 ;*
 ;****************************************************************************************
 ;*/


	
	EXPORT vectors
	
	IMPORT sys_Reset
	
	IMPORT IRQ_Exception
	IMPORT FIQ_Exception	
		
	IMPORT	SYSirq_IRQ_Handler
	IMPORT	SYSirq_FIQ_Handler
		
	IMPORT Undefined_Exception
	IMPORT SoftwareInterrupt_Exception
		
	IMPORT Reserved_Exception
	IMPORT PrefetchAbort_Exception
	IMPORT DataAbort_Exception
		
	PRESERVE8

	CODE32
    AREA SYS_BOOT, CODE, READONLY
    ENTRY
		
vectors
    ; reset handler
    LDR     PC,       boot_reset
    ; undefined handler
    LDR     PC,       boot_undefined
    ; SWI handler
    LDR     PC,		  boot_swi
    ; Prefetch error handler
    LDR     PC,       boot_pabort
    ; abort handler
    LDR     PC,       boot_dabort
    ; reserved vector
	LDR     PC,		  boot_reserved
    ; irq
    LDR     PC,       intc_irq
    ; fiq
    LDR     PC,       intc_fiq
	
	

boot_reset          DCD     sys_Reset
boot_undefined      DCD     Undefined
boot_swi            DCD     SoftwareInterrupt
boot_pabort        	DCD     PrefetchAbort
boot_dabort       	DCD     DataAbort
boot_reserved       DCD     sys_reserved
intc_irq            DCD     IRQ_Handler
intc_fiq            DCD     FIQ_Handler

;*******************************************************************************
;* Function Name  : UndefinedHandler
;* Description    : This function called when undefined instruction exception
;*                  is entered.
;* Input          : none
;* Output         : none
;*******************************************************************************
Undefined
        B       Undefined_Exception

;*******************************************************************************
;* Function Name  : SWIHandler
;* Description    : This function called when SWI instruction executed.
;* Input          : none
;* Output         : none
;*******************************************************************************
SoftwareInterrupt
        B       SoftwareInterrupt_Exception

PrefetchAbort
        B       PrefetchAbort_Exception

;*******************************************************************************
;* Function Name  : DataAbortHandler
;* Description    : This function is called when Data Abort exception is entered.
;* Input          : none
;* Output         : none
;*******************************************************************************
DataAbort
        B       DataAbort_Exception

;*******************************************************************************
;* Function Name  : IRQHandler
;* Description    : This function called when IRQ exception is entered.
;* Input          : none
;* Output         : none
;*******************************************************************************
IRQ_Handler
        B      IRQ_Exception
       
sys_reserved
		B	   Reserved_Exception

;*******************************************************************************
;* Function Name  : FIQHandler
;* Description    : This function is called when FIQ exception is entered.
;* Input          : none
;* Output         : none
;*******************************************************************************
FIQ_Handler
      
        B      FIQ_Exception
       
	
	

	
	

	END