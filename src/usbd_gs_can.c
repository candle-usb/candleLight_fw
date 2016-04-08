#include "usbd_gs_can.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"
#include "usbd_ioreq.h"
#include "gs_usb.h"

#define CAN_DATA_MAX_PACKET_SIZE 32  /* Endpoint IN & OUT Packet size */
#define CAN_CMD_PACKET_SIZE      64  /* Control Endpoint Packet size */
#define USB_CAN_CONFIG_DESC_SIZ  32

typedef struct {
	uint32_t RxLength;
	__IO uint32_t TxState;
	__IO uint32_t RxState;
	__IO uint32_t echo_id;

	uint8_t req_bRequest;
	uint8_t req_wLength;
	uint8_t slcan_str_index;
	uint8_t _dummy;

	uint8_t cmd_buf[CAN_CMD_PACKET_SIZE];
	uint8_t rx_buf[CAN_DATA_MAX_PACKET_SIZE];
	uint8_t tx_buf[CAN_DATA_MAX_PACKET_SIZE];

	struct gs_host_config host_config;
	struct gs_device_mode device_mode;
	struct gs_device_bittiming bittiming;

} USBD_GS_CAN_HandleTypeDef;

static uint8_t USBD_GS_CAN_PrepareReceive(USBD_HandleTypeDef *pdev);

static uint8_t USBD_GS_CAN_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_GS_CAN_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_GS_CAN_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_GS_CAN_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_GS_CAN_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_GS_CAN_EP0_RxReady(USBD_HandleTypeDef *pdev);
static uint8_t *USBD_GS_CAN_GetCfgDesc(uint16_t *len);
static uint8_t *USBD_GS_CAN_GetDeviceQualifierDescriptor(uint16_t *length);
static uint8_t USBD_GS_CAN_Transmit(USBD_HandleTypeDef *pdev, uint8_t *buf, uint16_t len);

/* CAN interface class callbacks structure */
USBD_ClassTypeDef USBD_GS_CAN = {
	USBD_GS_CAN_Init,
	USBD_GS_CAN_DeInit,
	USBD_GS_CAN_Setup,
	NULL,                 /* EP0_TxSent, */
	USBD_GS_CAN_EP0_RxReady,
	USBD_GS_CAN_DataIn,
	USBD_GS_CAN_DataOut,
	NULL,
	NULL,
	NULL,
	USBD_GS_CAN_GetCfgDesc,
	USBD_GS_CAN_GetCfgDesc,
	USBD_GS_CAN_GetCfgDesc,
	USBD_GS_CAN_GetDeviceQualifierDescriptor,
};


/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_GS_CAN_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
	USB_LEN_DEV_QUALIFIER_DESC,
	USB_DESC_TYPE_DEVICE_QUALIFIER,
	0x00,
	0x02,
	0x00,
	0x00,
	0x00,
	0x40,
	0x01,
	0x00,
};


/* GS_USB device Configuration Descriptor */
__ALIGN_BEGIN uint8_t USBD_GS_CAN_CfgDesc[USB_CAN_CONFIG_DESC_SIZ] __ALIGN_END =
{
	/*Configuration Descriptor*/
	0x09,   /* bLength: Configuration Descriptor size */
	USB_DESC_TYPE_CONFIGURATION,      /* bDescriptorType: Configuration */
	USB_CAN_CONFIG_DESC_SIZ,                /* wTotalLength:no of returned bytes */
	0x00,
	0x01,   /* bNumInterfaces */
	0x01,   /* bConfigurationValue: Configuration value */
	0x00,   /* iConfiguration: Index of string descriptor describing the configuration */
	0x80,   /* bmAttributes */
	0x4B,   /* MaxPower 150 mA */

	/*---------------------------------------------------------------------------*/

	/*Interface Descriptor */
	0x09,   /* bLength: Interface Descriptor size */
	USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: Interface */
	/* Interface descriptor type */
	0x00,   /* bInterfaceNumber: Number of Interface */
	0x00,   /* bAlternateSetting: Alternate setting */
	0x02,   /* bNumEndpoints */
	0xFF,   /* bInterfaceClass: Vendor Specific*/
	0x00,   /* bInterfaceSubClass: */
	0x00,   /* bInterfaceProtocol: */
	0x00,   /* iInterface: */

	/*---------------------------------------------------------------------------*/
	/* EP1 descriptor */
	0x07,   /* bLength */
	USB_DESC_TYPE_ENDPOINT,   /* bDescriptorType */
	GSUSB_ENDPOINT_IN,
	0x02, /* bmAttributes: bulk */
	LOBYTE(CAN_DATA_MAX_PACKET_SIZE),     /* wMaxPacketSize: */
	HIBYTE(CAN_DATA_MAX_PACKET_SIZE),
	0x00,                           /* bInterval: */
	/*---------------------------------------------------------------------------*/

	/*---------------------------------------------------------------------------*/
	/* EP2 descriptor */
	0x07,   /* bLength */
	USB_DESC_TYPE_ENDPOINT,   /* bDescriptorType */
	GSUSB_ENDPOINT_OUT,
	0x02, /* bmAttributes: bulk */
	LOBYTE(CAN_DATA_MAX_PACKET_SIZE),     /* wMaxPacketSize: */
	HIBYTE(CAN_DATA_MAX_PACKET_SIZE),
	0x00,                           /* bInterval: */
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
		hcan->echo_id = 0xFFFFFFFF;
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
    		memcpy(&hcan->host_config, hcan->cmd_buf, sizeof(hcan->host_config));
    		break;

    	case GS_USB_BREQ_MODE:
    		// TODO set device mode (flags, start/reset...)
    		memcpy(&hcan->device_mode, hcan->cmd_buf, sizeof(hcan->device_mode));
    		break;

    	case GS_USB_BREQ_BITTIMING:
    		// TODO set bit timing
    		memcpy(&hcan->bittiming, hcan->cmd_buf, sizeof(hcan->bittiming));
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
			USBD_CtlPrepareRx(pdev, hcan->cmd_buf, req->wLength);
			break;

		case GS_USB_BREQ_DEVICE_CONFIG:
			memcpy(hcan->cmd_buf, &dconf, sizeof(dconf));
			USBD_CtlSendData(pdev, hcan->cmd_buf, req->wLength);
			break;

		case GS_USB_BREQ_BT_CONST:
			memcpy(hcan->cmd_buf, &btconst, sizeof(btconst));
			USBD_CtlSendData(pdev, hcan->cmd_buf, req->wLength);
			break;

	}

	return USBD_OK;
}

static uint8_t USBD_GS_CAN_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{

	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*) pdev->pClassData;
	static uint8_t ifalt = 0;

	switch (req->bmRequest & USB_REQ_TYPE_MASK) {

		case USB_REQ_TYPE_VENDOR:
			return USBD_GS_CAN_Vendor_Request(pdev, req);

		case USB_REQ_TYPE_STANDARD:
			switch (req->bRequest) {
				case USB_REQ_GET_INTERFACE:
					USBD_CtlSendData (pdev, &ifalt, 1);
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
		struct gs_host_frame *hf = (struct gs_host_frame*) hcan->rx_buf;

		// TODO send can message

		hcan->echo_id = hf->echo_id;
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

uint8_t *USBD_GS_CAN_GetDeviceQualifierDescriptor(uint16_t *length)
{
	*length = sizeof(USBD_GS_CAN_DeviceQualifierDesc);
	return USBD_GS_CAN_DeviceQualifierDesc;
}

static uint8_t USBD_GS_CAN_PrepareReceive(USBD_HandleTypeDef *pdev)
{
	/* Suspend or Resume USB Out process */
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*)pdev->pClassData;
	return USBD_LL_PrepareReceive(pdev, GSUSB_ENDPOINT_OUT, hcan->rx_buf, CAN_DATA_MAX_PACKET_SIZE);
}

static uint8_t USBD_GS_CAN_Transmit(USBD_HandleTypeDef *pdev, uint8_t *buf, uint16_t len)
{
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*)pdev->pClassData;
	// TODO handle received CAN messages via this function
	if (hcan->TxState == 0) {
		hcan->TxState = 1;

		USBD_memset(hcan->tx_buf, 0, CAN_DATA_MAX_PACKET_SIZE);
		USBD_memcpy(hcan->tx_buf, buf, len);
		USBD_LL_Transmit(pdev, GSUSB_ENDPOINT_IN, hcan->tx_buf, len);
		return USBD_OK;
	} else {
		return USBD_BUSY;
	}

}

void USBD_GS_CAN_MessageReceived(
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
