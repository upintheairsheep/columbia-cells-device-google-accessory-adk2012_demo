/**
 * \file
 *
 * \brief Default descriptors for a USB Device with a single interface Audio 1.0
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

#include "conf_usb.h"
#include "udd.h"
#include "udc_desc.h"
#include "udi_audio.h"


/**
 * \ingroup udi_audio_group
 * \defgroup udi_audio_group_desc Default descriptors for a USB Device
 *
 * @{
 */

//! Two interfaces for this device
#define  USB_DEVICE_NB_INTERFACE       2

/**INDENT-OFF**/

//! USB Device Descriptor
COMPILER_WORD_ALIGNED
UDC_DESC_STORAGE usb_dev_desc_t udc_device_desc = {
	.bLength                   = sizeof(usb_dev_desc_t),
	.bDescriptorType           = USB_DT_DEVICE,
	.bcdUSB                    = LE16(USB_V2_0),
	.bDeviceClass              = 0,
	.bDeviceSubClass           = 0,
	.bDeviceProtocol           = 0,
	.bMaxPacketSize0           = USB_DEVICE_EP_CTRL_SIZE,
	.idVendor                  = LE16(USB_DEVICE_VENDOR_ID),
	.idProduct                 = LE16(USB_DEVICE_PRODUCT_ID),
	.bcdDevice                 = LE16((USB_DEVICE_MAJOR_VERSION << 8)
		| USB_DEVICE_MINOR_VERSION),
#ifdef USB_DEVICE_MANUFACTURE_NAME
	.iManufacturer             = 1,
#else
	.iManufacturer             = 0,  // No manufacture string 
#endif
#ifdef USB_DEVICE_PRODUCT_NAME
	.iProduct                  = 2,
#else
	.iProduct                  = 0,  // No product string 
#endif
#ifdef USB_DEVICE_SERIAL_NAME
	.iSerialNumber             = 3,
#else
	.iSerialNumber             = 0,  // No serial string 
#endif
	.bNumConfigurations        = 1
};


#ifdef USB_DEVICE_HS_SUPPORT
//! USB Device Qualifier Descriptor for HS
COMPILER_WORD_ALIGNED
UDC_DESC_STORAGE usb_dev_qual_desc_t udc_device_qual = {
	.bLength                   = sizeof(usb_dev_qual_desc_t),
	.bDescriptorType           = USB_DT_DEVICE_QUALIFIER,
	.bcdUSB                    = LE16(USB_V2_0),
	.bDeviceClass              = 0,
	.bDeviceSubClass           = 0,
	.bDeviceProtocol           = 0,
	.bMaxPacketSize0           = USB_DEVICE_EP_CTRL_SIZE,
	.bNumConfigurations        = 1
};
#endif

//! Structure for USB Device Configuration Descriptor
COMPILER_PACK_SET(1);
typedef struct {
	usb_conf_desc_t conf;
	udi_audio_control_desc_t udi_audio_control;
	udi_audio_data_desc_t udi_audio_data;
} udc_desc_t;
COMPILER_PACK_RESET();

//! USB Device Configuration Descriptor filled for FS
COMPILER_WORD_ALIGNED
UDC_DESC_STORAGE udc_desc_t udc_desc_fs = {
	.conf.bLength              = sizeof(usb_conf_desc_t),
	.conf.bDescriptorType      = USB_DT_CONFIGURATION,
	.conf.wTotalLength         = LE16(sizeof(udc_desc_t)),
	.conf.bNumInterfaces       = USB_DEVICE_NB_INTERFACE,
	.conf.bConfigurationValue  = 1,
	.conf.iConfiguration       = 0,
	.conf.bmAttributes         = USB_CONFIG_ATTR_MUST_SET | USB_DEVICE_ATTR,
	.conf.bMaxPower            = USB_CONFIG_MAX_POWER(USB_DEVICE_POWER),
	.udi_audio_control         = UDI_AUDIO_CONTROL_DESC,
	.udi_audio_data            = UDI_AUDIO_DATA_DESC_FS,
};

#ifdef USB_DEVICE_HS_SUPPORT
//! USB Device Configuration Descriptor filled for HS
COMPILER_WORD_ALIGNED
UDC_DESC_STORAGE udc_desc_t udc_desc_hs = {
	.conf.bLength              = sizeof(usb_conf_desc_t),
	.conf.bDescriptorType      = USB_DT_CONFIGURATION,
	.conf.wTotalLength         = LE16(sizeof(udc_desc_t)),
	.conf.bNumInterfaces       = USB_DEVICE_NB_INTERFACE,
	.conf.bConfigurationValue  = 1,
	.conf.iConfiguration       = 0,
	.conf.bmAttributes         = USB_CONFIG_ATTR_MUST_SET | USB_DEVICE_ATTR,
	.conf.bMaxPower            = USB_CONFIG_MAX_POWER(USB_DEVICE_POWER),
	.udi_audio_control         = UDI_AUDIO_CONTROL_DESC,
	.udi_audio_data            = UDI_AUDIO_DATA_DESC_HS,
};
#endif


/**
 * \name UDC structures which contain all USB Device definitions
 */
//@{

//! Associate an UDI for each USB interface 
UDC_DESC_STORAGE udi_api_t *udi_apis[USB_DEVICE_NB_INTERFACE] = {
	&udi_api_audio_control,
	&udi_api_audio_data,
};

//! Add UDI with USB Descriptors FS
UDC_DESC_STORAGE udc_config_speed_t   udc_config_lsfs[1] = {{
	.desc          = (usb_conf_desc_t UDC_DESC_STORAGE*)&udc_desc_fs,
	.udi_apis      = udi_apis,
}};

#ifdef USB_DEVICE_HS_SUPPORT
//! Add UDI with USB Descriptors HS
UDC_DESC_STORAGE udc_config_speed_t   udc_config_hs[1] = {{
	.desc          = (usb_conf_desc_t UDC_DESC_STORAGE*)&udc_desc_hs,
	.udi_apis      = udi_apis,
}};
#endif

//! Add all information about USB Device in global structure for UDC
UDC_DESC_STORAGE udc_config_t udc_config = {
	.confdev_lsfs = &udc_device_desc,
	.conf_lsfs = udc_config_lsfs,
#ifdef USB_DEVICE_HS_SUPPORT
	.confdev_hs = &udc_device_desc,
	.qualifier = &udc_device_qual,
	.conf_hs = udc_config_hs,
#endif
};

//@}
/**INDENT-ON**/
//@}
