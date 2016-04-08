#include "usbd_gs_can.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"
#include "usbd_ioreq.h"
#include "gs_usb.h"

#define CAN_DATA_MAX_PACKET_SIZE 32  /* Endpoint IN & OUT Packet size */
#define CAN_CMD_PACKET_SIZE      64  /* Control Endpoint Packet size */
#define USB_CAN_CONFIG_DESC_SIZ  32

typedef struct {
	__IO uint32_t TxState;

	uint8_t req_bRequest;
	uint8_t req_wLength;

	uint8_t ep0_buf[CAN_CMD_PACKET_SIZE];
	uint8_t ep_out_buf[CAN_DATA_MAX_PACKET_SIZE];
	uint8_t ep_in_buf[CAN_DATA_MAX_PACKET_SIZE];

	struct gs_host_config host_config;
	struct gs_device_mode device_mode;
	struct gs_device_bittiming bittiming;

} USBD_GS_CAN_HandleTypeDef;

static uint8_t USBD_GS_CAN_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_GS_CAN_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_GS_CAN_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_GS_CAN_EP0_RxReady(USBD_HandleTypeDef *pdev);
static uint8_t USBD_GS_CAN_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_GS_CAN_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t *USBD_GS_CAN_GetCfgDesc(uint16_t *len);

static uint8_t USBD_GS_CAN_PrepareReceive(USBD_HandleTypeDef *pdev);
static uint8_t USBD_GS_CAN_Transmit(USBD_HandleTypeDef *pdev, uint8_t *buf, uint16_t len);

/* CAN interface class callbacks structure */
USBD_ClassTypeDef USBD_GS_CAN = {
	USBD_GS_CAN_Init,
	USBD_GS_CAN_DeInit,
	USBD_GS_CAN_Setup,
	NULL, // EP0_TxSent
	USBD_GS_CAN_EP0_RxReady,
	USBD_GS_CAN_DataIn,
	USBD_GS_CAN_DataOut,
	NULL, // SOF
	NULL, // IsoInComplete
	NULL, // IsoOutComplete
	USBD_GS_CAN_GetCfgDesc,
	USBD_GS_CAN_GetCfgDesc,
	USBD_GS_CAN_GetCfgDesc,
	NULL, // GetDeviceQualifierDescriptor
};


/* Configuration Descriptor */
__ALIGN_BEGIN uint8_t USBD_GS_CAN_CfgDesc[USB_CAN_CONFIG_DESC_SIZ] __ALIGN_END =
{
	/*---------------------------------------------------------------------------*/
	/* Configuration Descriptor */
	0x09,                        /* bLength */
	USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType */
	USB_CAN_CONFIG_DESC_SIZ,     /* wTotalLength */
	0x00,
	0x01,   /* bNumInterfaces */
	0x01,   /* bConfigurationValue */
	0x00,   /* iConfiguration */
	0x80,   /* bmAttributes */
	0x4B,   /* MaxPower 150 mA */
	/*---------------------------------------------------------------------------*/

	/*---------------------------------------------------------------------------*/
	/* Interface Descriptor */
	0x09,   /* bLength */
	USB_DESC_TYPE_INTERFACE,  /* bDescriptorType */
	0x00,   /* bInterfaceNumber */
	0x00,   /* bAlternateSetting */
	0x02,   /* bNumEndpoints */
	0xFF,   /* bInterfaceClass: Vendor Specific*/
	0x00,   /* bInterfaceSubClass */
	0x00,   /* bInterfaceProtocol */
	0x00,   /* iInterface */
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

};

static uint8_t USBD_GS_CAN_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
	(void) cfgidx;
	uint8_t ret = 0;
  
	USBD_LL_OpenEP(pdev, GSUSB_ENDPOINT_IN, USBD_EP_TYPE_BULK, CAN_DATA_MAX_PACKET_SIZE);
	USBD_LL_OpenEP(pdev, GSUSB_ENDPOINT_OUT, USBD_EP_TYPE_BULK, CAN_DATA_MAX_PACKET_SIZE);

	USBD_GS_CAN_HandleTypeDef *hcan = USBD_malloc(sizeof(USBD_GS_CAN_HandleTypeDef));
  
	if(hcan == 0) {
		ret = 1;
	} else {
		memset(hcan, 0, sizeof(USBD_GS_CAN_HandleTypeDef));
		pdev->pClassData = hcan;
		USBD_GS_CAN_PrepareReceive(pdev);
	}
	return ret;
}

static uint8_t USBD_GS_CAN_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
	(void) cfgidx;
	uint8_t ret = 0;

	USBD_LL_CloseEP(pdev, GSUSB_ENDPOINT_IN);
	USBD_LL_CloseEP(pdev, GSUSB_ENDPOINT_OUT);

	/* DeInit  physical Interface components */
	if(pdev->pClassData != NULL) {
		USBD_free(pdev->pClassData);
		pdev->pClassData = NULL;
	}

	return ret;
}

static uint8_t USBD_GS_CAN_EP0_RxReady(USBD_HandleTypeDef *pdev) {

	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*) pdev->pClassData;

    switch (hcan->req_bRequest) {

    	case GS_USB_BREQ_HOST_FORMAT:
    		// TODO process host data (expect 0x0000beef in byte_order)
    		memcpy(&hcan->host_config, hcan->ep0_buf, sizeof(hcan->host_config));
    		break;

    	case GS_USB_BREQ_MODE:
    		// TODO set device mode (flags, start/reset...)
    		memcpy(&hcan->device_mode, hcan->ep0_buf, sizeof(hcan->device_mode));
    		break;

    	case GS_USB_BREQ_BITTIMING:
    		// TODO set bit timing
    		memcpy(&hcan->bittiming, hcan->ep0_buf, sizeof(hcan->bittiming));
    		break;

		default:
			break;
    }


	hcan->req_bRequest = 0xFF;
	return USBD_OK;
}

static uint8_t USBD_GS_CAN_Vendor_Request(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*) pdev->pClassData;

	struct gs_device_config dconf;
	memset(&dconf, 0, sizeof(dconf));
	dconf.icount = 0;
	dconf.hw_version = 1;
	dconf.sw_version = 1;

	struct gs_device_bt_const btconst;
	memset(&btconst, 0, sizeof(btconst));
	btconst.fclk_can = 48000000;
	btconst.tseg1_min = 1;
	btconst.tseg1_max = 16;
	btconst.tseg2_min = 1;
	btconst.tseg2_max = 8;
	btconst.sjw_max = 4;
	btconst.brp_min = 1;
	btconst.brp_max = 1024;
	btconst.brp_inc = 1;
	btconst.feature = GS_CAN_FEATURE_LISTEN_ONLY | GS_CAN_FEATURE_LOOP_BACK;

	switch (req->bRequest) {

		case GS_USB_BREQ_HOST_FORMAT:
		case GS_USB_BREQ_MODE:
		case GS_USB_BREQ_BITTIMING:
			hcan->req_bRequest = req->bRequest;
			hcan->req_wLength = (uint8_t)req->wLength;
			USBD_CtlPrepareRx(pdev, hcan->ep0_buf, req->wLength);
			break;

		case GS_USB_BREQ_DEVICE_CONFIG:
			memcpy(hcan->ep0_buf, &dconf, sizeof(dconf));
			USBD_CtlSendData(pdev, hcan->ep0_buf, req->wLength);
			break;

		case GS_USB_BREQ_BT_CONST:
			memcpy(hcan->ep0_buf, &btconst, sizeof(btconst));
			USBD_CtlSendData(pdev, hcan->ep0_buf, req->wLength);
			break;

	}

	return USBD_OK;
}

static uint8_t USBD_GS_CAN_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
	static uint8_t ifalt = 0;

	switch (req->bmRequest & USB_REQ_TYPE_MASK) {

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

	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*)pdev->pClassData;

	uint32_t rxlen = USBD_LL_GetRxDataSize(pdev, epnum);
	if (rxlen >= sizeof(struct gs_host_frame)) {
		struct gs_host_frame *hf = (struct gs_host_frame*) hcan->ep_out_buf;
		// TODO process and send echo back to host (from non-interrupt context?)
	}

	USBD_GS_CAN_PrepareReceive(pdev);
    return USBD_OK;
}

static uint8_t *USBD_GS_CAN_GetCfgDesc(uint16_t *len)
{
	*len = sizeof(USBD_GS_CAN_CfgDesc);
	return USBD_GS_CAN_CfgDesc;
}

static uint8_t USBD_GS_CAN_PrepareReceive(USBD_HandleTypeDef *pdev)
{
	/* Suspend or Resume USB Out process */
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*)pdev->pClassData;
	return USBD_LL_PrepareReceive(pdev, GSUSB_ENDPOINT_OUT, hcan->ep_out_buf, CAN_DATA_MAX_PACKET_SIZE);
}

static uint8_t USBD_GS_CAN_Transmit(USBD_HandleTypeDef *pdev, uint8_t *buf, uint16_t len)
{
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*)pdev->pClassData;
	// TODO handle received CAN messages via this function
	if (hcan->TxState == 0) {
		hcan->TxState = 1;

		USBD_memset(hcan->ep_in_buf, 0, CAN_DATA_MAX_PACKET_SIZE);
		USBD_memcpy(hcan->ep_in_buf, buf, len);
		USBD_LL_Transmit(pdev, GSUSB_ENDPOINT_IN, hcan->ep_in_buf, len);
		return USBD_OK;
	} else {
		return USBD_BUSY;
	}

}

void USBD_GS_CAN_SendFrameToHost(
	USBD_HandleTypeDef *pdev,
	uint32_t echo_id,
	uint32_t can_id,
	uint8_t dlc,
	uint8_t channel,
	uint8_t flags,
	uint8_t *data
) {
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*)pdev->pClassData;
	struct gs_host_frame hf;

	if (dlc>8) { dlc = 8; }

	hf.echo_id = echo_id;
	hf.can_id = can_id;
	hf.can_dlc = dlc;
	hf.channel = channel;
	hf.flags = flags;

	for (int i=0; i<dlc; i++) {
		hf.data[i] = data[i];
	}

	if (hcan->device_mode.mode == GS_CAN_MODE_START) {
		USBD_GS_CAN_Transmit(pdev, (uint8_t*)&hf, sizeof(hf));
	}
}
