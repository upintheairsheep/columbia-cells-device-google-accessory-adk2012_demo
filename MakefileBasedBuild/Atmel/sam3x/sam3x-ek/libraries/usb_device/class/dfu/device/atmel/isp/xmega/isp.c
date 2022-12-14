/**
 * \file
 *
 * \brief In system programming to control security, memories and fuses
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

#include "conf_isp.h"
#include "nvm.h"
#include "isp.h"
#include "string.h"


#ifdef __GNUC__
	uint16_t start_app_key __attribute__ ((section (".noinit")));
#else
	__no_init uint16_t start_app_key @"SRAM_NO_INIT"; 
#endif

/**
 * \name Memory APIs
 */
//@{

//! Memory signature which store information about device
static isp_mem_signature_t mem_signature;

//! Memory bootloader which store information about bootloader
static isp_mem_bootloader_t mem_bootloader = {
	.version = BOOTLOADER_VERSION,
	.id1 = 0,
	.id2 = 0,
};


void mem_flash_read(void *dst, isp_addr_t src, uint16_t nbytes)
{
	nvm_flash_read_buffer(src, dst, nbytes);
}

void mem_flash_write(isp_addr_t dst, const void *src, uint16_t nbytes)
{
	nvm_flash_erase_and_write_buffer(dst, src, nbytes, true);
}

void mem_eeprom_read(void *dst, isp_addr_t src, uint16_t nbytes)
{
	nvm_eeprom_read_buffer( src, dst, nbytes );
}

void mem_eeprom_write(isp_addr_t dst, const void *src, uint16_t nbytes)
{
	nvm_eeprom_erase_and_write_buffer(dst, src, nbytes);
}

static void mem_bootloader_read(void *dst, isp_addr_t src, uint16_t nbytes)
{
	memcpy(dst, (uint8_t*)&mem_bootloader + src, nbytes);
}

void mem_signature_read(void *dst, isp_addr_t src, uint16_t nbytes)
{
	memcpy(dst, (uint8_t*)&mem_signature + src, nbytes);
}

//! Interface for memory flash
const isp_mem_t isp_flash = {
	.size        = FLASH_SIZE,
	.fnct_read   = mem_flash_read,
	.fnct_write  = mem_flash_write,
};

//! Interface for memory eeprom
const isp_mem_t isp_eeprom = {
	.size        = EEPROM_SIZE,
	.fnct_read   = mem_eeprom_read,
	.fnct_write  = mem_eeprom_write,
};

//! Interface for memory bootloader
const isp_mem_t isp_bootloader = {
	.size        = sizeof(mem_bootloader),
	.fnct_read   = mem_bootloader_read,
	.fnct_write  = NULL,
};

//! Interface for memory signature
const isp_mem_t isp_signature = {
	.size        = sizeof(mem_signature),
	.fnct_read   = mem_signature_read,
	.fnct_write  = NULL,
};

//! Interface for memory no available
const isp_mem_t isp_no_available = {
	.size        = 0,
	.fnct_read   = NULL,
	.fnct_write  = NULL,
};

#if (UDI_DFU_ATMEL_PROTOCOL_VERSION == DFU_ATMEL_PROTOCOL_VERSION_1)
const isp_mems_t isp_memories = {
	.flash =      			&isp_flash,
	.eeprom =     			&isp_eeprom,
	.bootloader = 			&isp_bootloader,
	.signature =  			&isp_signature,
	.user =       			&isp_no_available,
};
#endif

#if (UDI_DFU_ATMEL_PROTOCOL_VERSION == DFU_ATMEL_PROTOCOL_VERSION_2)
const isp_mems_t isp_memories = {
	.flash =      			&isp_flash,
	.eeprom =     			&isp_eeprom,
	.security =   			&isp_no_available,
	.conf =       			&isp_no_available,
	.bootloader = 			&isp_bootloader,
	.signature =  			&isp_signature,
	.user =       			&isp_no_available,
	.int_ram =    			&isp_no_available,
	.ext_mem_cs0 =			&isp_no_available,
	.ext_mem_cs1 =			&isp_no_available,
	.ext_mem_cs2 =			&isp_no_available,
	.ext_mem_cs3 =			&isp_no_available,
	.ext_mem_cs4 =			&isp_no_available,
	.ext_mem_cs5 =			&isp_no_available,
	.ext_mem_cs6 =			&isp_no_available,
	.ext_mem_cs7 =			&isp_no_available,
	.ext_mem_df =      	&isp_no_available,
};
#endif

//@}

void isp_init(void)
{
	mem_signature.manufacture = MCU.DEVID0;
	mem_signature.product_number_msb = MCU.DEVID1;
	mem_signature.product_number_lsb = MCU.DEVID2;
	mem_signature.product_revision = MCU.REVID;
}

bool isp_is_security(void)
{
	return !(NVM.LOCKBITS&NVM_LOCKBITS_LB1_bm);
}

void isp_force_isp(bool force)
{
}

bool isp_erase_chip(void)
{
	nvm_flash_erase_app();
	nvm_eeprom_erase_all();
	return true;
}

void isp_start_appli_rst(void)
{
	cpu_irq_disable();
	// generate soft reset for xmega
	start_app_key=0x55AA;
	ccp_write_io((uint8_t *)&RST.CTRL, RST.CTRL | RST_SWRST_bm);
	while (1);
}

void isp_start_appli_norst(void)
{
	isp_start_appli_rst();
}

