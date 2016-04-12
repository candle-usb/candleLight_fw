#include "usbd_gs_can.h"

#include "stm32f0xx_hal.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"
#include "usbd_ioreq.h"
#include "gs_usb.h"
#include "can.h"

#define CAN_DATA_MAX_PACKET_SIZE 32  /* Endpoint IN & OUT Packet size */
#define CAN_CMD_PACKET_SIZE      64  /* Control Endpoint Packet size */
#define USB_CAN_CONFIG_DESC_SIZ  32
#define NUM_CAN_CHANNEL           1

typedef struct {
	uint8_t ep0_buf[CAN_CMD_PACKET_SIZE];
	uint8_t ep_in_buf[CAN_DATA_MAX_PACKET_SIZE];

	__IO uint32_t TxState;

	uint8_t  req_bRequest;
	uint16_t req_wLength;
	uint16_t req_wValue;


	struct gs_host_config host_config;
	queue_t *q_frame_pool;
	queue_t *q_from_host;

	struct gs_host_frame *from_host_buf;

	CAN_HandleTypeDef *channels[NUM_CAN_CHANNEL];

} USBD_GS_CAN_HandleTypeDef __attribute__ ((aligned (4)));

static uint8_t USBD_GS_CAN_Start(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_GS_CAN_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_GS_CAN_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_GS_CAN_EP0_RxReady(USBD_HandleTypeDef *pdev);
static uint8_t USBD_GS_CAN_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t *USBD_GS_CAN_GetCfgDesc(uint16_t *len);
static uint8_t USBD_GS_CAN_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t USBD_GS_CAN_PrepareReceive(USBD_HandleTypeDef *pdev);

/* CAN interface class callbacks structure */
USBD_ClassTypeDef USBD_GS_CAN = {
	USBD_GS_CAN_Start,
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
	0x09,                             /* bLength */
	USB_DESC_TYPE_CONFIGURATION,      /* bDescriptorType */
	USB_CAN_CONFIG_DESC_SIZ,          /* wTotalLength */
	0x00,
	0x01,                             /* bNumInterfaces */
	0x01,                             /* bConfigurationValue */
	0x00,                             /* iConfiguration */
	0x80,                             /* bmAttributes */
	0x4B,                             /* MaxPower 150 mA */
	/*---------------------------------------------------------------------------*/

	/*---------------------------------------------------------------------------*/
	/* Interface Descriptor */
	0x09,                             /* bLength */
	USB_DESC_TYPE_INTERFACE,          /* bDescriptorType */
	0x00,                             /* bInterfaceNumber */
	0x00,                             /* bAlternateSetting */
	0x02,                             /* bNumEndpoints */
	0xFF,                             /* bInterfaceClass: Vendor Specific*/
	0x00,                             /* bInterfaceSubClass */
	0x00,                             /* bInterfaceProtocol */
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

};

uint8_t USBD_GS_CAN_Init(USBD_HandleTypeDef *pdev, queue_t *q_frame_pool, queue_t *q_from_host)
{
	uint8_t ret = USBD_FAIL;
	USBD_GS_CAN_HandleTypeDef *hcan = calloc(1, sizeof(USBD_GS_CAN_HandleTypeDef));

	if(hcan != 0) {
		hcan->q_frame_pool = q_frame_pool;
		hcan->q_from_host = q_from_host;
		pdev->pClassData = hcan;
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


void USBD_GS_CAN_SetChannel(USBD_HandleTypeDef *pdev, uint8_t channel, CAN_HandleTypeDef* handle) {
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*) pdev->pClassData;
	if ((hcan!=NULL) && (channel < NUM_CAN_CHANNEL)) {
		hcan->channels[channel] = handle;
	}
}

static uint8_t USBD_GS_CAN_EP0_RxReady(USBD_HandleTypeDef *pdev) {

	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*) pdev->pClassData;

	struct gs_device_bittiming *timing;
	struct gs_device_mode *mode;
	CAN_HandleTypeDef *ch;

    switch (hcan->req_bRequest) {

    	case GS_USB_BREQ_HOST_FORMAT:
    		// TODO process host data (expect 0x0000beef in byte_order)
    		memcpy(&hcan->host_config, hcan->ep0_buf, sizeof(hcan->host_config));
    		break;

    	case GS_USB_BREQ_MODE:
    		if (hcan->req_wValue < NUM_CAN_CHANNEL) {

    			mode = (struct gs_device_mode*)hcan->ep0_buf;
    			ch = hcan->channels[hcan->req_wValue];

				if (mode->mode == GS_CAN_MODE_RESET) {

					can_disable(ch);

				} else if (mode->mode == GS_CAN_MODE_START) {

					can_enable(ch,
						(mode->flags & GS_CAN_MODE_LOOP_BACK) != 0,
						(mode->flags & GS_CAN_MODE_LISTEN_ONLY) != 0,
						(mode->flags & GS_CAN_MODE_ONE_SHOT) != 0
						// triple sampling not supported on bxCAN
					);

				}
			}
    		break;

    	case GS_USB_BREQ_BITTIMING:
    		timing = (struct gs_device_bittiming*)hcan->ep0_buf;
    		if (hcan->req_wValue < NUM_CAN_CHANNEL) {
				can_set_bittiming(
					hcan->channels[hcan->req_wValue],
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
			hcan->req_wLength = req->wLength;
			hcan->req_wValue = req->wValue;
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

	uint8_t retval = USBD_FAIL;

	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*)pdev->pClassData;

	uint32_t rxlen = USBD_LL_GetRxDataSize(pdev, epnum);
	if (rxlen >= sizeof(struct gs_host_frame)) {
		queue_push_back_i(hcan->q_from_host, hcan->from_host_buf);
		hcan->from_host_buf = queue_pop_front_i(hcan->q_frame_pool);
		retval = USBD_OK;
	}

	USBD_GS_CAN_PrepareReceive(pdev);
    return retval;
}

static uint8_t *USBD_GS_CAN_GetCfgDesc(uint16_t *len)
{
	*len = sizeof(USBD_GS_CAN_CfgDesc);
	return USBD_GS_CAN_CfgDesc;
}

static uint8_t USBD_GS_CAN_PrepareReceive(USBD_HandleTypeDef *pdev)
{
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*)pdev->pClassData;
	return USBD_LL_PrepareReceive(pdev, GSUSB_ENDPOINT_OUT, hcan->from_host_buf, sizeof(struct gs_host_frame));
}

bool USBD_GS_CAN_TxReady(USBD_HandleTypeDef *pdev)
{
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*)pdev->pClassData;
	return hcan->TxState == 0;
}

uint8_t USBD_GS_CAN_Transmit(USBD_HandleTypeDef *pdev, uint8_t *buf, uint16_t len)
{
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*)pdev->pClassData;
	if (USBD_GS_CAN_TxReady(pdev)) {
		hcan->TxState = 1;
		USBD_LL_Transmit(pdev, GSUSB_ENDPOINT_IN, buf, len);
		return USBD_OK;
	} else {
		return USBD_BUSY;
	}

}
