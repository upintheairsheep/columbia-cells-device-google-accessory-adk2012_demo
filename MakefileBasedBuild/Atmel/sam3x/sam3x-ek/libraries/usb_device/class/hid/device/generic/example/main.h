/*This file is prepared for Doxygen automatic documentation generation.*/
/*! \file ******************************************************************
 *
 * \brief Declaration of main function used by HID generic example
 *
 * - Compiler:           IAR EWAVR32 and GNU GCC for AVR32
 * - Supported devices:  All AVR32 devices with a USB module can be used.
 * - AppNote:
 *
 * \author               Atmel Corporation: http://www.atmel.com \n
 *                       Support and FAQ: http://support.atmel.no/
 *
 ***************************************************************************/

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

#ifndef _MAIN_H_
#define _MAIN_H_

/*! \brief Called by HID interface
 * Callback running when USB Host enable generic interface
 *
 * \retval true if generic startup is ok
 */
bool main_generic_enable(void);

/*! \brief Called by HID interface
 * Callback running when USB Host disable generic interface
 */
void main_generic_disable(void);

/*! \brief Called when Vbus line state change
 */
void main_vbus_action(bool b_high);

/*! \brief Called when a start of frame is received on USB line
 */
void main_sof_action(void);

/*! \brief Called by UDD when a suspend is received
 * Callback running when USB Host set USB line in suspend state
 */
void main_suspend_action(void);

/*! \brief Called by UDD when the USB line exit of suspend state
 */
void main_resume_action(void);

/*! \brief Called by UDC when USB Host request to enable remote wakeup
 */
void main_remotewakeup_enable(void);

/*! \brief Called by UDC when USB Host request to disable remote wakeup
 */
void main_remotewakeup_disable(void);

/*! \brief Reset target to jump in bootloader
 */
void main_generic_run_bootloader(void);

void main_report_out(uint8_t *data);

void main_set_feature(uint8_t *feature);


#endif // _MAIN_H_
