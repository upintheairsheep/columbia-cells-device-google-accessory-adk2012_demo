/**
 * \file
 *
 * \brief USBB OTG Driver header file.
 *
 * Copyright (C) 2009 Atmel Corporation. All rights reserved.
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 * Atmel AVR product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#ifndef _USBB_OTG_H_
#define _USBB_OTG_H_

#include "compiler.h"
//#include "preprocessor.h"

//! \ingroup usb_group
//! \defgroup otg_group USB OTG Driver (OTG)
//! USBB low-level driver for OTG features
//!
//! @warning Bit-masks are used instead of bit-fields because PB registers
//! require 32-bit write accesses while AVR32-GCC 4.0.2 builds 8-bit
//! accesses even when volatile unsigned int bit-fields are specified.
//! @{

//! @name USBB IP properties
//! These macros give access to IP properties
//! @{
   //! Get IP name part 1 or 2
#define  otg_get_ip_name()                   (((uint64_t)AVR32_USBB.uname2<<32)|(uint64_t)AVR32_USBB.uname1)
   //! Instruction to access at a peripheral register after interrupt clear, see AVR32002 - AVR32UC Technical reference $6.6 Memory barriers
#define  otg_data_memory_barrier()           
   //! Get IP version
#define  otg_get_ip_version()                (Rd_bitfield(AVR32_USBB.uvers, AVR32_USBB_UVERS_VERSION_NUM_MASK))
   //! Get number of metal fixes
#define  otg_get_metal_fix_nbr()             (Rd_bitfield(AVR32_USBB.uvers, AVR32_USBB_UVERS_METAL_FIX_NUM_MASK))
   //! Get number of hardware-implemented DMA channels
#define  otg_get_dma_channel_nbr()           (Rd_bitfield(AVR32_USBB.ufeatures, AVR32_USBB_UFEATURES_DMA_CHANNEL_NBR_MASK))
   //! Get DMA buffer size
#define  otg_get_dma_buffer_size()           (Rd_bitfield(AVR32_USBB.ufeatures, AVR32_USBB_UFEATURES_DMA_BUFFER_SIZE_MASK))
   //! Get DMA FIFO depth in words
#define  otg_get_dma_fifo_word_depth()       (((Rd_bitfield(AVR32_USBB.ufeatures, AVR32_USBB_UFEATURES_DMA_FIFO_WORD_DEPTH_MASK) - 1) & ((1 << AVR32_USBB_UFEATURES_DMA_FIFO_WORD_DEPTH_SIZE) - 1)) + 1)
   //! Get DPRAM size (FIFO maximal size) in bytes
#define  otg_get_dpram_size()                (128 << Rd_bitfield(AVR32_USBB.ufeatures, AVR32_USBB_UFEATURES_FIFO_MAX_SIZE_MASK))
   //! Test if DPRAM is natively byte write capable
#define  Is_otg_dpram_byte_write_capable()   (Tst_bits(AVR32_USBB.ufeatures, AVR32_USBB_UFEATURES_BYTE_WRITE_DPRAM_MASK))
   //! Get size of USBB PB address space
#define  otg_get_ip_paddress_size()          (AVR32_USBB.uaddrsize)
//! @}

//! @name USBB OTG ID pin management
//! These macros manage the ID pin use or not to switch between Host or Device mode
//! @{
   //! Pin and function for USB_ID according to configuration from OTG_ID
//#define  OTG_ID_PIN                          ATPASTE2(OTG_ID, _PIN)
//#define  OTG_ID_FUNCTION                     ATPASTE2(OTG_ID, _FUNCTION)

   //! Input OTG_ID from its pin
#if 0
#define  otg_input_id_pin() \
{\
   (Tst_bits(OTG_ID_FUNCTION, 0x01)) ?\
      (AVR32_GPIO.port[OTG_ID_PIN >> 5].pmr0s = 1 << (OTG_ID_PIN & 0x1F)) :\
      (AVR32_GPIO.port[OTG_ID_PIN >> 5].pmr0c = 1 << (OTG_ID_PIN & 0x1F)); \
   (Tst_bits(OTG_ID_FUNCTION, 0x02)) ?\
      (AVR32_GPIO.port[OTG_ID_PIN >> 5].pmr1s = 1 << (OTG_ID_PIN & 0x1F)) :\
      (AVR32_GPIO.port[OTG_ID_PIN >> 5].pmr1c = 1 << (OTG_ID_PIN & 0x1F)); \
   AVR32_GPIO.port[OTG_ID_PIN >> 5].gperc = 1 << (OTG_ID_PIN & 0x1F);\
   AVR32_GPIO.port[OTG_ID_PIN >> 5].puers = 1 << (OTG_ID_PIN & 0x1F);\
}
#endif
#define  otg_input_id_pin() \
{\
    static const uotgIdPin = {PIN_UOTGHS_ID};\
    PIO_PinConfigure(&uotgIdPin, 1);\
}
   //! Test if OTG_ID is input from its pin
#if 0
#define  Is_otg_id_pin_input() \
            ( !Tst_bits(AVR32_GPIO.port[OTG_ID_PIN >> 5].gper, 1 << (OTG_ID_PIN & 0x1F)) &&\
               Tst_bits(AVR32_GPIO.port[OTG_ID_PIN >> 5].pmr0, 1 << (OTG_ID_PIN & 0x1F)) == Tst_bits(OTG_ID_PIN, 0x01) &&\
               Tst_bits(AVR32_GPIO.port[OTG_ID_PIN >> 5].pmr1, 1 << (OTG_ID_PIN & 0x1F)) == Tst_bits(OTG_ID_PIN, 0x02))
#endif
#define  Is_otg_id_pin_input() \
            ( Tst_bits(PIOB->PIO_PDSR, PIO_PB11) )

   //! Enable external OTG_ID pin (listened to by USB)
#define  otg_enable_id_pin()                 (Set_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_UIDE))
   //! Disable external OTG_ID pin (ignored by USB)
#define  otg_disable_id_pin()                (Clr_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_UIDE))
   //! Test if external OTG_ID pin enabled (listened to by USB)
#define  Is_otg_id_pin_enabled()             (Tst_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_UIDE))
   //! Disable external OTG_ID pin and force device mode
#define  otg_force_device_mode()             (Set_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_UIMOD), otg_disable_id_pin())
   //! Test if device mode is forced
#define  Is_otg_device_mode_forced()         (!Is_otg_id_pin_enabled() && Tst_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_UIMOD))
   //! Disable external OTG_ID pin and force host mode
#define  otg_force_host_mode()               (Clr_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_UIMOD), otg_disable_id_pin())
   //! Test if host mode is forced
#define  Is_otg_host_mode_forced()           (!Is_otg_id_pin_enabled() && !Tst_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_UIMOD))

//! @name USBB OTG ID pin interrupt management
//! These macros manage the ID pin interrupt
//! @{
#define  otg_enable_id_interrupt()           (Set_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_IDTE))
#define  otg_disable_id_interrupt()          (Clr_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_IDTE))
#define  Is_otg_id_interrupt_enabled()       (Tst_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_IDTE))
#define  Is_otg_id_device()                  (Tst_bits(UOTGHS->UOTGHS_SR, UOTGHS_SR_ID))
#define  otg_ack_id_transition()             (UOTGHS->UOTGHS_SCR = UOTGHS_SCR_IDTIC)
#define  otg_raise_id_transition()           (UOTGHS->UOTGHS_SFR = UOTGHS_SFR_IDTIS)
#define  Is_otg_id_transition()              (Tst_bits(UOTGHS->UOTGHS_SR, UOTGHS_SR_IDTI))
//! @}

//! @}

//! @name USBB OTG main management
//! These macros allows to enable/disable pad and USBB hardware
//! @{
  //! Enable USB macro
#define  otg_enable()                        (Set_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_USBE))
  //! Disable USB macro                     
#define  otg_disable()                       (Clr_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_USBE))
#define  Is_otg_enabled()                    (Tst_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_USBE))

  //! Enable OTG pad                        
#define  otg_enable_pad()                    (Set_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_OTGPADE))
  //! Disable OTG pad                       
#define  otg_disable_pad()                   (Clr_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_OTGPADE))
#define  Is_otg_pad_enabled()                (Tst_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_OTGPADE))

  //! Check Clock Usable               
  //! For parts with HS feature, this one corresponding at UTMI clock
#define  Is_clock_usable()                   (Tst_bits(UOTGHS->UOTGHS_SR, UOTGHS_SR_CLKUSABLE))

  //! Stop (freeze) internal USB clock
#define  otg_freeze_clock()                  (Set_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_FRZCLK))
#define  otg_unfreeze_clock()                (Clr_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_FRZCLK))
#define  Is_otg_clock_frozen()               (Tst_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_FRZCLK))

  //! Configure time-out of specified OTG timer
#define  otg_configure_timeout(timer, timeout) (Set_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_UNLOCK),\
                                             Wr_bitfield(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_TIMPAGE_Msk, timer),\
                                             Wr_bitfield(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_TIMVALUE_Msk, timeout),\
                                             Clr_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_UNLOCK))
  //! Get configured time-out of specified OTG timer
#define  otg_get_timeout(timer)              (Set_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_UNLOCK),\
                                             Wr_bitfield(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_TIMPAGE_Msk, timer),\
                                             Clr_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_UNLOCK),\
                                             Rd_bitfield(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_TIMVALUE_Msk))


  //! Get the dual-role device state of the internal USB finite state machine of the USBB controller
#define  otg_get_fsm_drd_state()             (Rd_bitfield(UOTGHS->UOTGHS_FSM, UOTGHS_FSM_DRDSTATE_Msk))
//! @}

//! @name USBB OTG hardware protocol
//! These macros manages the hardware OTG protocol
//! @{
  //! initiates a Host Negociation Protocol
#define  otg_device_initiate_hnp()           (Set_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_HNPREQ))
  //! accepts a Host Negociation Protocol
#define  otg_host_accept_hnp()               (Set_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_HNPREQ))
  //! rejects a Host Negociation Protocol
#define  otg_host_reject_hnp()               (Clr_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_HNPREQ))
  //! initiates a Session Request Protocol
#define  otg_device_initiate_srp()           (Set_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_SRPREQ))
  //! selects VBus as SRP method
#define  otg_select_vbus_srp_method()        (Set_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_SRPSEL))
#define  Is_otg_vbus_srp_method_selected()   (Tst_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_SRPSEL))
  //! selects data line as SRP method
#define  otg_select_data_srp_method()        (Clr_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_SRPSEL))
#define  Is_otg_data_srp_method_selected()   (!Is_otg_vbus_srp_method_selected())
  //! tests if a HNP occurs
#define  Is_otg_hnp()                        (Tst_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_HNPREQ))
  //! tests if a SRP from device occurs
#define  Is_otg_device_srp()                 (Tst_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_SRPREQ))

  //! enables HNP error interrupt
#define  otg_enable_hnp_error_interrupt()       (Set_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_HNPERRE))
  //! disables HNP error interrupt
#define  otg_disable_hnp_error_interrupt()      (Clr_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_HNPERRE))
#define  Is_otg_hnp_error_interrupt_enabled()   (Tst_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_HNPERRE))
  //! acks HNP error interrupt
#define  otg_ack_hnp_error_interrupt()          (UOTGHS->UOTGHS_SCR = UOTGHS_SCR_HNPERRIC)
  //! raises HNP error interrupt
#define  otg_raise_hnp_error_interrupt()        (UOTGHS->UOTGHS_SFR = UOTGHS_SFR_HNPERRIS)
  //! tests if a HNP error occurs
#define  Is_otg_hnp_error_interrupt()           (Tst_bits(UOTGHS->UOTGHS_SR, UOTGHS_SR_HNPERRI))

  //! enables role exchange interrupt
#define  otg_enable_role_exchange_interrupt()      (Set_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_ROLEEXE))
  //! disables role exchange interrupt
#define  otg_disable_role_exchange_interrupt()     (Clr_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_ROLEEXE))
#define  Is_otg_role_exchange_interrupt_enabled()  (Tst_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_ROLEEXE))
  //! acks role exchange interrupt
#define  otg_ack_role_exchange_interrupt()         (UOTGHS->UOTGHS_SCR = UOTGHS_SCR_ROLEEXIC)
  //! raises role exchange interrupt
#define  otg_raise_role_exchange_interrupt()       (UOTGHS->UOTGHS_SFR = UOTGHS_SFR_ROLEEXIS)
  //! tests if a role exchange occurs
#define  Is_otg_role_exchange_interrupt()          (Tst_bits(UOTGHS->UOTGHS_SR, UOTGHS_SR_ROLEEXI))

  //! enables SRP interrupt
#define  otg_enable_srp_interrupt()          (Set_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_SRPE))
  //! disables SRP interrupt
#define  otg_disable_srp_interrupt()         (Clr_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_SRPE))
#define  Is_otg_srp_interrupt_enabled()      (Tst_bits(UOTGHS->UOTGHS_CTRL, UOTGHS_CTRL_SRPE))
  //! acks SRP interrupt
#define  otg_ack_srp_interrupt()             (UOTGHS->UOTGHS_SCR = UOTGHS_SCR_SRPIC)
  //! raises SRP interrupt
#define  otg_raise_srp_interrupt()           (UOTGHS->UOTGHS_SFR = UOTGHS_SFR_SRPIS)
  //! tests if a SRP occurs
#define  Is_otg_srp_interrupt()              (Tst_bits(UOTGHS->UOTGHS_SR, UOTGHS_SR_SRPI))
//! @}

//! @}

#endif // _USBB_OTG_H_
