/*

The MIT License (MIT)

Copyright (c) 2016 Hubert Denkmair

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#include <stdint.h>
#include <string.h>

#include "config.h"
#include "usbd_ctlreq.h"
#include "usbd_desc.h"
#include "util.h"

static uint8_t *USBD_FS_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *USBD_FS_LangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *USBD_FS_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *USBD_FS_ProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *USBD_FS_SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *USBD_FS_ConfigStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *USBD_FS_InterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);

const USBD_DescriptorsTypeDef FS_Desc = {
	USBD_FS_DeviceDescriptor,
	USBD_FS_LangIDStrDescriptor,
	USBD_FS_ManufacturerStrDescriptor,
	USBD_FS_ProductStrDescriptor,
	USBD_FS_SerialStrDescriptor,
	USBD_FS_ConfigStrDescriptor,
	USBD_FS_InterfaceStrDescriptor,
};

__ALIGN_BEGIN uint8_t USBD_DescBuf[USBD_DESC_BUF_SIZE] __ALIGN_END;

/* USB_DeviceDescriptor */
static const uint8_t USBD_FS_DeviceDesc[USB_LEN_DEV_DESC] = {
	0x12,                       /* bLength */
	USB_DESC_TYPE_DEVICE,       /* bDescriptorType */
	0x00,                       /* bcdUSB */
	0x02,
	0x00,                       /* bDeviceClass */
	0x00,                       /* bDeviceSubClass */
	0x00,                       /* bDeviceProtocol */
	USB_MAX_EP0_SIZE,           /* bMaxPacketSize */
	LOBYTE(USBD_VID),           /* idVendor */
	HIBYTE(USBD_VID),
	LOBYTE(USBD_PID_FS),        /* idProduct */
	HIBYTE(USBD_PID_FS),
	0x00,                       /* bcdDevice rel. 0.00 */
	0x00,
	USBD_IDX_MFC_STR,           /* Index of manufacturer string */
	USBD_IDX_PRODUCT_STR,       /* Index of product string */
	USBD_IDX_SERIAL_STR,        /* Index of serial number string */
	USBD_MAX_NUM_CONFIGURATION  /* bNumConfigurations */
};

/* USB Standard Device Descriptor */
static const uint8_t USBD_LangIDDesc[USB_LEN_LANGID_STR_DESC] =
{
	USB_LEN_LANGID_STR_DESC,
	USB_DESC_TYPE_STRING,
	LOBYTE(USBD_LANGID_STRING),
	HIBYTE(USBD_LANGID_STRING),
};

uint8_t *USBD_FS_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
	UNUSED(speed);
	*length = sizeof(USBD_FS_DeviceDesc);
	memcpy(USBD_DescBuf, USBD_FS_DeviceDesc, sizeof(USBD_FS_DeviceDesc));
	return USBD_DescBuf;
}

uint8_t *USBD_FS_LangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
	UNUSED(speed);
	*length = sizeof(USBD_LangIDDesc);
	memcpy(USBD_DescBuf, USBD_LangIDDesc, sizeof(USBD_LangIDDesc));
	return USBD_DescBuf;
}

uint8_t *USBD_FS_ProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
	UNUSED(speed);
	USBD_GetString(USBD_PRODUCT_STRING_FS, USBD_DescBuf, length);
	return USBD_DescBuf;
}

uint8_t *USBD_FS_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
	UNUSED(speed);
	USBD_GetString (USBD_MANUFACTURER_STRING, USBD_DescBuf, length);
	return USBD_DescBuf;
}

uint8_t *USBD_FS_SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
	char buf[25];

	UNUSED(speed);

	hex32(buf,		*(uint32_t*)(UID_BASE    ));
	hex32(buf +  8, *(uint32_t*)(UID_BASE + 4));
	hex32(buf + 16, *(uint32_t*)(UID_BASE + 8));

	USBD_GetString((uint8_t*)buf, USBD_DescBuf, length);
	return USBD_DescBuf;
}

uint8_t *USBD_FS_ConfigStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
	UNUSED(speed);
	USBD_GetString(USBD_CONFIGURATION_STRING_FS, USBD_DescBuf, length);
	return USBD_DescBuf;
}

uint8_t *USBD_FS_InterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
	UNUSED(speed);
	USBD_GetString(USBD_INTERFACE_STRING_FS, USBD_DescBuf, length);
	return USBD_DescBuf;
}
