/**
 * \file
 *
 * \brief USB Device drivers
 * Compliance with common driver UDD
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

#include "board.h"
#include "conf_usb.h"
//#include "sysclk.h"
#include "udd.h"
#include "usbb_otg.h"
#include "usbb_device.h"
#include <string.h>

#ifndef UDD_NO_SLEEP_MGR
//#include "sleepmgr.h"
#endif

#ifndef UDD_USB_INT_LEVEL
#  define UDD_USB_INT_LEVEL 0   // By default USB interrupt have low priority
#endif

#define UDD_EP_DMA_SUPPORTED // Not used now (always support DMA)
//#define UDD_EP_FIFO_SUPPORTED

/**
 * \ingroup usb_device_group
 * \defgroup udd_group USB Device Driver (UDD)
 *  
 * \section USBB_CONF USBB Custom configuration
 * The following USBB driver configuration must be included in the conf_usb.h
 * file of the application.
 *
 * UDD_USB_INT_LEVEL<br>
 * Option to change the interrupt priority (0 to 3) by default 0 (recommended).
 * 
 * UDD_ISOCHRONOUS_NB_BANK<br>
 * Feature to reduce or increase isochronous endpoints buffering (1 to 2).
 * Default value 2.
 * 
 * UDD_BULK_NB_BANK<br>
 * Feature to reduce or increase bulk endpoints buffering (1 to 2).
 * Default value 2.
 * 
 * UDD_INTERRUPT_NB_BANK<br>
 * Feature to reduce or increase interrupt endpoints buffering (1 to 2).
 * Default value 1.
 *  
 * \section Callbacks management
 * The USB driver is fully managed by interrupt and does not request periodique
 * task. Thereby, the USB events use callbacks to transfer the information.
 * The callbacks are declared in static during compilation or in variable during
 * code execution.
 *  
 * Static declarations defined in conf_usb.h:
 * - UDC_VBUS_EVENT(bool b_present)<br>
 *   To signal Vbus level change
 * - UDC_SUSPEND_EVENT()<br>
 *   Called when USB bus enter in suspend mode
 * - UDC_RESUME_EVENT()<br>
 *   Called when USB bus is wakeup
 * - UDC_SOF_EVENT()<br>
 *   Called for each received SOF, Note: Each 1ms in HS/FS mode only.
 *   
 * Dynamic callbacks, called "endpoint job" , are registered
 * in udd_ep_job_t structure via the following functions:
 * - udd_ep_run()<br>
 *   To call it when a transfer is finish
 * - udd_ep_wait_stall_clear()<br>
 *   To call it when a endpoint halt is disabled
 *   
 * \section Power mode management
 * The Sleep modes authorized :
 * - in USB IDLE state, the USBB needs of USB clock and authorizes up to IDLE mode 
 * - in USB SUSPEND state, the USBB no needs USB clock but requests a minimum 
 *   clock restart timing. Thus, it authorizes up to STATIC or STANDBY mode.
 * - VBUS monitoring used in USB Self-Power mode authorizes up to STOP mode
 * 
 * The USBB_SLEEP_MODE_USB_IDLE equals SLEEPMGR_IDLE.
 *
 * The USBB_SLEEP_MODE_USB_SUSPEND depends on USB Power mode, 
 * USB clock startup timing and USB Speed mode:
 * | Power Mode | Speed mode | Clock Startup | Sleep mode authorized |
 * | Self-Power | LS, FS, HS | X             | SLEEPMGR_STOP        |
 * | Bus-Power  | LS, FS     | >10ms         | SLEEPMGR_STDBY       |
 * | Bus-Power  | LS, FS     | <=10ms        | SLEEPMGR_STATIC      |
 * | Bus-Power  | HS         | >3ms          | SLEEPMGR_STDBY       |
 * | Bus-Power  | HS         | <=3ms         | SLEEPMGR_STATIC      |
 *
 * @{
 */


// Check USB Device configuration
#ifndef USB_DEVICE_EP_CTRL_SIZE
#  error USB_DEVICE_EP_CTRL_SIZE not defined
#endif
#ifndef USB_DEVICE_MAX_EP
#  error USB_DEVICE_MAX_EP not defined
#endif
//#if (UC3A0 || UC3A1 || UC3B)
//#  ifdef USB_DEVICE_HS_SUPPORT
//#  error The High speed mode is not supported on this part, please remove USB_DEVICE_HS_SUPPORT in conf_usb.h
//#  endif
//#endif

#ifndef UDD_ISOCHRONOUS_NB_BANK
  #define UDD_ISOCHRONOUS_NB_BANK   2
#else
  #if (UDD_ISOCHRONOUS_NB_BANK<1) || (UDD_ISOCHRONOUS_NB_BANK>2)
    #error UDD_ISOCHRONOUS_NB_BANK must be define with 1 or 2.
  #endif
#endif
#ifndef UDD_BULK_NB_BANK
  #define UDD_BULK_NB_BANK  2
#else
  #if (UDD_BULK_NB_BANK<1) || (UDD_BULK_NB_BANK>2)
    #error UDD_BULK_NB_BANK must be define with 1 or 2.
  #endif
#endif
#ifndef UDD_INTERRUPT_NB_BANK
  #define UDD_INTERRUPT_NB_BANK 1
#else
  #if (UDD_INTERRUPT_NB_BANK<1) || (UDD_INTERRUPT_NB_BANK>2)
    #error UDD_INTERRUPT_NB_BANK must be define with 1 or 2.
  #endif
#endif


/**
 * \name Power management routine.
 */
//@{

#ifndef UDD_NO_SLEEP_MGR

//! Definition of sleep levels
#if (USB_DEVICE_ATTR & USB_CONFIG_ATTR_SELF_POWERED)
#  define USBB_SLEEP_MODE_USB_SUSPEND  SLEEPMGR_STOP
#else
#  if ((defined USB_DEVICE_HS_SUPPORT) && (USBCLK_STARTUP_TIMEOUT>3000)) \
     || ((!defined USB_DEVICE_HS_SUPPORT) && (USBCLK_STARTUP_TIMEOUT>10000))
#    define USBB_SLEEP_MODE_USB_SUSPEND  SLEEPMGR_STDBY
#  else
#    define USBB_SLEEP_MODE_USB_SUSPEND  SLEEPMGR_STATIC
#  endif
#endif
#define USBB_SLEEP_MODE_USB_IDLE  SLEEPMGR_IDLE

//! State of USB line
static bool udd_b_idle;


/*! \brief Authorize or not the CPU powerdown mode
 * 
 * \param b_enable   true to authorize powerdown mode
 */
static void udd_sleep_mode(bool b_idle)
{
    if (!b_idle && udd_b_idle) {
        sleepmgr_lock_mode(USBB_SLEEP_MODE_USB_IDLE);
    }
    if (b_idle && !udd_b_idle) {
        sleepmgr_unlock_mode(USBB_SLEEP_MODE_USB_IDLE);
    }
    udd_b_idle = b_idle;
}
#else

static void udd_sleep_mode(bool b_idle) {
    b_idle = b_idle;
}

#endif  // UDD_NO_SLEEP_MGR

//@}


/**
 * \name Control endpoint low level management routine.
 *
 * This function performs control endpoint mangement.
 * It handle the SETUP/DATA/HANDSHAKE phases of a control transaction.
 */
//@{

//! Global variable to give and record information about setup request management
COMPILER_WORD_ALIGNED udd_ctrl_request_t udd_g_ctrlreq;

//! Bit definitions about endpoint control state machine for udd_ep_control_state
typedef enum {
    UDD_EPCTRL_SETUP = 0,   //!< Wait a SETUP packet
    UDD_EPCTRL_DATA_OUT = 1,    //!< Wait a OUT data packet
    UDD_EPCTRL_DATA_IN = 2, //!< Wait a IN data packet
    UDD_EPCTRL_HANDSHAKE_WAIT_IN_ZLP = 3,   //!< Wait a IN ZLP packet
    UDD_EPCTRL_HANDSHAKE_WAIT_OUT_ZLP = 4,  //!< Wait a OUT ZLP packet
    UDD_EPCTRL_STALL_REQ = 5    //!< STALL enabled on IN & OUT packet
} udd_ctrl_ep_state_t;

//! State of the endpoint control management
static udd_ctrl_ep_state_t udd_ep_control_state;
//! Total number of data received/sent during data packet phase with previous payload buffers
static uint16_t udd_ctrl_prev_payload_nb_trans;
//! Number of data received/sent to/from udd_g_ctrlreq.payload buffer
static uint16_t udd_ctrl_payload_nb_trans;
//! Signal if the udd_g_ctrlreq.payload buffer is modulo endpoint control and need of data ZLP
static bool udd_ctrl_payload_need_in_zlp;

/**
 * \brief Reset control endpoint
 * 
 * Called after a USB line reset or when UDD is enabled
 */
static void udd_reset_ep_ctrl(void);

/**
 * \brief Reset control endpoint management
 * 
 * Called after a USB line reset or at the end of SETUP request (after ZLP)
 */
static void udd_ctrl_init(void);

//! \brief Managed reception of SETUP packet on control enpoint
static void udd_ctrl_setup_received(void);

//! \brief Managed reception of IN packet on control enpoint
static void udd_ctrl_in_sent(void);

//! \brief Managed reception of OUT packet on control enpoint
static void udd_ctrl_out_received(void);

//! \brief Managed underflow event of IN packet on control enpoint
static void udd_ctrl_underflow(void);

//! \brief Managed overflow event of OUT packet on control enpoint
static void udd_ctrl_overflow(void);

//! \brief Managed stall event of IN/OUT packet on control enpoint
static void udd_ctrl_stall_data(void);

//! \brief Send a ZLP IN on control endpoint
static void udd_ctrl_send_zlp_in(void);

//! \brief Send a ZLP OUT on control endpoint
static void udd_ctrl_send_zlp_out(void);

//! \brief Call callback associated to setup request
static void udd_ctrl_endofrequest(void);

/**
 * \brief Main interrupt routine for control endpoint
 *
 * This switchs control endpoint events to correct sub function.
 *
 * \return \c 1 if an event about control endpoint is occured, otherwise \c 0.
 */
static bool udd_ctrl_interrupt(void);

//@}


/**
 * \name Management of bulk/interrupt/isochronous endpoints
 *
 * The UDD manages the data transfer on endpoints:
 * - Start data tranfer on endpoint with USB Device DMA
 * - Send a ZLP packet if requested
 * - Call callback registered to signal end of transfer
 * The transfer abort and stall feature are supported.
 */
//@{
#if (0!=USB_DEVICE_MAX_EP)

//! Structure definition about job registered on an endpoint
typedef struct {
    uint8_t busy:1; //!< A job is registered on this endpoint
    uint8_t stall_requested:1;  //!< A stall has been requested but not executed
    uint8_t need_zlp:1; //!< ZLP is need to finish this IN transfer
    uint8_t *buf;   //!< Buffer located in internal RAM to send or fill during job
    iram_size_t buf_size;   //!< Size of buffer to send or fill
    iram_size_t buf_cnt; //!< Size of data transfered
    union {
        udd_callback_trans_t call_trans;    //!< Callback to call at the end of transfer
        udd_callback_halt_cleared_t call_nohalt;    //!< Callback to call when the endpoint halt is cleared
    };
} udd_ep_job_t;


//! Array to register a job on bulk/interrupt/isochronous endpoint
static udd_ep_job_t udd_ep_job[USB_DEVICE_MAX_EP];

//! \brief Reset all job table
static void udd_ep_job_table_reset(void);

//! \brief Abort all endpoint jobs on going
static void udd_ep_job_table_kill(void);

#ifdef UDD_EP_FIFO_SUPPORTED
//! \brief Kill last data in bank
//! \return Number of busy banks
static uint32_t udd_ep_kill_last(udd_ep_id_t ep);

//! \brief Fill banks and send them
static void udd_ep_in_sent(udd_ep_id_t ep);

//! \brief Store received banks
static void udd_ep_out_received(udd_ep_id_t ep);
#endif

/**
 * \brief Abort endpoint job on going
 *
 * \param ep         endpoint number of job to abort
 */
static void udd_ep_abort_job(udd_ep_id_t ep);

/**
 * \brief Call the callback associated to the job which is finished
 *
 * \param ptr_job  job to complete
 * \param b_abort  if true then the job has been aborted
 */
static void udd_ep_finish_job(udd_ep_job_t * ptr_job, bool b_abort);

/**
 * \brief Main interrupt routine for bulk/interrupt/isochronous endpoints
 *
 * This switchs endpoint events to correct sub function.
 *
 * \return \c 1 if an event about bulk/interrupt/isochronous endpoints has occured, otherwise \c 0.
 */
static bool udd_ep_interrupt(void);

#endif // (0!=USB_DEVICE_MAX_EP)
//@}


//--------------------------------------------------------
//--- INTERNAL ROUTINES TO MANAGED GLOBAL EVENTS

#define udd_interrupt UOTGHS_IrqHandler
/**
 * \internal
 * \brief Function called by USBB interrupt to manage USB Device interrupts
 *
 * USB Device interrupt events are splited in three parts:
 * - USB line events (SOF, reset, suspend, resume, wakeup)
 * - control endpoint events (setup reception, end of data transfer, underflow, overflow, stall)
 * - bulk/interrupt/isochronous endpoints events (end of data transfer)
 *
 * Note:
 * Here, the global interrupt mask is not clear when an USB interrupt is enabled
 * because this one can not be occured during the USB ISR (=during INTX is masked).
 * See Technical reference $3.8.3 Masking interrupt requests in peripheral modules.
 */
#ifdef OTG
static void udd_interrupt(void)
#else
// Fix the fact that, for some IAR header files, the AVR32_USBB_IRQ_GROUP define 
// has been defined as AVR32_USB_IRQ_GROUP instead.
#ifdef __ICCAVR32__
#if __ICCAVR32__
#if !defined(AVR32_USBB_IRQ_GROUP)
#define AVR32_USBB_IRQ_GROUP AVR32_USB_IRQ_GROUP
#endif
#endif
#endif
ISR(udd_interrupt, AVR32_USBB_IRQ_GROUP, UDD_USB_INT_LEVEL)
#endif
{
#ifdef UDC_SOF_EVENT
    if (Is_udd_sof()) {
        udd_ack_sof();
        UDC_SOF_EVENT();
        goto udd_interrupt_end;
    }
#endif

    if (udd_ctrl_interrupt())
        goto udd_interrupt_end; // Interrupt acked by control endpoint managed

#if (0!=USB_DEVICE_MAX_EP)
    if (udd_ep_interrupt())
        goto udd_interrupt_end; // Interrupt acked by bulk/interrupt/isochronous endpoint managed
#endif

    // USB bus reset detection
    if (Is_udd_reset()) {
        udd_ack_reset();
        // Abort all jobs on-going
#if (0!=USB_DEVICE_MAX_EP)
        udd_ep_job_table_kill();
#endif
        // Reset USB Device Stack Core
        udc_reset();
        // Reset endpoint control
        udd_reset_ep_ctrl();
        // Reset endpoint control management
        udd_ctrl_init();
        goto udd_interrupt_end;
    }

    if (Is_udd_suspend_interrupt_enabled() && Is_udd_suspend()) {
        otg_unfreeze_clock();
        // The suspend interrupt is automatic acked when a wakeup occur
        udd_disable_suspend_interrupt();
        udd_enable_wake_up_interrupt();
        otg_freeze_clock(); // Mandatory to exit of sleep mode after a wakeup event
        udd_sleep_mode(false);  // Enter in SUSPEND mode
#ifdef UDC_SUSPEND_EVENT
        UDC_SUSPEND_EVENT();
#endif
        goto udd_interrupt_end;
    }

    if (Is_udd_wake_up_interrupt_enabled() && Is_udd_wake_up()) {
        // Ack wakeup interrupt and enable suspend interrupt
        otg_unfreeze_clock();
        // Check USB clock ready after suspend and eventually sleep USB clock
        while( !Is_clock_usable() ) {
            if(Is_udd_suspend()) break;   // In case of USB state change in HS
        };
        // The wakeup interrupt is automatic acked when a suspend occur
        udd_disable_wake_up_interrupt();
        udd_enable_suspend_interrupt();
        udd_sleep_mode(true);   // Enter in IDLE mode
#ifdef UDC_RESUME_EVENT
        UDC_RESUME_EVENT();
#endif
        goto udd_interrupt_end;
    }

    if (Is_udd_vbus_transition()) {
        // Ack VBus transition and send status to high level
        otg_unfreeze_clock();
        udd_ack_vbus_transition();
        otg_freeze_clock();
#ifdef UDC_VBUS_EVENT
        UDC_VBUS_EVENT(Is_udd_vbus_high());
#endif
        goto udd_interrupt_end;
    }
udd_interrupt_end:
    otg_data_memory_barrier();
    return;
}


bool udd_include_vbus_monitoring(void)
{
    return false; // Since VBus detection has pb.
    //return true;
}


void udd_enable(void)
{
    irqflags_t flags;
    sysclk_enable_usb();

    flags = cpu_irq_save();

    //** Enable USB hardware
    otg_disable();
    (void)Is_otg_enabled();
#ifdef OTG
    // Check UID pin state before enter in USB device mode
    if (!Is_otg_id_device())
        return FALSE;
#else
    // Here, only the Device mode is possible, then link USBB interrupt to UDD interrupt
    //irq_register_handler(udd_interrupt, AVR32_USBB_IRQ, UDD_USB_INT_LEVEL);
    irq_register_handler(udd_interrupt, ID_UOTGHS, UDD_USB_INT_LEVEL);
    
    otg_force_device_mode();
#endif
    otg_disable_pad();
    otg_enable_pad();
    otg_enable();
    otg_unfreeze_clock();
    (void)Is_otg_clock_frozen();
//#if UC3A3
    // For parts with high speed feature, the "USABLE" clock is the UTMI clock,
    // and the UTMI clock is disabled in suspend mode. Thereby, the utmi clock
    // can't be checked when USB line is not attached or in suspend mode 
//#else
    // Check USB clock
    while( !Is_clock_usable() );
//#endif

    // Reset internal variables
#if (0!=USB_DEVICE_MAX_EP)
    udd_ep_job_table_reset();
#endif

    // Set the USB speed requested by configuration file
#ifdef USB_DEVICE_LOW_SPEED
    udd_low_speed_enable();
#else
    udd_low_speed_disable();
#  ifdef USB_DEVICE_HS_SUPPORT
    udd_high_speed_enable();
#  else
    udd_high_speed_disable();
#  endif
#endif
    udd_enable_vbus_interrupt();
    otg_freeze_clock();
    // Always authorize asynchrone USB interrupts to exit of sleep mode
    //??AVR32_PM.AWEN.usb_waken = 1;

#ifndef UDD_NO_SLEEP_MGR
    // Initialize the sleep mode authorized for the USB suspend mode
    udd_b_idle = false;
    sleepmgr_lock_mode(USBB_SLEEP_MODE_USB_SUSPEND);
#endif

    cpu_irq_restore(flags);
}


void udd_disable(void)
{
    irqflags_t flags;
    flags = cpu_irq_save();
    // Disable USB pad
    otg_disable();
    otg_disable_pad();
    sysclk_disable_usb();
    udd_sleep_mode(false);
#ifndef UDD_NO_SLEEP_MGR
    sleepmgr_unlock_mode(USBB_SLEEP_MODE_USB_SUSPEND);
#endif
    cpu_irq_restore(flags);
}


void udd_attach(void)
{
    irqflags_t flags;
    flags = cpu_irq_save();

    // At startup the USB bus state is unknown, 
    // therefore the state is considered IDLE to not miss any USB event
    udd_sleep_mode(true);
    otg_unfreeze_clock();
    
    // This section of clock check can be improved with a chek of 
    // USB clock source via sysclk()
#if 0//UC3A3
    // For parts with high speed feature, the "USABLE" clock is the UTMI clock,
    // and the UTMI clock is disabled in suspend mode. Thereby, the utmi clock
    // can't be checked when USB line is not attached or in suspend mode 
    // But it is not a issue, because the clock source is the OSC
#else
    // Check USB clock because the source can be a PLL
    while( !Is_clock_usable() );
#endif
    // Authorize attach if VBus is present
    udd_attach_device();

    // (RESET_AND_WAKEUP)
    // After the attach and the first USB suspend, the following USB Reset time can be inferior to CPU restart clock time.
    // Thus, the USB Reset state is not detected and endpoint control is not allocated
    // In this case, a Reset is do automatically after attach.
    udc_reset();    // Reset USB Device Stack Core
    udd_reset_ep_ctrl();    // Reset endpoint control
    udd_ctrl_init();    // Reset endpoint control management

    // Enable USB line events
    udd_enable_reset_interrupt();
    udd_enable_suspend_interrupt();
    udd_enable_wake_up_interrupt();
#ifdef UDC_SOF_EVENT
    udd_enable_sof_interrupt();
#endif
    // Reset following interupts flag
    udd_ack_reset();
    udd_ack_sof();
   
   // The first suspend interrupt must be forced
#if 0//UC3A3
   // With UTMI, the first suspend is detected but must be cleared to reoccur interrupt
   udd_ack_suspend();
#else
   // The first suspend interrupt is not detected else raise it
   udd_raise_suspend();
#endif
    udd_ack_wake_up();
    otg_freeze_clock();
    cpu_irq_restore(flags);
}


void udd_detach(void)
{
    otg_unfreeze_clock();
    // Detach device from the bus
    udd_detach_device();
    udd_sleep_mode(false);
}


bool udd_is_high_speed(void)
{
#ifdef USB_DEVICE_HS_SUPPORT
    return !Is_udd_full_speed_mode();
#else
    return false;
#endif
}


void udd_set_address(uint8_t address)
{
    udd_disable_address();
    udd_configure_address(address);
    udd_enable_address();
}


uint8_t udd_getaddress(void)
{
    return udd_get_configured_address();
}


uint16_t udd_get_frame_number(void)
{
    return udd_frame_number();
}


void udd_send_wake_up(void)
{
#ifndef UDD_NO_SLEEP_MGR
    if (!udd_b_idle)
#endif
    {
        udd_sleep_mode(true);   // Enter in IDLE mode
        otg_unfreeze_clock();
        udd_initiate_remote_wake_up();
    }
}


void udd_set_setup_payload( uint8_t *payload,   uint16_t payload_size )
{
    udd_g_ctrlreq.payload = payload;
    udd_g_ctrlreq.payload_size = payload_size;
}


#if (0!=USB_DEVICE_MAX_EP)
bool udd_ep_alloc(udd_ep_id_t ep, uint8_t bmAttributes,
        uint16_t MaxEndpointSize)
{
    bool b_dir_in;
    uint16_t ep_allocated;
    uint8_t nb_bank, bank, i;

    b_dir_in = ep & USB_EP_DIR_IN;
    ep = ep & USB_EP_ADDR_MASK;

    if (ep > USB_DEVICE_MAX_EP)
        return false;
    if (Is_udd_endpoint_enabled(ep))
        return false;

    // Bank choise
    switch(bmAttributes&USB_EP_TYPE_MASK) {
    case USB_EP_TYPE_ISOCHRONOUS:
        nb_bank = UDD_ISOCHRONOUS_NB_BANK;
        break;
    case USB_EP_TYPE_INTERRUPT:
        nb_bank = UDD_INTERRUPT_NB_BANK;
        break;
    case USB_EP_TYPE_BULK:
        nb_bank = UDD_BULK_NB_BANK;
        break;
    default: 
        Assert(false);
        return false;
    }
    switch(nb_bank) {
    case 1:
        bank = UOTGHS_DEVEPTCFG_EPBK_1_BANK >> UOTGHS_DEVEPTCFG_EPBK_Pos;
        break;
    case 2:
        bank = UOTGHS_DEVEPTCFG_EPBK_2_BANK >> UOTGHS_DEVEPTCFG_EPBK_Pos;
        break;
    case 3:
        bank = UOTGHS_DEVEPTCFG_EPBK_3_BANK >> UOTGHS_DEVEPTCFG_EPBK_Pos;
        break;
    }

    // Check if endpoint size is 8,16,32,64,128,256,512 or 1023
    Assert(MaxEndpointSize < 1024);
    Assert((MaxEndpointSize == 1023) || !(MaxEndpointSize & (MaxEndpointSize - 1)));
    Assert(MaxEndpointSize >= 8);
           
    // Set configuration of new endpoint
    udd_configure_endpoint(ep, bmAttributes, (b_dir_in ? 1 : 0),
            MaxEndpointSize, bank);
    ep_allocated = 1 << ep;

    // Unalloc endpoints superior
    for (i = USB_DEVICE_MAX_EP; i > ep; i--) {
        if (Is_udd_endpoint_enabled(i)) {
            ep_allocated |= 1 << i;
            udd_disable_endpoint(i);
            udd_unallocate_memory(i);
        }
    }

    // Realloc/Enable endpoints
    for (i = ep; i <= USB_DEVICE_MAX_EP; i++) {
        if (ep_allocated & (1 << i)) {
            udd_allocate_memory(i);
            udd_enable_endpoint(i);
            if (!Is_udd_endpoint_configured(i))
                return false;
        }
    }
    return true;
}


void udd_ep_free(udd_ep_id_t ep)
{
    udd_disable_endpoint(ep & USB_EP_ADDR_MASK);
    udd_unallocate_memory(ep & USB_EP_ADDR_MASK);
    udd_ep_abort_job(ep);
}


bool udd_ep_is_halted(udd_ep_id_t ep)
{
    return Is_udd_endpoint_stall_requested(ep & USB_EP_ADDR_MASK);
}


bool udd_ep_set_halt(udd_ep_id_t ep)
{
    uint8_t index = ep & USB_EP_ADDR_MASK;

    if (USB_DEVICE_MAX_EP < index)
        return false;
    if (Is_udd_bank_interrupt_enabled(index)) {
        // Wait end of transfer (= no busy bank) before stall endpoint
        udd_ep_job[index - 1].stall_requested = true;
    } else {
        // Stall endpoint
        udd_enable_stall_handshake(index);
        udd_reset_data_toggle(index);
    }
    udd_ep_abort_job(ep);
    return true;
}


bool udd_ep_clear_halt(udd_ep_id_t ep)
{
    udd_ep_job_t *ptr_job;

    ep &= USB_EP_ADDR_MASK;
    if (USB_DEVICE_MAX_EP < ep)
        return false;
    ptr_job = &udd_ep_job[ep - 1];

    if (Is_udd_endpoint_stall_requested(ep) // Endpoint stalled
            || ptr_job->stall_requested) {  // Endpoint stall is requested
        // Remove request to stall
        ptr_job->stall_requested = false;
        // Remove stall
        udd_disable_stall_handshake(ep);
        // If a job is register on clear halt action
        // then execute callback
        if (ptr_job->busy == true) {
            ptr_job->busy = false;
            ptr_job->call_nohalt();
        }
    }
    return true;
}


bool udd_ep_run(udd_ep_id_t ep, bool b_shortpacket,
        uint8_t * buf, iram_size_t buf_size,
        udd_callback_trans_t callback)
{
    bool b_dir_in;
    uint32_t udd_dma_ctrl = 0;
    udd_ep_job_t *ptr_job;
    irqflags_t flags;

    b_dir_in = (USB_EP_DIR_IN == (ep & USB_EP_DIR_IN));
    ep &= USB_EP_ADDR_MASK;
    if (USB_DEVICE_MAX_EP < ep)
        return false;

    // Get job about endpoint
    ptr_job = &udd_ep_job[ep - 1];

    if ((!Is_udd_endpoint_enabled(ep))
            || Is_udd_endpoint_stall_requested(ep)
            || ptr_job->stall_requested)
        return false;   // Endpoint is halted

    flags = cpu_irq_save();
    if (ptr_job->busy == true) {
        cpu_irq_restore(flags);
        return false;   // Job already on going
    }
    ptr_job->busy = true;
    cpu_irq_restore(flags);

    // The USBB supports a maximum transfer size of 64KB
    if (0x10000 <= buf_size) {
        // Transfer size = 64KB
        ptr_job->buf_size = 0x10000;
        buf_size = 0;
    } else {
        ptr_job->buf_size = buf_size;
        if (b_dir_in && (0 != buf_size % udd_get_endpoint_size(ep))) {
            // Force short packet option to send a shortpacket on IN,
            // else the DMA transfer is accepted and interrupt DMA valid but nothing is sent.
            b_shortpacket = true;
        }
    }
    ptr_job->buf = buf;
    ptr_job->call_trans = callback;

  #ifdef UDD_EP_FIFO_SUPPORTED
    // No DMA support
    if (!Is_udd_endpoint_dma_supported(ep)) {
        //printf("fifoEP%d.%c\n\r", ep, b_dir_in ? 'i':'o');
        ptr_job->buf_cnt = 0;
        //ptr_job->need_zlp = b_shortpacket;
        if (b_dir_in && buf_size == 0 && buf == NULL)
            ptr_job->need_zlp = true; // Only accept a forced ZLP
        flags = cpu_irq_save();
        udd_enable_endpoint_interrupt(ep);
        if (b_dir_in) {
            udd_ep_in_sent(ep);
            udd_enable_in_send_interrupt(ep);
        }
        else {
            udd_enable_out_received_interrupt(ep);
        }
        cpu_irq_restore(flags);
        return true;
    }
  #endif

    // Start USB DMA to fill or read fifo of the selected endpoint
    udd_endpoint_dma_set_addr(ep, (U32) buf);
    if (b_shortpacket) {
        if (b_dir_in) {
            udd_dma_ctrl = UOTGHS_DEVDMACONTROL_END_B_EN;
        } else {
            udd_dma_ctrl = UOTGHS_DEVDMACONTROL_END_TR_IT
                    |
                    UOTGHS_DEVDMACONTROL_END_TR_EN;
        }
    }
    udd_dma_ctrl |= (buf_size <<
            UOTGHS_DEVDMACONTROL_BUFF_LENGTH_Pos)
            & UOTGHS_DEVDMACONTROL_BUFF_LENGTH_Msk;
    udd_dma_ctrl |= UOTGHS_DEVDMACONTROL_END_BUFFIT |
            UOTGHS_DEVDMACONTROL_CHANN_ENB;
    udd_enable_endpoint_bank_autoswitch(ep);
    udd_endpoint_dma_set_control(ep, udd_dma_ctrl);
    flags = cpu_irq_save();
    udd_enable_endpoint_dma_interrupt(ep);
    cpu_irq_restore(flags);

    return true;
}


void udd_ep_abort(udd_ep_id_t ep)
{
  #ifdef UDD_EP_FIFO_SUPPORTED
    if (!Is_udd_endpoint_dma_supported(ep)) {
        // Disable interrupts
        udd_disable_endpoint_interrupt(ep);
        udd_disable_out_received_interrupt(ep);
        udd_disable_in_send_interrupt(ep);
        // Kill all banks
        while(udd_ep_kill_last(ep) > 0);
    }
    else
  #endif
    {
        // Stop DMA transfer
        udd_endpoint_dma_set_control((ep & USB_EP_ADDR_MASK), 0);
    }
    udd_ep_abort_job(ep);
}


bool udd_ep_wait_stall_clear(udd_ep_id_t ep,
        udd_callback_halt_cleared_t callback)
{
    udd_ep_job_t *ptr_job;

    ep &= USB_EP_ADDR_MASK;
    if (USB_DEVICE_MAX_EP < ep)
        return false;
    ptr_job = &udd_ep_job[ep - 1];

    if (!Is_udd_endpoint_enabled(ep))
        return false;   // Endpoint not enabled

    // Wait clear halt endpoint
    if (ptr_job->busy == true)
        return false;   // Job already on going

    if (Is_udd_endpoint_stall_requested(ep)
            || ptr_job->stall_requested) {
        // Endpoint halted then registes the callback
        ptr_job->busy = true;
        ptr_job->call_nohalt = callback;
    } else {
        // Enpoint not halted then call directly callback
        callback();
    }
    return true;
}
#endif // (0!=USB_DEVICE_MAX_EP)


#ifdef USB_DEVICE_HS_SUPPORT

void udd_test_mode_j(void)
{
    udd_enable_hs_test_mode();
    udd_enable_hs_test_mode_j();
}


void udd_test_mode_k(void)
{
    udd_enable_hs_test_mode();
    udd_enable_hs_test_mode_k();
}


void udd_test_mode_se0_nak(void)
{
    udd_enable_hs_test_mode();
}


void udd_test_mode_packet(void)
{
    uint8_t i;
    uint8_t *ptr_dest;
    const uint8_t *ptr_src;
    irqflags_t flags;

    const uint8_t test_packet[] = {
        // 00000000 * 9
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        // 01010101 * 8
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        // 01110111 * 8
        0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE,
        // 0, {111111S * 15}, 111111
        0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                0xFF, 0xFF,
        // S, 111111S, {0111111S * 7}
        0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD,
        // 00111111, {S0111111 * 9}, S0
        0xFC, 0x7E, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0x7E
    };

    // Reconfigure control endpoint to bulk IN endpoint
    udd_disable_endpoint(0);
    udd_configure_endpoint(0, USB_EP_TYPE_BULK, 1,  // IN
            64, UOTGHS_DEVEPTCFG_EPBK_1_BANK);
    udd_allocate_memory(0);
    udd_enable_endpoint(0);

    udd_enable_hs_test_mode();
    udd_enable_hs_test_mode_packet();

    // Send packet on endpoint 0
    ptr_dest = (uint8_t *) & udd_get_endpoint_fifo_access(0, 8);
    ptr_src = test_packet;
    for (i = 0; i < sizeof(test_packet); i++) {
        *ptr_dest++ = *ptr_src++;
    }
    flags = cpu_irq_save();
    udd_enable_in_send_interrupt(0);
    cpu_irq_restore(flags);
    udd_ack_in_send(0);
}
#endif // USB_DEVICE_HS_SUPPORT



//--------------------------------------------------------
//--- INTERNAL ROUTINES TO MANAGED THE CONTROL ENDPOINT

static void udd_reset_ep_ctrl(void)
{
    irqflags_t flags;
    // Reset USB address to 0
    udd_configure_address(0);
    udd_enable_address();
    // Alloc and configure control endpoint
    //udd_configure_endpoint(0,
    //      USB_EP_TYPE_CONTROL,
    //      0,
    //      USB_DEVICE_EP_CTRL_SIZE, AVR32_USBB_UECFG0_EPBK_SINGLE);
    udd_configure_endpoint(0,
            USB_EP_TYPE_CONTROL,
            0,
            USB_DEVICE_EP_CTRL_SIZE, UOTGHS_DEVEPTCFG_EPBK_1_BANK);

    udd_allocate_memory(0);
    udd_enable_endpoint(0);
    flags = cpu_irq_save();
    udd_enable_setup_received_interrupt(0);
    udd_enable_out_received_interrupt(0);
    udd_enable_endpoint_interrupt(0);
    cpu_irq_restore(flags);
}

static void udd_ctrl_init(void)
{
    irqflags_t flags;
    flags = cpu_irq_save();
    // In case of abort of IN Data Phase:
    // No need to abort IN transfer (rise TXINI), 
    // because it is automatically done by hardware when a Setup packet is received.
    // But the interrupt must be disabled to don't generate interrupt TXINI 
    // after SETUP reception.
    udd_disable_in_send_interrupt(0);
    cpu_irq_restore(flags);
    // In case of OUT ZLP event is no processed before Setup event occurs
    udd_ack_out_received(0);

    udd_g_ctrlreq.callback = NULL;
    udd_g_ctrlreq.over_under_run = NULL;
    udd_g_ctrlreq.payload_size = 0;
    udd_ep_control_state = UDD_EPCTRL_SETUP;
}


static void udd_ctrl_setup_received(void)
{
    irqflags_t flags;
    uint8_t i;

    if (UDD_EPCTRL_SETUP != udd_ep_control_state) {
        // May be a hidden DATA or ZLP phase
        // or protocol abort
        udd_ctrl_endofrequest();
        // Reinitializes control endpoint management
        udd_ctrl_init();
    }
    // Fill setup request structure
    if (8 != udd_byte_count(0)) {
        udd_ctrl_stall_data();
        udd_ack_setup_received(0);
        return; // Error data number doesn't correspond to SETUP packet
    }
    uint32_t *ptr = (uint32_t *) & udd_get_endpoint_fifo_access(0, 32);
    for (i = 0; i < 8 / 4; i++) {
        ((uint32_t *) & udd_g_ctrlreq.req)[i] = *ptr++;
    }
    // Manage LSB/MSB to fit with CPU usage
    udd_g_ctrlreq.req.wValue = le16_to_cpu(udd_g_ctrlreq.req.wValue);
    udd_g_ctrlreq.req.wIndex = le16_to_cpu(udd_g_ctrlreq.req.wIndex);
    udd_g_ctrlreq.req.wLength = le16_to_cpu(udd_g_ctrlreq.req.wLength);

    // Decode setup request
    if (udc_process_setup() == false) {
        // Setup request unknow then stall it
        udd_ctrl_stall_data();
        udd_ack_setup_received(0);
        return;
    }
    udd_ack_setup_received(0);

    if (Udd_setup_is_in()) {
        // Compute if an IN ZLP must be send after IN data
        udd_ctrl_payload_need_in_zlp =
                ((udd_g_ctrlreq.payload_size %
                        USB_DEVICE_EP_CTRL_SIZE) == 0);
        // IN data phase requested
        udd_ctrl_prev_payload_nb_trans = 0;
        udd_ctrl_payload_nb_trans = 0;
        udd_ep_control_state = UDD_EPCTRL_DATA_IN;
        udd_ctrl_in_sent(); // Send first data transfer
    } else {
        if (0 == udd_g_ctrlreq.req.wLength) {
            // No data phase requested
            // Send IN ZLP to ACK setup request
            udd_ctrl_send_zlp_in();
            return;
        }
        // OUT data phase requested
        udd_ctrl_prev_payload_nb_trans = 0;
        udd_ctrl_payload_nb_trans = 0;
        udd_ep_control_state = UDD_EPCTRL_DATA_OUT;
        // To detect a protocol error, enable nak interrupt on data IN phase
        udd_ack_nak_in(0);
        flags = cpu_irq_save();
        udd_enable_nak_in_interrupt(0);
        cpu_irq_restore(flags);
    }
}


static void udd_ctrl_in_sent(void)
{
    uint16_t nb_remain;
    uint8_t i;
    uint8_t *ptr_dest, *ptr_src;
    irqflags_t flags;

    flags = cpu_irq_save();
    udd_disable_in_send_interrupt(0);
    cpu_irq_restore(flags);

    if (UDD_EPCTRL_HANDSHAKE_WAIT_IN_ZLP == udd_ep_control_state) {
        // ZLP on IN is sent, then valid end of setup request
        udd_ctrl_endofrequest();
        // Reinitializes control endpoint management
        udd_ctrl_init();
        return;
    }
    Assert(udd_ep_control_state == UDD_EPCTRL_DATA_IN);

    nb_remain = udd_g_ctrlreq.payload_size - udd_ctrl_payload_nb_trans;
    if (0 == nb_remain) {
        // All content of current buffer payload are sent
        if (!udd_ctrl_payload_need_in_zlp) {
            // It is the end of data phase, because the last data packet is a short packet
            // then generate an OUT ZLP for handshake phase.
            udd_ctrl_send_zlp_out();
            return;
        }
        if ((udd_g_ctrlreq.req.wLength > (udd_ctrl_prev_payload_nb_trans
                                +
                                udd_g_ctrlreq.
                                payload_size))
                || (!udd_g_ctrlreq.over_under_run)
                || (!udd_g_ctrlreq.over_under_run())) {
            // Underrun or data packet complette than send zlp on IN (note don't change DataToggle)
            udd_ctrl_payload_need_in_zlp = false;
            // nb_remain==0 allows to send a IN ZLP
        } else {
            // A new payload buffer is given
            // Update number of total data sending by previous playlaod buffer
            udd_ctrl_prev_payload_nb_trans +=
                    udd_ctrl_payload_nb_trans;
            // Update maangement of current playoad transfer
            udd_ctrl_payload_nb_trans = 0;
            nb_remain = udd_g_ctrlreq.payload_size;
            // Compute if an IN ZLP must be send after IN data
            udd_ctrl_payload_need_in_zlp =
                    ((udd_g_ctrlreq.payload_size %
                            USB_DEVICE_EP_CTRL_SIZE)
                    == 0);
        }
    }
    // Continue transfer and send next data
    if (nb_remain > USB_DEVICE_EP_CTRL_SIZE) {
        nb_remain = USB_DEVICE_EP_CTRL_SIZE;
    }
    // Fill buffer of endpoint control
    ptr_dest = (uint8_t *) & udd_get_endpoint_fifo_access(0, 8);
    ptr_src = udd_g_ctrlreq.payload + udd_ctrl_payload_nb_trans;
    //** Critical section
    // Only in case of DATA IN phase abort without USB Reset signal after.
    // The IN data don't must be writed in endpoint 0 DPRAM during
    // a next setup reception in same endpoint 0 DPRAM.
    // Thereby, an OUT ZLP reception must check before IN data write 
    // and if no OUT ZLP is recevied the data must be written quickly (800us)
    // before an eventually ZLP OUT and SETUP reception
    flags = cpu_irq_save();
    if (Is_udd_out_received(0)) {
        // IN DATA phase aborted by OUT ZLP
        cpu_irq_restore(flags);
        udd_ep_control_state = UDD_EPCTRL_HANDSHAKE_WAIT_OUT_ZLP;
        return; // Exit of IN DATA phase
    }
    // Write quickly the IN data
    for (i = 0; i < nb_remain; i++) {
        *ptr_dest++ = *ptr_src++;
    }
    udd_ctrl_payload_nb_trans += nb_remain;

    // Validate and send the data available in the control endpoint buffer
    udd_ack_in_send(0);
    udd_enable_in_send_interrupt(0);
    // In case of abort of DATA IN phase, no need to enable nak OUT interrupt
    // because OUT endpoint is already free and ZLP OUT accepted.
    cpu_irq_restore(flags);
}


static void udd_ctrl_out_received(void)
{
    irqflags_t flags;
    uint8_t i;
    uint16_t nb_data;

    if (UDD_EPCTRL_DATA_OUT != udd_ep_control_state) {
        if ((UDD_EPCTRL_DATA_IN == udd_ep_control_state)
                || (UDD_EPCTRL_HANDSHAKE_WAIT_OUT_ZLP ==
                        udd_ep_control_state)) {
            // End of SETUP request:
            // - Data IN Phase aborted,
            // - or last Data IN Phase hidden by ZLP OUT sending quiclky,
            // - or ZLP OUT received normaly.
            udd_ctrl_endofrequest();
        } else {
            // Protocol error during SETUP request
            udd_ctrl_stall_data();
        }
        // Reinitializes control endpoint management
        udd_ctrl_init();
        return;
    }
    // Read data received during OUT phase
    nb_data = udd_byte_count(0);
    if (udd_g_ctrlreq.payload_size < (udd_ctrl_payload_nb_trans + nb_data)) {
        // Payload buffer too small
        nb_data = udd_g_ctrlreq.payload_size -
                udd_ctrl_payload_nb_trans;
    }
    uint8_t *ptr_src = (uint8_t *) & udd_get_endpoint_fifo_access(0, 8);
    uint8_t *ptr_dest = udd_g_ctrlreq.payload + udd_ctrl_payload_nb_trans;
    for (i = 0; i < nb_data; i++) {
        *ptr_dest++ = *ptr_src++;
    }
    udd_ctrl_payload_nb_trans += nb_data;

    if ((USB_DEVICE_EP_CTRL_SIZE != nb_data)
            || (udd_g_ctrlreq.req.wLength <=
                    (udd_ctrl_prev_payload_nb_trans +
                            udd_ctrl_payload_nb_trans)))
    {
        // End of reception because it is a short packet
        // Before send ZLP, call intermediat calback 
        // in case of data receiv generate a stall
        udd_g_ctrlreq.payload_size = udd_ctrl_payload_nb_trans;
        if (NULL != udd_g_ctrlreq.over_under_run) {
            if (!udd_g_ctrlreq.over_under_run()) {
                // Stall ZLP
                udd_ctrl_stall_data();
                // Ack reception of OUT to replace NAK by a STALL
                udd_ack_out_received(0);
                return;
            }
        }
        // Send IN ZLP to ACK setup request
        udd_ack_out_received(0);
        udd_ctrl_send_zlp_in();
        return;
    }

    if (udd_g_ctrlreq.payload_size == udd_ctrl_payload_nb_trans) {
        // Overrun then request a new payload buffer
        if (!udd_g_ctrlreq.over_under_run) {
            // No callback availabled to request a new payload buffer
            udd_ctrl_stall_data();
            // Ack reception of OUT to replace NAK by a STALL
            udd_ack_out_received(0);
            return;
        }
        if (!udd_g_ctrlreq.over_under_run()) {
            // No new payload buffer delivered
            udd_ctrl_stall_data();
            // Ack reception of OUT to replace NAK by a STALL
            udd_ack_out_received(0);
            return;
        }
        // New payload buffer available
        // Update number of total data received
        udd_ctrl_prev_payload_nb_trans += udd_ctrl_payload_nb_trans;
        // Reinit reception on payload buffer
        udd_ctrl_payload_nb_trans = 0;
    }
    // Free buffer of control endpoint to authorize next reception
    udd_ack_out_received(0);
    // To detect a protocol error, enable nak interrupt on data IN phase
    udd_ack_nak_in(0);
    flags = cpu_irq_save();
    udd_enable_nak_in_interrupt(0);
    cpu_irq_restore(flags);
}


static void udd_ctrl_underflow(void)
{
    if (Is_udd_out_received(0))
        return; // underflow ignored if OUT data is received

    if (UDD_EPCTRL_DATA_OUT == udd_ep_control_state) {
        // Host want to stop OUT transaction
        // then stop to wait OUT data phase and wait IN ZLP handshake
        udd_ctrl_send_zlp_in();
    } else if (UDD_EPCTRL_HANDSHAKE_WAIT_OUT_ZLP == udd_ep_control_state) {
        // A OUT handshake is waiting by device,
        // but host want extra IN data then stall extra IN data
        udd_enable_stall_handshake(0);
    }
}


static void udd_ctrl_overflow(void)
{
    if (Is_udd_in_send(0))
        return; // overflow ignored if IN data is received

    // The case of UDD_EPCTRL_DATA_IN is not managed
    // because the OUT endpoint is already free and OUT ZLP accepted

    if (UDD_EPCTRL_HANDSHAKE_WAIT_IN_ZLP == udd_ep_control_state) {
        // A IN handshake is waiting by device,
        // but host want extra OUT data then stall extra OUT data
        udd_enable_stall_handshake(0);
    }
}


static void udd_ctrl_stall_data(void)
{
    // Stall all packets on IN & OUT control endpoint
    udd_ep_control_state = UDD_EPCTRL_STALL_REQ;
    udd_enable_stall_handshake(0);
}


static void udd_ctrl_send_zlp_in(void)
{
    irqflags_t flags;

    udd_ep_control_state = UDD_EPCTRL_HANDSHAKE_WAIT_IN_ZLP;
    // Validate and send empty IN packet on control endpoint
    flags = cpu_irq_save();
    // Send ZLP on IN endpoint
    udd_ack_in_send(0);
    udd_enable_in_send_interrupt(0);
    // To detect a protocol error, enable nak interrupt on data OUT phase
    udd_ack_nak_out(0);
    udd_enable_nak_out_interrupt(0);
    cpu_irq_restore(flags);
}


static void udd_ctrl_send_zlp_out(void)
{
    irqflags_t flags;

    udd_ep_control_state = UDD_EPCTRL_HANDSHAKE_WAIT_OUT_ZLP;
    // No action is necessary to accept OUT ZLP
    // because the buffer of control endpoint is already free

    // To detect a protocol error, enable nak interrupt on data IN phase
    flags = cpu_irq_save();
    udd_ack_nak_in(0);
    udd_enable_nak_in_interrupt(0);
    cpu_irq_restore(flags);
}


static void udd_ctrl_endofrequest(void)
{
    // If a callback is registered then call it
    if (udd_g_ctrlreq.callback) {
        udd_g_ctrlreq.callback();
    }
}


static bool udd_ctrl_interrupt(void)
{

    if (!Is_udd_endpoint_interrupt(0))
        return false;   // No interrupt events on control endpoint

    // By default disable overflow and underflow interrupt
    udd_disable_nak_in_interrupt(0);
    udd_disable_nak_out_interrupt(0);


    // Search event on control endpoint
    if (Is_udd_setup_received(0)) {
        // SETUP packet received
        udd_ctrl_setup_received();
        return true;
    }
    if (Is_udd_in_send(0) && Is_udd_in_send_interrupt_enabled(0)) {
        // IN packet sent
        udd_ctrl_in_sent();
        return true;
    }
    if (Is_udd_out_received(0)) {
        // OUT packet received
        udd_ctrl_out_received();
        return true;
    }
    if (Is_udd_nak_out(0)) {
        // Overflow on OUT packet
        udd_ack_nak_out(0);
        udd_ctrl_overflow();
        return true;
    }
    if (Is_udd_nak_in(0)) {
        // Underflow on IN packet
        udd_ack_nak_in(0);
        udd_ctrl_underflow();
        return true;
    }
    return false;
}


//--------------------------------------------------------
//--- INTERNAL ROUTINES TO MANAGED THE BULK/INTERRUPT/ISOCHRONOUS ENDPOINTS

#if (0!=USB_DEVICE_MAX_EP)

static void udd_ep_job_table_reset(void)
{
    uint8_t i;
    for (i = 0; i < USB_DEVICE_MAX_EP; i++) {
        udd_ep_job[i].busy = false;
        udd_ep_job[i].stall_requested = false;
    }
}


static void udd_ep_job_table_kill(void)
{
    uint8_t i;
    // For each endpoint, kill job
    for (i = 0; i < USB_DEVICE_MAX_EP; i++) {
        udd_ep_finish_job(&udd_ep_job[i], true);
    }
}


static void udd_ep_abort_job(udd_ep_id_t ep)
{
    ep &= USB_EP_ADDR_MASK;
    // Abort job on endpoint
    udd_ep_finish_job(&udd_ep_job[ep - 1], true);
}


static void udd_ep_finish_job(udd_ep_job_t * ptr_job, bool b_abort)
{
    if (ptr_job->busy == false)
        return; // No on-going job
    ptr_job->busy = false;
    if (NULL == ptr_job->call_trans)
        return; // No callback linked to job
    ptr_job->call_trans((b_abort) ? UDD_EP_TRANSFER_ABORT :
            UDD_EP_TRANSFER_OK, ptr_job->buf_size);
}

#ifdef UDD_EP_FIFO_SUPPORTED
static uint32_t udd_ep_kill_last(udd_ep_id_t ep)
{
    udd_kill_last_in_bank(ep);
    while(Is_udd_last_in_bank_killed(ep));
    return udd_nb_busy_bank(ep);
}

static void udd_ep_in_sent(udd_ep_id_t ep)
{
    udd_ep_job_t *ptr_job = &udd_ep_job[ep - 1];
    uint8_t *ptr_src = &ptr_job->buf[ptr_job->buf_cnt];
    uint8_t *ptr_dst = (uint8_t*) &udd_get_endpoint_fifo_access(ep, 8);
    uint8_t  nb_bank = udd_get_endpoint_bank(ep);
    uint32_t pkt_size = udd_get_endpoint_size(ep);
    uint32_t nb_data = 0, i;
    uint32_t nb_remain;
    //printf("i%d.%d ", ep, nb_bank);
    // All transfer done, including ZLP, Finish Job
    if (ptr_job->buf_cnt >= ptr_job->buf_size && !ptr_job->need_zlp) {
        udd_disable_in_send_interrupt(ep);
        udd_disable_endpoint_interrupt(ep);
        udd_ack_in_send(ep); // Clear IN Sent
        //printf("EoI%d\n\r", ep);
        ptr_job->buf_size = ptr_job->buf_cnt; // buf_size is passed to callback as XFR count
        udd_ep_finish_job(ptr_job, false);
        return;
    }
    // There is data to send
    if (udd_nb_busy_bank(ep)) {
        udd_ack_in_send(ep);
    }
    // Fill FIFOs
    while (true) {
        // All data sent
        if (ptr_job->buf_cnt >= ptr_job->buf_size && !ptr_job->need_zlp) {
            break;
        }
        ptr_dst = (uint8_t*) &udd_get_endpoint_fifo_access(ep, 8);
        ptr_src = &ptr_job->buf[ptr_job->buf_cnt];
        nb_remain = ptr_job->buf_size - ptr_job->buf_cnt;
        // Fill a bank even if no data (ZLP)
        nb_data = min(nb_remain, pkt_size);
        // Modify job information
        ptr_job->buf_cnt += nb_data;
        // Copy buffer to FIFO
        for (i = 0; i < nb_data; i ++) {
            *ptr_dst ++ = *ptr_src ++;
        }
        // Switch to next bank
        udd_ack_fifocon(ep);
        //printf("%d,%d ", nb_data, udd_nb_busy_bank(ep));
        // ZLP?
        if (nb_data < pkt_size)
            ptr_job->need_zlp = false;
        // All banks filled
        if (udd_nb_busy_bank(ep) >= nb_bank)
            break;
        // Short packet is processed!
        if (nb_data < pkt_size) {
            ptr_job->need_zlp = false;
            break;
        }
    }
}

static void udd_ep_out_received(udd_ep_id_t ep)
{
    udd_ep_job_t *ptr_job = &udd_ep_job[ep - 1];
    uint32_t nb_data = 0, i;
    uint32_t nb_remain = ptr_job->buf_size - ptr_job->buf_cnt;
    uint32_t pkt_size = udd_get_endpoint_size(ep);
    uint8_t *ptr_src = (uint8_t*) &udd_get_endpoint_fifo_access(ep, 8);
    uint8_t *ptr_dst = &ptr_job->buf[ptr_job->buf_cnt];
    bool b_full = false, b_short = false;
    // Clear RX OUT
    udd_ack_out_received(ep);
    // Read byte count
    nb_data = udd_byte_count(ep);
    if (nb_data < pkt_size)
        b_short = true;
    //printf("o%d ", ep);
    //printf("%d ", nb_data);
    // Copy data if there is
    if (nb_data > 0) {
        if (nb_data >= nb_remain) {
            nb_data  = nb_remain;
            b_full = true;
        }
        // Modify job information
        ptr_job->buf_cnt += nb_data;
        // Copy FIFO to buffer
        for (i = 0; i < nb_data; i ++) {
            *ptr_dst ++ = *ptr_src ++;
        }
    }
    // Clear FIFO Status
    udd_ack_fifocon(ep);
    // Finish job on error or short packet
    if (b_full || b_short) {
        //printf("EoO%d\n\r", ep);
        udd_disable_out_received_interrupt(ep);
        udd_disable_endpoint_interrupt(ep);
        ptr_job->buf_size = ptr_job->buf_cnt; // buf_size is passed to callback as XFR count
        udd_ep_finish_job(ptr_job, false);
    }
}
#endif // #ifdef UDD_EP_FIFO_SUPPORTED

static bool udd_ep_interrupt(void)
{
    udd_ep_id_t ep;
    udd_ep_job_t *ptr_job;

    // For each endpoint different of control endpoint (0)
    for (ep = 1; ep <= USB_DEVICE_MAX_EP; ep++) {
        // Check DMA event
        if (Is_udd_endpoint_dma_interrupt_enabled(ep)
                && Is_udd_endpoint_dma_interrupt(ep)) {
            uint32_t nb_remaining;
            udd_disable_endpoint_dma_interrupt(ep);
            // Save number of data no transfered
            nb_remaining = (udd_endpoint_dma_get_status(ep) &
                    UOTGHS_DEVDMASTATUS_BUFF_COUNT_Msk)
                    >>
                    UOTGHS_DEVDMASTATUS_BUFF_COUNT_Pos;
            // Get job corresponding at endpoint
            ptr_job = &udd_ep_job[ep - 1];
            // Update number of data transfered
            ptr_job->buf_size -= nb_remaining;

            if (!Is_udd_endpoint_in(ep)) {
                // Disable autoswitch bank on OUT
                udd_disable_endpoint_bank_autoswitch(ep);
            } else {
                // Wait end of background transfer on IN endpoint before disabled autoswitch bank
                udd_enable_endpoint_interrupt(ep);
                udd_enable_bank_interrupt(ep);
            }
            // Call callback to signal end of transfer
            udd_ep_finish_job(&udd_ep_job[ep - 1], false);
            return true;
        }
      #ifdef UDD_EP_FIFO_SUPPORTED
        // Check RXRDY and TXEMPTY event for none DMA endpoints
        if (!Is_udd_endpoint_dma_supported(ep)
            && Is_udd_endpoint_interrupt_enabled(ep)) {
            //printf("%x ", USBB_ARRAY(UOTGHS_DEVEPTISR[0],ep));
            // RXOUT: Full packet received
            if (Is_udd_out_received(ep)
                 && Is_udd_out_received_interrupt_enabled(ep)) {
                udd_ep_out_received(ep);
                return true;
            }
            // TXIN: packet sent
            if (Is_udd_in_send(ep)
                 && Is_udd_in_send_interrupt_enabled(ep)) {
                udd_ep_in_sent(ep);
                return true;
            }
            // Errors: Abort?
            if (Is_udd_overflow(ep) || Is_udd_underflow(ep) || Is_udd_crc_error(ep)) {
                udd_ep_abort(ep);
                return true;
            }
        }
      #endif
        // Check empty bank interrupt event
        if (Is_udd_endpoint_interrupt_enabled(ep)
                && Is_udd_endpoint_dma_supported(ep)
                && (0 == udd_nb_busy_bank(ep))) {
            // End of background transfer on IN endpoint
            udd_disable_bank_interrupt(ep);
            udd_disable_endpoint_interrupt(ep);
            // If no new transfer running then disable autoswitch bank
            if (!udd_ep_job[ep - 1].busy) {
                udd_disable_endpoint_bank_autoswitch(ep);
            }
            // If a stall has been requested during backgound transfer then execute it
            if (udd_ep_job[ep - 1].stall_requested) {
                udd_ep_job[ep - 1].stall_requested = false;
                udd_enable_stall_handshake(ep);
                udd_reset_data_toggle(ep);
            }
            return true;
        }
    }
    return false;
}
#endif // (0!=USB_DEVICE_MAX_EP)

//@}
