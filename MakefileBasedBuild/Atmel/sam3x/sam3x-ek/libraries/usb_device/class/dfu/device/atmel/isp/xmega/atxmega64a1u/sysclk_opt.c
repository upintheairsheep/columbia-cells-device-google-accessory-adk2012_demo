/**
 * \file
 *
 * \brief Chip-specific system clock management functions
 *
 * Copyright (C) 2010 Atmel Corporation. All rights reserved.
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

#include <compiler.h>

#include <sysclk.h>
#include <osc.h>
#include <pll.h>
#if (CONFIG_OSC_RC32_CAL==48000000UL)
#include <nvm.h>
#endif

void sysclk_init(void)
{
	sysclk_set_prescalers(CONFIG_SYSCLK_PSADIV,CONFIG_SYSCLK_PSBCDIV);
		//* Replace osc_user_calibration(OSC_ID_RC32MHZ,cal);
		DFLLRC32M.CALA=0x40;
		DFLLRC32M.CALB=0x23;
	osc_enable(OSC_ID_RC32MHZ);
	osc_wait_ready(OSC_ID_RC32MHZ);
	ccp_write_io((uint8_t *)&CLK.CTRL, CONFIG_SYSCLK_SOURCE);
		//* Replace osc_enable_autocalibration(CONFIG_OSC_AUTOCAL,CONFIG_OSC_AUTOCAL_REF_OSC);
		OSC.DFLLCTRL &= ~(OSC_RC32MCREF_gm);
		// Calibrate 32MRC at 48MHz using USB SOF
		// 48MHz/1kHz=0xBB80
		DFLLRC32M.COMP1=0x80;
		DFLLRC32M.COMP2=0xBB;
		OSC.DFLLCTRL |= OSC_RC32MCREF_USBSOF_gc;
		DFLLRC32M.CTRL |= DFLL_ENABLE_bm;
}


/**
 * \brief Enable clock for the USB module
 *
 * \pre CONFIG_USBCLK_SOURCE must be defined.
 *
 * \param freq The required USB clock frequency in MHz:
 * \arg \c 6 for 6 MHz
 * \arg \c 48 for 48 MHz
 */
void sysclk_enable_usb(uint8_t freq)
{
	Assert(freq == 48);
	ccp_write_io((uint8_t *)&CLK.USBCTRL, 0
		| CLK_USBSRC_RCOSC_gc 
		| CLK_USBSEN_bm);
}

/**
 * \brief Disable clock for the USB module
 */
void sysclk_disable_usb(void)
{
	ccp_write_io((uint8_t *)&CLK.USBCTRL, 0);
}
