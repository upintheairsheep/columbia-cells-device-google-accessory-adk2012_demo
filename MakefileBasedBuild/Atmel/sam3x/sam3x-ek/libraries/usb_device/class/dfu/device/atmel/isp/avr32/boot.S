/*This file is prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief AVR32 UC3 ISP boot.
 *
 * - Compiler:           GNU GCC for AVR32
 * - Supported devices:  All AVR32UC devices with an INTC module can be used.
 *
 * \author               Atmel Corporation: http://www.atmel.com \n
 *                       Support and FAQ: http://support.atmel.no/
 *
 ******************************************************************************/

/* Copyright (c) 2009 Atmel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an Atmel
 * AVR product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 *
 */

#include <avr32/io.h>
#include "conf_isp.h"
#include "parts.h"

#if UC3C
//  These defines are missing from or wrong in the toolchain header file ip_xxx.h or part.h
#ifndef AVR32_WDT_KEY_VALUE
#define AVR32_WDT_KEY_VALUE                            0x00000055
#endif
#define AVR32_SR_M_SUP                                 0x00000001
#define AVR32_SR_M_SIZE                                         3
#define AVR32_SR_M_OFFSET                                      22
#define AVR32_SRAM_ADDRESS                             0x00000000
#endif

//! @{
//! \verbatim


  // Performs efficiently a bitwise logical Exclusive-OR between the specified
  // register and an immediate value of up to 32 bits. The result is stored in
  // the destination register.
  .macro  eor.w   rd, imm
    .if \imm & 0x0000FFFF
      eorl    \rd, LO(\imm)
    .endif
    .if \imm & 0xFFFF0000
      eorh    \rd, HI(\imm)
    .endif
  .endm

  // Moves efficiently an immediate value of up to 32 bits into a register.
  .macro  mov.w   rd, imm
    .if ((-(1 << (21 - 1))) <= \imm) && (\imm <= ((1 << (21 - 1)) - 1))
      mov     \rd, \imm
#if __AVR32_UC__ >= 2
    .elseif !(\imm & 0x0000FFFF)
      movh    \rd, HI(\imm)
#endif
    .else
      mov     \rd, LO(\imm)
      orh     \rd, HI(\imm)
    .endif
  .endm

  // Performs efficiently a bitwise logical OR between the specified register
  // and an immediate value of up to 32 bits. The result is stored in the
  // destination register.
  .macro  or.w    rd, imm
    .if \imm & 0x0000FFFF
      orl     \rd, LO(\imm)
    .endif
    .if \imm & 0xFFFF0000
      orh     \rd, HI(\imm)
    .endif
  .endm


  .section  .reset, "ax", @progbits


  .balign 2

  // Reset vector: This must be linked @ 0x80000000.
  .global _start
  .type _start, @function
_start:
  mov.w   r8, ISP_KEY_ADDRESS               // r8 = &ISP_KEY
  mov.w   r9, AVR32_WDT_ADDRESS             // r9 = &AVR32_WDT
  mov.w   r10, ISP_CFG1_ADDRESS             // r10 = &ISP_CFG1
  mov.w   r11, AVR32_PM_ADDRESS             // r11 = &AVR32_PM
  ld.w    r0, r8[0]                         // r0 = ISP_KEY[0]
  mov.w   r1, ISP_KEY_VALUE                 // r1 = ISP_KEY_VALUE
  
  // Check CRC8 validity of the user page configuration word1.
  ld.w    r2, r10[0]                        // r2 = config_word1
  mov.w   r3, ISP_CFG1_CRC8_POLYNOMIAL      // r3 = crc8_polynomial
  mov     r4, r2                            // r4 = config_word1_tempo
  rcall   test_crc8_end
  brne    start_loader  // The config word1 CRC8 is invalid: launch the ISP process.
  
  // Check the magic key of the user page configuration word1.
  bfextu  r3, r2, ISP_CFG1_BOOT_KEY1_OFFSET, ISP_CFG1_BOOT_KEY1_SIZE // r3 = ISP_BOOT_KEY1
  cp.w    r3, ISP_CFG1_BOOT_KEY1_VALUE
  brne    start_loader  // The config word1 magic key is incorrect: launch the ISP process

  // Test the value of the ISP_FORCE bit.
test_isp_force:
  bld     r2, ISP_CFG1_FORCE_OFFSET
  brcs    start_loader  // The ISP_FORCE bit is set: launch the ISP process.
  
  // Test the reset cause.
  ld.w    r3, r11[AVR32_PM_RCAUSE]          // r3 = AVR32_PM.RCAUSE
#if (UC3A || UC3B)
  mov.w   r4, AVR32_PM_RCAUSE_POR_MASK |\
              AVR32_PM_RCAUSE_EXT_MASK |\
              AVR32_PM_RCAUSE_JTAG_MASK |\
              AVR32_PM_RCAUSE_OCDRST_MASK |\
              AVR32_PM_RCAUSE_JTAGHARD_MASK // r4 = POR|EXT|OCD|JTAG
#elif UC3C  
  mov.w   r4, AVR32_PM_RCAUSE_POR_MASK |\
              AVR32_PM_RCAUSE_EXT_MASK |\
              AVR32_PM_RCAUSE_JTAG_MASK |\
              AVR32_PM_RCAUSE_OCDRST_MASK |\
							AVR32_PM_RCAUSE_BOD33_MASK |\
							AVR32_PM_RCAUSE_BOD_MASK  // r4 = POR|EXT|OCD|JTAG|BOD33|BOD
#else
#error 'Part configuration is missing'
#endif              
  tst     r3, r4
  brne    manage_io_cond  // The reset cause was one of POR|EXT|OCD|JTAG|JTAGHARD: check the IO conditions that control the ISP activation.

  // Was the reset cause a WDT reset?
  bld     r3, AVR32_PM_RCAUSE_WDT_OFFSET
  brcs    start_program // The reset cause was a WDT reset: start the application

  // ISP RAM Key set?
  cp.w    r0, r1
  brne    start_program_no_isp_key  // Start the application

  // Start of the ISP process
start_loader:
  rcall   disable_wdt                       // Disable the WDT
  st.w    r8[0], r1                         // Set ISP RAM Key

  // Set initial stack pointer.
  mov     sp, _estack

  // Disable the exception processing.
  ssrf    AVR32_SR_EM_OFFSET

  // Set up EVBA so interrupts can be enabled.
  mov     r0, _evba
  mtsr    AVR32_EVBA, r0

  // Load initialized data having a global lifetime from the data LMA.
  mov     r0, _data
  mov     r1, _edata
  sub     r2, pc, $ - _data_lma
  rcall   load_idata

  // Clear uninitialized data having a global lifetime in the blank static storage section.
  mov     r0, __bss_start
  mov     r1, _end
  mov.w   r2, 0
  mov.w   r3, 0
  rjmp    clear_udata
clear_udata_loop:
  st.d    r0++, r2
clear_udata:
  cp.w    r0, r1
  brlo    clear_udata_loop

  // Load constant data and code from the const LMA.
  mov     r0, _const
  mov     r1, _econst
  sub     r2, pc, $ - _const_lma
  rcall   load_idata

  // Call the ISP main function, which must not return.
  call    main

  // Check of the IO conditions that control the ISP activation.
manage_io_cond:
  // Test the ISP_IO_COND_EN bit.
  bld     r2, ISP_CFG1_IO_COND_EN_OFFSET
  brcc    start_program                     // ISP_IO_COND_EN is 0: start the application.

  // Check CRC8 validity of the user page configuration word2.
  mov.w   r10, ISP_CFG2_ADDRESS
  ld.w    r2, r10[0]                        // r2 = config_word2
  mov.w   r3, ISP_CFG2_CRC8_POLYNOMIAL      // r3 = crc8_polynomial
  mov     r4, r2                            // r4 = config_word2_tempo
  rcall   test_crc8_end
  brne    start_loader  // The config word2 CRC8 is invalid: launch the ISP process.

  // Check the magic key of the user page configuration word2
  bfextu  r3, r2, ISP_CFG2_BOOT_KEY_OFFSET, ISP_CFG2_BOOT_KEY_SIZE
  cp.w    r3, ISP_CFG2_BOOT_KEY_VALUE
  brne    start_loader  // The config word2 magic key is incorrect: launch the ISP process

  // ISP IO Condition active?
  bfextu  r3, r2, ISP_CFG2_IO_COND_PIN_OFFSET, ISP_CFG2_IO_COND_PIN_SIZE  // r3 = IO_COND_PIN
  cp.w    r3, AVR32_GPIO_NUMBER_OF_PINS
  brhs    start_loader  // The IO pin is invalid: launch the ISP process
  
  // Check the IO condition
  mov.w   r10, AVR32_GPIO_ADDRESS
  lsr     r4, r3, 5                         // r4 = IO_COND_PIN/32 = GPIO_port_IO_COND_PIN
  lsl     r4, 8                             // r4 = offset to the port of the IO_COND_PIN
  add     r10, r4                           // r10 = &(AVR32_GPIO[port of the IO_COND_PIN].GPER)
  ld.w    r4, r10[AVR32_GPIO_PVR]           // r4 = AVR32_GPIO[port of the IO_COND_PIN].PVR
  andl    r3, 0x1F                          // r3 = pin&0x1F
  lsr     r4, r4, r3                        // r4 = gpio_port->pvr >> (pin & 0x1F)
  bfextu  r3, r2, ISP_CFG2_IO_COND_LEVEL_OFFSET, ISP_CFG2_IO_COND_LEVEL_SIZE  // r3 = IO_COND_LEVEL
  eor     r4, r3
  bld     r4, 0
  brcc    start_loader  // The ISP IO Condition is not active: launch the ISP process

start_program:
  cp.w    r0, r1
  brne    start_program_no_isp_key
  rcall   disable_wdt
  .global boot_program
  .type boot_program, @function
boot_program:
  // Clear ISP RAM Key
  mov.w   r8, ISP_KEY_ADDRESS
  mov.w   r0, 0
  st.w    r8[0], r0
start_program_no_isp_key:
  mov.w   r0, AVR32_SR_GM_MASK | AVR32_SR_EM_MASK | (AVR32_SR_M_SUP << AVR32_SR_M_OFFSET)
  mtsr    AVR32_SR, r0
  .irp    rd, r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, sp, lr
    mov.w   \rd, 0
  .endr
  mtsr    AVR32_EVBA, r0
  mtsr    AVR32_COUNT, r0
  lddpc   pc, program_start_address

disable_wdt:
  mov.w   r2, AVR32_WDT_KEY_VALUE << AVR32_WDT_CTRL_KEY_OFFSET
  st.w    r9[AVR32_WDT_CTRL], r2
  eor.w   r2, AVR32_WDT_CTRL_KEY_MASK
  st.w    r9[AVR32_WDT_CTRL], r2
  mov     pc, lr

load_idata_loop:
  ld.d    r4, r2++
  st.d    r0++, r4
load_idata:
  cp.w    r0, r1
  brlo    load_idata_loop
  mov     pc, lr
  
  // Common CRC8 function
crc8:
  clz     r5, r4
  rsub    r5, r5, 32 - 9
  lsl     r5, r3, r5
  eor     r4, r5
#if 0
  .global test_crc8_end
  .type test_crc8_end, @function
#endif
test_crc8_end:
  cp.w    r4, 0xFF
  brhi    crc8
  cp.w    r4, 0
  retal   r4


// Constant data area.

  .balign 4

program_start_address:
  .word PROGRAM_START_ADDRESS


  .section  .evba, "ax", @progbits


  .balign 2

  // Start of exception vector table: Unrecoverable exception.
  .global _evba
  .type _evba, @function
_evba:
  lda.w   r8, _start
  mov.w   r9, AVR32_SR_GM_MASK | AVR32_SR_EM_MASK | (AVR32_SR_M_SUP << AVR32_SR_M_OFFSET)
  mov     sp, _estack - 6 * 4
  pushm   r8-r9
  rete


//! \endverbatim
//! @}
