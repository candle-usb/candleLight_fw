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

#include "usbd_gs_can.h"
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include "stm32f0xx_hal.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"
#include "usbd_ioreq.h"
#include "gs_usb.h"
#include "can.h"
#include "timer.h"
#include "flash.h"

typedef struct {
	uint8_t ep0_buf[CAN_CMD_PACKET_SIZE];

	__IO uint32_t TxState;

	USBD_SetupReqTypedef last_setup_request;

	struct gs_host_config host_config;
	queue_t *q_frame_pool;
	queue_t *q_from_host;

        struct gs_host_frame *from_host_buf;

	can_data_t *channels[NUM_CAN_CHANNEL];

	uint32_t out_requests;
	uint32_t out_requests_fail;
	uint32_t out_requests_no_buf;

	led_data_t *leds;
	bool dfu_detach_requested;

	bool timestamps_enabled;
	uint32_t sof_timestamp_us;

        bool pad_pkts_to_max_pkt_size;

} USBD_GS_CAN_HandleTypeDef __attribute__ ((aligned (4)));

static uint8_t USBD_GS_CAN_Start(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_GS_CAN_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_GS_CAN_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_GS_CAN_EP0_RxReady(USBD_HandleTypeDef *pdev);
static uint8_t USBD_GS_CAN_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t *USBD_GS_CAN_GetCfgDesc(uint16_t *len);
static uint8_t USBD_GS_CAN_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t *USBD_GS_CAN_GetStrDesc(USBD_HandleTypeDef *pdev, uint8_t index, uint16_t *length);
static uint8_t USBD_GS_CAN_SOF(struct _USBD_HandleTypeDef *pdev);

/* CAN interface class callbacks structure */
USBD_ClassTypeDef USBD_GS_CAN = {
	USBD_GS_CAN_Start,
	USBD_GS_CAN_DeInit,
	USBD_GS_CAN_Setup,
	NULL, // EP0_TxSent
	USBD_GS_CAN_EP0_RxReady,
	USBD_GS_CAN_DataIn,
	USBD_GS_CAN_DataOut,
	USBD_GS_CAN_SOF,
	NULL, // IsoInComplete
	NULL, // IsoOutComplete
	USBD_GS_CAN_GetCfgDesc,
	USBD_GS_CAN_GetCfgDesc,
	USBD_GS_CAN_GetCfgDesc,
	NULL, // GetDeviceQualifierDescriptor
	USBD_GS_CAN_GetStrDesc // GetUsrStrDescriptor
};


/* Configuration Descriptor */
__ALIGN_BEGIN uint8_t USBD_GS_CAN_CfgDesc[USB_CAN_CONFIG_DESC_SIZ] __ALIGN_END =
{
	/*---------------------------------------------------------------------------*/
	/* Configuration Descriptor */
	0x09,                             /* bLength */
	USB_DESC_TYPE_CONFIGURATION,      /* bDescriptorType */
	USB_CAN_CONFIG_DESC_SIZ,          /* wTotalLength */
	0x00,
	0x02,                             /* bNumInterfaces */
	0x01,                             /* bConfigurationValue */
	0x00,                             /* iConfiguration */
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
__ALIGN_BEGIN uint8_t USBD_GS_CAN_WINUSB_STR[] __ALIGN_END =
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
static __ALIGN_BEGIN uint8_t USBD_MS_COMP_ID_FEATURE_DESC[] __ALIGN_END = {
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
static __ALIGN_BEGIN uint8_t USBD_MS_EXT_PROP_FEATURE_DESC[] __ALIGN_END = {
	0x92, 0x00, 0x00, 0x00, /* length */
	0x00, 0x01,				/* version 1.0 */
	0x05, 0x00,             /* descr index (0x0005) */
	0x01, 0x00,             /* number of sections */
	0x88, 0x00, 0x00, 0x00, /* property section size */
	0x07, 0x00, 0x00, 0x00, /* property data type 7: Unicode REG_MULTI_SZ */
	0x2a, 0x00,				/* property name length */

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
	0, // reserved 1
	0, // reserved 2
	0, // reserved 3
	0, // interface count (0=1, 1=2..)
	2, // software version
	1  // hardware version
};

// bit timing constraints
static const struct gs_device_bt_const USBD_GS_CAN_btconst = {
	GS_CAN_FEATURE_LISTEN_ONLY  // supported features
	| GS_CAN_FEATURE_LOOP_BACK
	| GS_CAN_FEATURE_HW_TIMESTAMP
	| GS_CAN_FEATURE_IDENTIFY
	| GS_CAN_FEATURE_USER_ID
	| GS_CAN_FEATURE_PAD_PKTS_TO_MAX_PKT_SIZE,
	48000000, // can timing base clock
	1, // tseg1 min
	16, // tseg1 max
	1, // tseg2 min
	8, // tseg2 max
	4, // sjw max
	1, // brp min
	1024, //brp_max
	1, // brp increment;
};


uint8_t USBD_GS_CAN_Init(USBD_HandleTypeDef *pdev, queue_t *q_frame_pool, queue_t *q_from_host, led_data_t *leds)
{
	uint8_t ret = USBD_FAIL;
	USBD_GS_CAN_HandleTypeDef *hcan = calloc(1, sizeof(USBD_GS_CAN_HandleTypeDef));

	if(hcan != 0) {
		hcan->q_frame_pool = q_frame_pool;
		hcan->q_from_host = q_from_host;
		hcan->leds = leds;
		pdev->pClassData = hcan;
		hcan->from_host_buf = NULL;

		ret = USBD_OK;
	} else {
		pdev->pClassData = 0;
	}

	return ret;
}



static uint8_t USBD_GS_CAN_Start(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
	UNUSED(cfgidx);
	uint8_t ret = USBD_FAIL;

	if (pdev->pClassData) {
	  USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*) pdev->pClassData;
		USBD_LL_OpenEP(pdev, GSUSB_ENDPOINT_IN, USBD_EP_TYPE_BULK, CAN_DATA_MAX_PACKET_SIZE);
		USBD_LL_OpenEP(pdev, GSUSB_ENDPOINT_OUT, USBD_EP_TYPE_BULK, CAN_DATA_MAX_PACKET_SIZE);
                hcan->from_host_buf = queue_pop_front(hcan->q_frame_pool);
		USBD_GS_CAN_PrepareReceive(pdev);
		ret = USBD_OK;
	} else {
		ret = USBD_FAIL;
	}

	return ret;
}

static uint8_t USBD_GS_CAN_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
	UNUSED(cfgidx);

	USBD_LL_CloseEP(pdev, GSUSB_ENDPOINT_IN);
	USBD_LL_CloseEP(pdev, GSUSB_ENDPOINT_OUT);

	return USBD_OK;
}

static uint8_t USBD_GS_CAN_SOF(struct _USBD_HandleTypeDef *pdev)
{
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*) pdev->pClassData;
	hcan->sof_timestamp_us = timer_get();
	return USBD_OK;
}

void USBD_GS_CAN_SetChannel(USBD_HandleTypeDef *pdev, uint8_t channel, can_data_t* handle) {
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*) pdev->pClassData;
	if ((hcan!=NULL) && (channel < NUM_CAN_CHANNEL)) {
		hcan->channels[channel] = handle;
	}
}

static led_seq_step_t led_identify_seq[] = {
		{ .state = 0x01, .time_in_10ms = 10 },
		{ .state = 0x02, .time_in_10ms = 10 },
		{ .state = 0x00, .time_in_10ms = 0 }
};

static uint8_t USBD_GS_CAN_EP0_RxReady(USBD_HandleTypeDef *pdev) {

	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*) pdev->pClassData;

	struct gs_device_bittiming *timing;
	struct gs_device_mode *mode;
	can_data_t *ch;
	uint32_t param_u32;

	USBD_SetupReqTypedef *req = &hcan->last_setup_request;

    switch (req->bRequest) {

    	case GS_USB_BREQ_HOST_FORMAT:
    		// TODO process host data (expect 0x0000beef in byte_order)
    		memcpy(&hcan->host_config, hcan->ep0_buf, sizeof(hcan->host_config));
    		break;

    	case GS_USB_BREQ_IDENTIFY:
    		memcpy(&param_u32, hcan->ep0_buf, sizeof(param_u32));
    		if (param_u32) {
    			led_run_sequence(hcan->leds, led_identify_seq, -1);
    		} else {
    			ch = hcan->channels[req->wValue]; // TODO verify wValue input data (implement getChannelData() ?)
        		led_set_mode(hcan->leds, can_is_enabled(ch) ? led_mode_normal : led_mode_off);
    		}
    		break;

    	case GS_USB_BREQ_SET_USER_ID:
    		memcpy(&param_u32, hcan->ep0_buf, sizeof(param_u32));
    		if (flash_set_user_id(req->wValue, param_u32)) {
    			flash_flush();
    		}
    		break;

    	case GS_USB_BREQ_MODE:
    		if (req->wValue < NUM_CAN_CHANNEL) {

    			mode = (struct gs_device_mode*)hcan->ep0_buf;
    			ch = hcan->channels[req->wValue];

				if (mode->mode == GS_CAN_MODE_RESET) {

					can_disable(ch);
					led_set_mode(hcan->leds, led_mode_off);

				} else if (mode->mode == GS_CAN_MODE_START) {

					hcan->timestamps_enabled = (mode->flags & GS_CAN_MODE_HW_TIMESTAMP) != 0;
					hcan->pad_pkts_to_max_pkt_size = (mode->flags & GS_CAN_MODE_PAD_PKTS_TO_MAX_PKT_SIZE) != 0;

					can_enable(ch,
						(mode->flags & GS_CAN_MODE_LOOP_BACK) != 0,
						(mode->flags & GS_CAN_MODE_LISTEN_ONLY) != 0,
						(mode->flags & GS_CAN_MODE_ONE_SHOT) != 0
						// triple sampling not supported on bxCAN
					);

					led_set_mode(hcan->leds, led_mode_normal);
				}
			}
    		break;

    	case GS_USB_BREQ_BITTIMING:
    		timing = (struct gs_device_bittiming*)hcan->ep0_buf;
    		if (req->wValue < NUM_CAN_CHANNEL) {
				can_set_bittiming(
					hcan->channels[req->wValue],
					timing->brp,
					timing->prop_seg + timing->phase_seg1,
					timing->phase_seg2,
					timing->sjw
				);
    		}
    		break;

		default:
			break;
    }

	req->bRequest = 0xFF;
	return USBD_OK;
}

static uint8_t USBD_GS_CAN_DFU_Request(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*) pdev->pClassData;
	switch (req->bRequest) {

		case 0: // DETACH request
			hcan->dfu_detach_requested = true;
			break;

		case 3: // GET_STATIS request
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

static uint8_t USBD_GS_CAN_Config_Request(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*) pdev->pClassData;
	uint32_t d32;

	switch (req->bRequest) {

		case GS_USB_BREQ_HOST_FORMAT:
		case GS_USB_BREQ_MODE:
		case GS_USB_BREQ_BITTIMING:
		case GS_USB_BREQ_IDENTIFY:
		case GS_USB_BREQ_SET_USER_ID:
			hcan->last_setup_request = *req;
			USBD_CtlPrepareRx(pdev, hcan->ep0_buf, req->wLength);
			break;

		case GS_USB_BREQ_DEVICE_CONFIG:
			memcpy(hcan->ep0_buf, &USBD_GS_CAN_dconf, sizeof(USBD_GS_CAN_dconf));
			USBD_CtlSendData(pdev, hcan->ep0_buf, req->wLength);
			break;

		case GS_USB_BREQ_BT_CONST:
			memcpy(hcan->ep0_buf, &USBD_GS_CAN_btconst, sizeof(USBD_GS_CAN_btconst));
			USBD_CtlSendData(pdev, hcan->ep0_buf, req->wLength);
			break;

		case GS_USB_BREQ_TIMESTAMP:
			memcpy(hcan->ep0_buf, &hcan->sof_timestamp_us, sizeof(hcan->sof_timestamp_us));
			USBD_CtlSendData(pdev, hcan->ep0_buf, sizeof(hcan->sof_timestamp_us));
    		break;

		case GS_USB_BREQ_GET_USER_ID:
			if (req->wValue < NUM_CAN_CHANNEL) {
				d32 = flash_get_user_id(req->wValue);
				memcpy(hcan->ep0_buf, &d32, sizeof(d32));
				USBD_CtlSendData(pdev, hcan->ep0_buf, sizeof(d32));
			} else {
				USBD_CtlError(pdev, req);
			}
			break;


		default:
			USBD_CtlError(pdev, req);
	}

	return USBD_OK;
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

bool USBD_GS_CAN_CustomDeviceRequest(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
	uint16_t len = 0;
	uint8_t *pbuf;

	if (req->bRequest == USBD_GS_CAN_VENDOR_CODE) {

		switch (req->wIndex) {

			case 0x0004:
				pbuf = USBD_MS_COMP_ID_FEATURE_DESC;
				len = sizeof(USBD_MS_COMP_ID_FEATURE_DESC);
				USBD_CtlSendData(pdev, pbuf, MIN(len, req->wLength));
				return true;

			case 0x0005:
				if (req->wValue==0) { // only return our GUID for interface #0
					pbuf = USBD_MS_EXT_PROP_FEATURE_DESC;
					len = sizeof(USBD_MS_EXT_PROP_FEATURE_DESC);
					USBD_CtlSendData(pdev, pbuf, MIN(len, req->wLength));
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

static uint8_t USBD_GS_CAN_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum) {
	(void) epnum;

	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*)pdev->pClassData;
	hcan->TxState = 0;
	return USBD_OK;
}

static uint8_t USBD_GS_CAN_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum) {

	uint8_t retval = USBD_FAIL;

	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*)pdev->pClassData;

	hcan->out_requests++;

	uint32_t rxlen = USBD_LL_GetRxDataSize(pdev, epnum);
	if (rxlen >= (sizeof(struct gs_host_frame)-4)) {
	        struct gs_host_frame *frame = queue_pop_front_i(hcan->q_frame_pool);
		if(frame){
		        queue_push_back_i(hcan->q_from_host, hcan->from_host_buf);
		        hcan->from_host_buf = frame;
		  
		        retval = USBD_OK;
		}
		else{
		// Discard current packet from host if we have no place
		// to put the next one
		}
	}
	USBD_GS_CAN_PrepareReceive(pdev);
    return retval;
}

static uint8_t *USBD_GS_CAN_GetCfgDesc(uint16_t *len)
{
	*len = sizeof(USBD_GS_CAN_CfgDesc);
	return USBD_GS_CAN_CfgDesc;
}

inline uint8_t USBD_GS_CAN_PrepareReceive(USBD_HandleTypeDef *pdev)
{
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*)pdev->pClassData;
	return USBD_LL_PrepareReceive(pdev, GSUSB_ENDPOINT_OUT, (uint8_t*)hcan->from_host_buf, sizeof(*hcan->from_host_buf));
}

bool USBD_GS_CAN_TxReady(USBD_HandleTypeDef *pdev)
{
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*)pdev->pClassData;
	return hcan->TxState == 0;
}

uint8_t USBD_GS_CAN_Transmit(USBD_HandleTypeDef *pdev, uint8_t *buf, uint16_t len)
{
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*)pdev->pClassData;
	if (hcan->TxState == 0) {
		hcan->TxState = 1;
		USBD_LL_Transmit(pdev, GSUSB_ENDPOINT_IN, buf, len);
		return USBD_OK;
	} else {
		return USBD_BUSY;
	}
}

uint8_t USBD_GS_CAN_GetProtocolVersion(USBD_HandleTypeDef *pdev)
{
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*)pdev->pClassData;
	if (hcan->timestamps_enabled) {
		return 2;
	} else {
		return 1;
	}
}

uint8_t USBD_GS_CAN_GetPadPacketsToMaxPacketSize(USBD_HandleTypeDef *pdev)
{
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*)pdev->pClassData;
	return hcan->pad_pkts_to_max_pkt_size;
}

uint8_t USBD_GS_CAN_SendFrame(USBD_HandleTypeDef *pdev, struct gs_host_frame *frame)
{
        uint8_t buf[CAN_DATA_MAX_PACKET_SIZE],*send_addr;
  
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*)pdev->pClassData;
	size_t len = sizeof(struct gs_host_frame);

	if (!hcan->timestamps_enabled)
	  len -= 4;

	send_addr = (uint8_t *)frame;
	
	if(hcan->pad_pkts_to_max_pkt_size){
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
   
	return USBD_GS_CAN_Transmit(pdev, send_addr, len);
}

uint8_t *USBD_GS_CAN_GetStrDesc(USBD_HandleTypeDef *pdev, uint8_t index, uint16_t *length)
{
	UNUSED(pdev);

	switch (index) {
		case DFU_INTERFACE_STR_INDEX:
			USBD_GetString(DFU_INTERFACE_STRING_FS, USBD_StrDesc, length);
			return USBD_StrDesc;
		case 0xEE:
			*length = sizeof(USBD_GS_CAN_WINUSB_STR);
			return USBD_GS_CAN_WINUSB_STR;
		default:
			*length = 0;
			USBD_CtlError(pdev, 0);
			return 0;
	}
}

bool USBD_GS_CAN_DfuDetachRequested(USBD_HandleTypeDef *pdev)
{
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*)pdev->pClassData;
	return hcan->dfu_detach_requested;
}

