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

#include <stdlib.h>
#include <string.h>

#include "can.h"
#include "config.h"
#include "gpio.h"
#include "gs_usb.h"
#include "hal_include.h"
#include "led.h"
#include "timer.h"
#include "usbd_core.h"
#include "usbd_ctlreq.h"
#include "usbd_def.h"
#include "usbd_desc.h"
#include "usbd_gs_can.h"
#include "usbd_ioreq.h"
#include "util.h"

static volatile bool is_usb_suspend_cb = false;

/* Configuration Descriptor */
static const uint8_t USBD_GS_CAN_CfgDesc[USB_CAN_CONFIG_DESC_SIZ] =
{
	/*---------------------------------------------------------------------------*/
	/* Configuration Descriptor */
	0x09,                             /* bLength */
	USB_DESC_TYPE_CONFIGURATION,      /* bDescriptorType */
	USB_CAN_CONFIG_DESC_SIZ,          /* wTotalLength */
	0x00,
	0x02,                             /* bNumInterfaces */
	0x01,                             /* bConfigurationValue */
	USBD_IDX_CONFIG_STR,              /* iConfiguration */
	0x80,                             /* bmAttributes */
	0x4B,                             /* MaxPower 150 mA */
	/*---------------------------------------------------------------------------*/

	/*---------------------------------------------------------------------------*/
	/* GS_USB Interface Descriptor */
	0x09,                             /* bLength */
	USB_DESC_TYPE_INTERFACE,          /* bDescriptorType */
	0x00,                             /* bInterfaceNumber */
	0x00,                             /* bAlternateSetting */
	0x02,                             /* bNumEndpoints */
	0xFF,                             /* bInterfaceClass: Vendor Specific*/
	0xFF,                             /* bInterfaceSubClass: Vendor Specific */
	0xFF,                             /* bInterfaceProtocol: Vendor Specific */
	0x00,                             /* iInterface */
	/*---------------------------------------------------------------------------*/

	/*---------------------------------------------------------------------------*/
	/* EP1 descriptor */
	0x07,                             /* bLength */
	USB_DESC_TYPE_ENDPOINT,           /* bDescriptorType */
	GSUSB_ENDPOINT_IN,                /* bEndpointAddress */
	0x02,                             /* bmAttributes: bulk */
	LOBYTE(CAN_DATA_MAX_PACKET_SIZE), /* wMaxPacketSize */
	HIBYTE(CAN_DATA_MAX_PACKET_SIZE),
	0x00,                             /* bInterval: */
	/*---------------------------------------------------------------------------*/

	/*---------------------------------------------------------------------------*/
	/* EP2 descriptor */
	0x07,                             /* bLength */
	USB_DESC_TYPE_ENDPOINT,           /* bDescriptorType */
	GSUSB_ENDPOINT_OUT,               /* bEndpointAddress */
	0x02,                             /* bmAttributes: bulk */
	LOBYTE(CAN_DATA_MAX_PACKET_SIZE), /* wMaxPacketSize */
	HIBYTE(CAN_DATA_MAX_PACKET_SIZE),
	0x00,                             /* bInterval: */
	/*---------------------------------------------------------------------------*/

	/*---------------------------------------------------------------------------*/
	/* DFU Interface Descriptor */
	/*---------------------------------------------------------------------------*/
	0x09,                             /* bLength */
	USB_DESC_TYPE_INTERFACE,          /* bDescriptorType */
	DFU_INTERFACE_NUM,                /* bInterfaceNumber */
	0x00,                             /* bAlternateSetting */
	0x00,                             /* bNumEndpoints */
	0xFE,                             /* bInterfaceClass: Vendor Specific*/
	0x01,                             /* bInterfaceSubClass */
	0x01,                             /* bInterfaceProtocol : Runtime mode */
	DFU_INTERFACE_STR_INDEX,          /* iInterface */

	/*---------------------------------------------------------------------------*/
	/* Run-Time DFU Functional Descriptor */
	/*---------------------------------------------------------------------------*/
	0x09,                             /* bLength */
	0x21,                             /* bDescriptorType: DFU FUNCTIONAL */
	0x0B,                             /* bmAttributes: detach, upload, download */
	0xFF, 0x00,                       /* wDetachTimeOut */
	0x00, 0x08,                       /* wTransferSize */
	0x1a, 0x01,                       /* bcdDFUVersion: 1.1a */

};

/* Microsoft OS String Descriptor */
static const uint8_t USBD_GS_CAN_WINUSB_STR[] =
{
	0x12,                    /* length */
	0x03,                    /* descriptor type == string */
	0x4D, 0x00, 0x53, 0x00,  /* signature: "MSFT100" */
	0x46, 0x00, 0x54, 0x00,
	0x31, 0x00, 0x30, 0x00,
	0x30, 0x00,
	USBD_GS_CAN_VENDOR_CODE, /* vendor code */
	0x00                     /* padding */
};

/*  Microsoft Compatible ID Feature Descriptor  */
static const uint8_t USBD_MS_COMP_ID_FEATURE_DESC[] = {
	0x40, 0x00, 0x00, 0x00, /* length */
	0x00, 0x01,             /* version 1.0 */
	0x04, 0x00,             /* descr index (0x0004) */
	0x02,                   /* number of sections */
	0x00, 0x00, 0x00, 0x00, /* reserved */
	0x00, 0x00, 0x00,
	0x00,                   /* interface number */
	0x01,                   /* reserved */
	0x57, 0x49, 0x4E, 0x55, /* compatible ID ("WINUSB\0\0") */
	0x53, 0x42, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, /* sub-compatible ID */
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, /* reserved */
	0x00, 0x00,
	0x01,                   /* interface number */
	0x01,                   /* reserved */
	0x57, 0x49, 0x4E, 0x55, /* compatible ID ("WINUSB\0\0") */
	0x53, 0x42, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, /* sub-compatible ID */
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, /* reserved */
	0x00, 0x00
};

/* Microsoft Extended Properties Feature Descriptor */
static const uint8_t USBD_MS_EXT_PROP_FEATURE_DESC[] = {
	0x92, 0x00, 0x00, 0x00, /* length */
	0x00, 0x01,             /* version 1.0 */
	0x05, 0x00,             /* descr index (0x0005) */
	0x01, 0x00,             /* number of sections */
	0x88, 0x00, 0x00, 0x00, /* property section size */
	0x07, 0x00, 0x00, 0x00, /* property data type 7: Unicode REG_MULTI_SZ */
	0x2a, 0x00,             /* property name length */

	0x44, 0x00, 0x65, 0x00, /* property name "DeviceInterfaceGUIDs" */
	0x76, 0x00, 0x69, 0x00,
	0x63, 0x00, 0x65, 0x00,
	0x49, 0x00, 0x6e, 0x00,
	0x74, 0x00, 0x65, 0x00,
	0x72, 0x00, 0x66, 0x00,
	0x61, 0x00, 0x63, 0x00,
	0x65, 0x00, 0x47, 0x00,
	0x55, 0x00, 0x49, 0x00,
	0x44, 0x00, 0x73, 0x00,
	0x00, 0x00,

	0x50, 0x00, 0x00, 0x00, /* property data length */

	0x7b, 0x00, 0x63, 0x00, /* property name: "{c15b4308-04d3-11e6-b3ea-6057189e6443}\0\0" */
	0x31, 0x00, 0x35, 0x00,
	0x62, 0x00, 0x34, 0x00,
	0x33, 0x00, 0x30, 0x00,
	0x38, 0x00, 0x2d, 0x00,
	0x30, 0x00, 0x34, 0x00,
	0x64, 0x00, 0x33, 0x00,
	0x2d, 0x00, 0x31, 0x00,
	0x31, 0x00, 0x65, 0x00,
	0x36, 0x00, 0x2d, 0x00,
	0x62, 0x00, 0x33, 0x00,
	0x65, 0x00, 0x61, 0x00,
	0x2d, 0x00, 0x36, 0x00,
	0x30, 0x00, 0x35, 0x00,
	0x37, 0x00, 0x31, 0x00,
	0x38, 0x00, 0x39, 0x00,
	0x65, 0x00, 0x36, 0x00,
	0x34, 0x00, 0x34, 0x00,
	0x33, 0x00, 0x7d, 0x00,
	0x00, 0x00, 0x00, 0x00
};

// device info
static const struct gs_device_config USBD_GS_CAN_dconf = {
	.icount = NUM_CAN_CHANNEL - 1,
	.sw_version = 2,
	.hw_version = 1,
};

// bit timing constraints
static const struct gs_device_bt_const USBD_GS_CAN_btconst = {
	.feature =
		GS_CAN_FEATURE_LISTEN_ONLY |
		GS_CAN_FEATURE_LOOP_BACK |
		GS_CAN_FEATURE_HW_TIMESTAMP |
		GS_CAN_FEATURE_IDENTIFY |
		GS_CAN_FEATURE_PAD_PKTS_TO_MAX_PKT_SIZE
#ifdef TERM_Pin
		| GS_CAN_FEATURE_TERMINATION
#endif
	,
	.fclk_can = CAN_CLOCK_SPEED,
	.tseg1_min = 1,
	.tseg1_max = 16,
	.tseg2_min = 1,
	.tseg2_max = 8,
	.sjw_max = 4,
	.brp_min = 1,
	.brp_max = 1024,
	.brp_inc = 1,
};

/* It's unclear from the documentation, but it appears that the USB library is
 * not safely reentrant. It attempts to signal errors via return values if it is
 * reentered, but that code is not interrupt-safe and the error values are
 * silently ignored within the library in several cases. We'll just disable
 * interrupts at all entry points to be safe. Note that the callbacks are all
 * called from within the libary itself, either within the interrupt handler or
 * within other calls, which means the USB interrupt is already disabled and we
 * don't have any other interrupts to worry about. */

static inline uint8_t USBD_GS_CAN_PrepareReceive(USBD_HandleTypeDef *pdev)
{
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*)pdev->pClassData;
	struct gs_host_frame *frame = &hcan->from_host_buf->frame;

	return USBD_LL_PrepareReceive(pdev, GSUSB_ENDPOINT_OUT, (uint8_t *)frame, sizeof(*frame));
}

static uint8_t USBD_GS_CAN_Start(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
	UNUSED(cfgidx);

	assert_basic(pdev->pClassData);
	USBD_LL_OpenEP(pdev, GSUSB_ENDPOINT_IN,	 USBD_EP_TYPE_BULK, CAN_DATA_MAX_PACKET_SIZE);
	USBD_LL_OpenEP(pdev, GSUSB_ENDPOINT_OUT, USBD_EP_TYPE_BULK, CAN_DATA_MAX_PACKET_SIZE);
	USBD_GS_CAN_PrepareReceive(pdev);

	return USBD_OK;

}

static uint8_t USBD_GS_CAN_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
	UNUSED(cfgidx);

	USBD_LL_CloseEP(pdev, GSUSB_ENDPOINT_IN);
	USBD_LL_CloseEP(pdev, GSUSB_ENDPOINT_OUT);

	return USBD_OK;
}

static uint8_t USBD_GS_CAN_DFU_Request(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*) pdev->pClassData;
	switch (req->bRequest) {

		case 0: // DETACH request
			hcan->dfu_detach_requested = true;
			break;

		case 3: // GET_STATUS request
			hcan->ep0_buf[0] = 0x00; // bStatus: 0x00 == OK
			hcan->ep0_buf[1] = 0x00; // bwPollTimeout
			hcan->ep0_buf[2] = 0x00;
			hcan->ep0_buf[3] = 0x00;
			hcan->ep0_buf[4] = 0x00; // bState: appIDLE
			hcan->ep0_buf[5] = 0xFF; // status string descriptor index
			USBD_CtlSendData(pdev, hcan->ep0_buf, 6);
			break;

		default:
			USBD_CtlError(pdev, req);

	}
	return USBD_OK;
}

static can_data_t *USBD_GS_CAN_GetChannel(USBD_GS_CAN_HandleTypeDef *hcan,
										  const uint16_t ch)
{
	if (ch < ARRAY_SIZE(hcan->channels)) {
		return &hcan->channels[ch];
	}

	return NULL;
}

static uint8_t USBD_GS_CAN_Config_Request(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*) pdev->pClassData;
	struct gs_device_termination_state term_state;
	can_data_t *channel;
	const void *src = NULL;
	size_t len;

	/*
	 * For all "per device" USB control messages
	 * (GS_USB_BREQ_HOST_FORMAT and GS_USB_BREQ_DEVICE_CONFIG) the
	 * Linux gs_usb driver uses a req->wValue = 1.
	 *
	 * All other control messages are "per channel" and specify the
	 * channel number in req->wValue. So check req->wValue for valid
	 * CAN channel.
	 *
	 */
	if (!(req->bRequest == GS_USB_BREQ_HOST_FORMAT ||
		  req->bRequest == GS_USB_BREQ_DEVICE_CONFIG)) {
		channel = USBD_GS_CAN_GetChannel(hcan, req->wValue);
		if (!channel) {
			goto out_fail;
		}
	}

	switch (req->bRequest) {
		// Host -> Device
		case GS_USB_BREQ_HOST_FORMAT:
			len = sizeof(struct gs_host_config);
			break;
		case GS_USB_BREQ_BITTIMING:
			len = sizeof(struct gs_device_bittiming);
			break;
		case GS_USB_BREQ_MODE:
			len = sizeof(struct gs_device_mode);
			break;
		case GS_USB_BREQ_BT_CONST:
			src = &USBD_GS_CAN_btconst;
			len = sizeof(USBD_GS_CAN_btconst);
			break;
		case GS_USB_BREQ_DEVICE_CONFIG:
			src = &USBD_GS_CAN_dconf;
			len = sizeof(USBD_GS_CAN_dconf);
			break;
		case GS_USB_BREQ_TIMESTAMP:
			src = &hcan->sof_timestamp_us;
			len = sizeof(hcan->sof_timestamp_us);
			break;
		case GS_USB_BREQ_IDENTIFY:
			len = sizeof(struct gs_identify_mode);
			break;
		case GS_USB_BREQ_SET_TERMINATION:
			if (get_term(req->wValue) == GS_CAN_TERMINATION_UNSUPPORTED) {
				goto out_fail;
			}

			len = sizeof(struct gs_device_termination_state);
			break;
		case GS_USB_BREQ_GET_TERMINATION: {
			enum gs_can_termination_state state;

			state = get_term(req->wValue);
			if (state == GS_CAN_TERMINATION_UNSUPPORTED) {
				goto out_fail;
			}

			term_state.state = state;
			src = &term_state;
			len = sizeof(term_state);
			break;
		}
		default:
			goto out_fail;
	}

	if (req->wLength < len) {
		goto out_fail;
	}

	switch (req->bRequest) {
		case GS_USB_BREQ_HOST_FORMAT:
		case GS_USB_BREQ_BITTIMING:
		case GS_USB_BREQ_MODE:
		case GS_USB_BREQ_IDENTIFY:
		case GS_USB_BREQ_SET_TERMINATION:
			if (req->wLength > sizeof(hcan->ep0_buf)) {
				goto out_fail;
			}

			hcan->last_setup_request = *req;
			USBD_CtlPrepareRx(pdev, hcan->ep0_buf, req->wLength);
			break;

		// Device -> Host
		case GS_USB_BREQ_BT_CONST:
		case GS_USB_BREQ_DEVICE_CONFIG:
		case GS_USB_BREQ_TIMESTAMP:
		case GS_USB_BREQ_GET_TERMINATION:
			USBD_CtlSendData(pdev, (uint8_t *)src, len);
			break;
		default:
			goto out_fail;
	}

	return USBD_OK;

out_fail:
	USBD_CtlError(pdev, req);
	return USBD_FAIL;
}

static uint8_t USBD_GS_CAN_Vendor_Request(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
	uint8_t req_rcpt = req->bmRequest & 0x1F;
	uint8_t req_type = (req->bmRequest >> 5) & 0x03;

	if (
		(req_type == 0x01) // class request
	   && (req_rcpt == 0x01) // recipient: interface
	   && (req->wIndex == DFU_INTERFACE_NUM)
		) {
		return USBD_GS_CAN_DFU_Request(pdev, req);
	} else {
		return USBD_GS_CAN_Config_Request(pdev, req);
	}
}

static uint8_t USBD_GS_CAN_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
	static uint8_t ifalt = 0;

	switch (req->bmRequest & USB_REQ_TYPE_MASK) {

		case USB_REQ_TYPE_CLASS:
		case USB_REQ_TYPE_VENDOR:
			return USBD_GS_CAN_Vendor_Request(pdev, req);

		case USB_REQ_TYPE_STANDARD:
			switch (req->bRequest) {
				case USB_REQ_GET_INTERFACE:
					USBD_CtlSendData(pdev, &ifalt, 1);
					break;

				case USB_REQ_SET_INTERFACE:
				default:
					break;
			}
			break;

		default:
			break;
	}
	return USBD_OK;
}

static const led_seq_step_t led_identify_seq[] = {
	{ .state = 0x01, .time_in_10ms = 10 },
	{ .state = 0x02, .time_in_10ms = 10 },
	{ .state = 0x00, .time_in_10ms = 0 }
};

static uint8_t USBD_GS_CAN_EP0_RxReady(USBD_HandleTypeDef *pdev) {

	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*) pdev->pClassData;
	can_data_t *channel = NULL;
	USBD_SetupReqTypedef *req = &hcan->last_setup_request;

	/*
	 * The control messages GS_USB_BREQ_HOST_FORMAT and
	 * GS_USB_BREQ_DEVICE_CONFIG are per device, all others are per
	 * channel. The validity of req->wValue has been checked in
	 * USBD_GS_CAN_Config_Request() already.
	 */
	if (!(req->bRequest == GS_USB_BREQ_HOST_FORMAT ||
		  req->bRequest == GS_USB_BREQ_DEVICE_CONFIG))
		channel = USBD_GS_CAN_GetChannel(hcan, req->wValue);

	switch (req->bRequest) {
		case GS_USB_BREQ_HOST_FORMAT:
			/* The firmware on the original USB2CAN by Geschwister Schneider
			 * Technologie Entwicklungs- und Vertriebs UG exchanges all data
			 * between the host and the device in host byte order. This is done
			 * with the struct gs_host_config::byte_order member, which is sent
			 * first to indicate the desired byte order.
			 *
			 * The widely used open source firmware candleLight doesn't support
			 * this feature and exchanges the data in little endian byte order.
			 */
			break;

		case GS_USB_BREQ_BITTIMING: {
			struct gs_device_bittiming *timing;

			timing = (struct gs_device_bittiming*)hcan->ep0_buf;
			can_set_bittiming(channel, timing->brp,
							  timing->prop_seg + timing->phase_seg1,
							  timing->phase_seg2,
							  timing->sjw);
			break;
		}
		case GS_USB_BREQ_MODE: {
			struct gs_device_mode *mode;

			mode = (struct gs_device_mode*)hcan->ep0_buf;

			if (mode->mode == GS_CAN_MODE_RESET) {
				can_disable(channel);
				led_set_mode(&channel->leds, led_mode_off);
			} else if (mode->mode == GS_CAN_MODE_START) {
				hcan->timestamps_enabled = (mode->flags & GS_CAN_MODE_HW_TIMESTAMP) != 0;
				hcan->pad_pkts_to_max_pkt_size = (mode->flags & GS_CAN_MODE_PAD_PKTS_TO_MAX_PKT_SIZE) != 0;

				can_enable(channel,
						   (mode->flags & GS_CAN_MODE_LOOP_BACK) != 0,
						   (mode->flags & GS_CAN_MODE_LISTEN_ONLY) != 0,
						   (mode->flags & GS_CAN_MODE_ONE_SHOT) != 0
				           // triple sampling not supported on bxCAN
						   );

				led_set_mode(&channel->leds, led_mode_normal);
			}
			break;
		}
		case GS_USB_BREQ_IDENTIFY: {
			struct gs_identify_mode *imode;

			imode = (struct gs_identify_mode *)hcan->ep0_buf;
			if (imode->mode) {
				led_run_sequence(&channel->leds, led_identify_seq, -1);
			} else {
				led_set_mode(&channel->leds, can_is_enabled(channel) ?
							 led_mode_normal : led_mode_off);
			}
			break;
		}
		case GS_USB_BREQ_SET_TERMINATION: {
			if (get_term(req->wValue) != GS_CAN_TERMINATION_UNSUPPORTED) {
				struct gs_device_termination_state *term_state;

				term_state = (struct gs_device_termination_state *)hcan->ep0_buf;
				if (set_term(req->wValue, term_state->state) == GS_CAN_TERMINATION_UNSUPPORTED) {
					USBD_CtlError(pdev, req);
				}
			}
			break;
		}
		default:
			break;
	}

	req->bRequest = 0xFF;
	return USBD_OK;
}

static uint8_t USBD_GS_CAN_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum) {
	(void) epnum;

	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*)pdev->pClassData;
	hcan->TxState = 0;
	return USBD_OK;
}

// Note that the return value is completely ignored by the stack.
static uint8_t USBD_GS_CAN_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum) {
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*)pdev->pClassData;

	uint32_t rxlen = USBD_LL_GetRxDataSize(pdev, epnum);
	if (rxlen < (sizeof(struct gs_host_frame)-4)) {
		// Invalid frame length, just ignore it and receive into the same buffer
		// again next time.
		USBD_GS_CAN_PrepareReceive(pdev);
		return USBD_OK;
	}

	bool was_irq_enabled = disable_irq();
	// Enqueue the frame we just received.
	list_add_tail(&hcan->from_host_buf->list, &hcan->list_from_host);

	// Grab a buffer for the next frame from the pool.
	hcan->from_host_buf = list_first_entry_or_null(&hcan->list_frame_pool,
												   struct gs_host_frame_object,
												   list);
	if (hcan->from_host_buf) {
		list_del(&hcan->from_host_buf->list);
		restore_irq(was_irq_enabled);

		// We got a buffer! Get ready to receive from the USB host into it.
		USBD_GS_CAN_PrepareReceive(pdev);
	} else {
		restore_irq(was_irq_enabled);

		// gs_can has no way to drop packets. If we just drop this one, gs_can
		// will fill up its queue of packets awaiting ACKs and then hang. Instead,
		// wait to call PrepareReceive until we have a frame to receive into.
	}
	return USBD_OK;
}

static uint8_t USBD_GS_CAN_SOF(struct _USBD_HandleTypeDef *pdev)
{
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*) pdev->pClassData;
	hcan->sof_timestamp_us = timer_get();
	return USBD_OK;
}

static uint8_t *USBD_GS_CAN_GetCfgDesc(uint16_t *len)
{
	*len = sizeof(USBD_GS_CAN_CfgDesc);
	memcpy(USBD_DescBuf, USBD_GS_CAN_CfgDesc, sizeof(USBD_GS_CAN_CfgDesc));
	return USBD_DescBuf;
}

uint8_t *USBD_GS_CAN_GetStrDesc(USBD_HandleTypeDef *pdev, uint8_t index, uint16_t *length)
{
	UNUSED(pdev);

	switch (index) {
		case DFU_INTERFACE_STR_INDEX:
			USBD_GetString(DFU_INTERFACE_STRING_FS, USBD_DescBuf, length);
			return USBD_DescBuf;
		case 0xEE:
			*length = sizeof(USBD_GS_CAN_WINUSB_STR);
			memcpy(USBD_DescBuf, USBD_GS_CAN_WINUSB_STR, sizeof(USBD_GS_CAN_WINUSB_STR));
			return USBD_DescBuf;
		default:
			*length = 0;
			USBD_CtlError(pdev, 0);
			return 0;
	}
}

/* CAN interface class callbacks structure */
USBD_ClassTypeDef USBD_GS_CAN = {
	.Init = USBD_GS_CAN_Start,
	.DeInit = USBD_GS_CAN_DeInit,
	.Setup = USBD_GS_CAN_Setup,
	.EP0_RxReady = USBD_GS_CAN_EP0_RxReady,
	.DataIn = USBD_GS_CAN_DataIn,
	.DataOut = USBD_GS_CAN_DataOut,
	.SOF = USBD_GS_CAN_SOF,
	.GetHSConfigDescriptor = USBD_GS_CAN_GetCfgDesc,
	.GetFSConfigDescriptor = USBD_GS_CAN_GetCfgDesc,
	.GetOtherSpeedConfigDescriptor = USBD_GS_CAN_GetCfgDesc,
	.GetUsrStrDescriptor = USBD_GS_CAN_GetStrDesc,
};

uint8_t USBD_GS_CAN_Init(USBD_GS_CAN_HandleTypeDef *hcan, USBD_HandleTypeDef *pdev)
{
	pdev->pClassData = hcan;

	return USBD_OK;
}

bool USBD_GS_CAN_CustomDeviceRequest(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
	if (req->bRequest == USBD_GS_CAN_VENDOR_CODE) {

		switch (req->wIndex) {

			case 0x0004:
				memcpy(USBD_DescBuf, USBD_MS_COMP_ID_FEATURE_DESC, sizeof(USBD_MS_COMP_ID_FEATURE_DESC));
				USBD_CtlSendData(pdev, USBD_DescBuf, MIN(sizeof(USBD_MS_COMP_ID_FEATURE_DESC), req->wLength));
				return true;

			case 0x0005:
				if (req->wValue==0) { // only return our GUID for interface #0
					memcpy(USBD_DescBuf, USBD_MS_EXT_PROP_FEATURE_DESC, sizeof(USBD_MS_EXT_PROP_FEATURE_DESC));
					USBD_CtlSendData(pdev, USBD_DescBuf, MIN(sizeof(USBD_MS_EXT_PROP_FEATURE_DESC), req->wLength));
					return true;
				}
				break;

		}

	}

	return false;
}

bool USBD_GS_CAN_CustomInterfaceRequest(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
	return USBD_GS_CAN_CustomDeviceRequest(pdev, req);
}

bool USBD_GS_CAN_TxReady(USBD_HandleTypeDef *pdev)
{
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*)pdev->pClassData;
	bool was_irq_enabled = disable_irq();
	if (!hcan->from_host_buf) {
		hcan->from_host_buf = list_first_entry_or_null(&hcan->list_frame_pool,
													   struct gs_host_frame_object,
													   list);
		if (hcan->from_host_buf) {
			list_del(&hcan->from_host_buf->list);
			USBD_GS_CAN_PrepareReceive(pdev);
		}
	}
	bool result = hcan->TxState == 0;
	restore_irq(was_irq_enabled);
	return result;
}

static uint8_t USBD_GS_CAN_Transmit(USBD_HandleTypeDef *pdev, uint8_t *buf, uint16_t len)
{
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*)pdev->pClassData;
	if (hcan->TxState == 0 && (false == is_usb_suspend_cb)) {
		hcan->TxState = 1;
		USBD_LL_Transmit(pdev, GSUSB_ENDPOINT_IN, buf, len);
		return USBD_OK;
	} else {
		return USBD_BUSY;
	}
}

uint8_t USBD_GS_CAN_SendFrame(USBD_HandleTypeDef *pdev, struct gs_host_frame *frame)
{
	uint8_t buf[CAN_DATA_MAX_PACKET_SIZE],*send_addr;

	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*)pdev->pClassData;
	size_t len = sizeof(struct gs_host_frame);

	if (!hcan->timestamps_enabled) {
		len -= 4;
	}

	send_addr = (uint8_t *)frame;

	if (hcan->pad_pkts_to_max_pkt_size) {
		// When talking to WinUSB it seems to help a lot if the
		// size of packet you send equals the max packet size.
		// In this mode, fill packets out to max packet size and
		// then send.
		memcpy(buf, frame, len);

		// zero rest of buffer
		memset(buf + len, 0, sizeof(buf) - len);
		send_addr = buf;
		len = sizeof(buf);
	}

	bool was_irq_enabled = disable_irq();
	uint8_t result = USBD_GS_CAN_Transmit(pdev, send_addr, len);
	restore_irq(was_irq_enabled);
	return result;
}

bool USBD_GS_CAN_DfuDetachRequested(USBD_HandleTypeDef *pdev)
{
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*)pdev->pClassData;
	return hcan->dfu_detach_requested;
}

// Handle USB suspend event
void USBD_GS_CAN_SuspendCallback(USBD_HandleTypeDef  *pdev)
{
	// Disable CAN and go off bus on USB suspend
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*) pdev->pClassData;

	for (unsigned int i = 0; i < ARRAY_SIZE(hcan->channels); i++) {
		can_data_t *channel = &hcan->channels[i];

		can_disable(channel);
		led_set_mode(&channel->leds, led_mode_off);
	}

	is_usb_suspend_cb = true;
}

void USBD_GS_CAN_ResumeCallback(USBD_HandleTypeDef  *pdev)
{
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*) pdev->pClassData;
	hcan->TxState = 0;
	is_usb_suspend_cb = false;
}
