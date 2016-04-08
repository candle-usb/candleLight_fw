#include "usbd_gs_can.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"
#include "usbd_ioreq.h"

#define u32 uint32_t
#define u8 uint8_t

enum gs_usb_breq {
	GS_USB_BREQ_HOST_FORMAT = 0,
	GS_USB_BREQ_BITTIMING,
	GS_USB_BREQ_MODE,
	GS_USB_BREQ_BERR,
	GS_USB_BREQ_BT_CONST,
	GS_USB_BREQ_DEVICE_CONFIG
};

enum gs_can_mode {
	/* reset a channel. turns it off */
	GS_CAN_MODE_RESET = 0,
	/* starts a channel */
	GS_CAN_MODE_START
};

enum gs_can_state {
	GS_CAN_STATE_ERROR_ACTIVE = 0,
	GS_CAN_STATE_ERROR_WARNING,
	GS_CAN_STATE_ERROR_PASSIVE,
	GS_CAN_STATE_BUS_OFF,
	GS_CAN_STATE_STOPPED,
	GS_CAN_STATE_SLEEPING
};

/* data types passed between host and device */
struct gs_host_config {
	u32 byte_order;
} __packed;
/* All data exchanged between host and device is exchanged in host byte order,
 * thanks to the struct gs_host_config byte_order member, which is sent first
 * to indicate the desired byte order.
 */

struct gs_device_config {
	u8 reserved1;
	u8 reserved2;
	u8 reserved3;
	u8 icount;
	u32 sw_version;
	u32 hw_version;
} __packed;

#define GS_CAN_MODE_NORMAL               0
#define GS_CAN_MODE_LISTEN_ONLY          (1<<0)
#define GS_CAN_MODE_LOOP_BACK            (1<<1)
#define GS_CAN_MODE_TRIPLE_SAMPLE        (1<<2)
#define GS_CAN_MODE_ONE_SHOT             (1<<3)

struct gs_device_mode {
	u32 mode;
	u32 flags;
} __packed;

struct gs_device_state {
	u32 state;
	u32 rxerr;
	u32 txerr;
} __packed;

struct gs_device_bittiming {
	u32 prop_seg;
	u32 phase_seg1;
	u32 phase_seg2;
	u32 sjw;
	u32 brp;
} __packed;

#define GS_CAN_FEATURE_LISTEN_ONLY      (1<<0)
#define GS_CAN_FEATURE_LOOP_BACK        (1<<1)
#define GS_CAN_FEATURE_TRIPLE_SAMPLE    (1<<2)
#define GS_CAN_FEATURE_ONE_SHOT         (1<<3)

struct gs_device_bt_const {
	u32 feature;
	u32 fclk_can;
	u32 tseg1_min;
	u32 tseg1_max;
	u32 tseg2_min;
	u32 tseg2_max;
	u32 sjw_max;
	u32 brp_min;
	u32 brp_max;
	u32 brp_inc;
} __packed;

#define GS_CAN_FLAG_OVERFLOW 1

struct gs_host_frame {
	u32 echo_id;
	u32 can_id;

	u8 can_dlc;
	u8 channel;
	u8 flags;
	u8 reserved;

	u8 data[8];
} __packed;
/* The GS USB devices make use of the same flags and masks as in
 * linux/can.h and linux/can/error.h, and no additional mapping is necessary.
 */

/* Only send a max of GS_MAX_TX_URBS frames per channel at a time. */
#define GS_MAX_TX_URBS 10
/* Only launch a max of GS_MAX_RX_URBS usb requests at a time. */
#define GS_MAX_RX_URBS 30
/* Maximum number of interfaces the driver supports per device.
 * Current hardware only supports 2 interfaces. The future may vary.
 */
#define GS_MAX_INTF 2

struct gs_tx_context {
	struct gs_can *dev;
	unsigned int echo_id;
};


#define SLCAN_IN_EP              0x81  /* EP1 for data IN */
#define SLCAN_OUT_EP             0x01  /* EP1 for data OUT */
#define CAN_CMD_EP               0x82  /* EP2 for CDC commands */
#define CAN_DATA_MAX_PACKET_SIZE 64  /* Endpoint IN & OUT Packet size */
#define CAN_CMD_PACKET_SIZE      64  /* Control Endpoint Packet size */

#define USB_CAN_CONFIG_DESC_SIZ  67
#define CDC_GET_LINE_CODING      0x21

typedef struct {
	uint32_t RxLength;
	uint32_t TxLength;
	__IO uint32_t TxState;
	__IO uint32_t RxState;

	uint8_t req_bRequest;
	uint8_t req_wLength;
	uint8_t slcan_str_index;
	uint8_t _dummy;

	uint8_t slcan_str[32];
	uint8_t cmd_buf[CAN_CMD_PACKET_SIZE];
	uint8_t rx_buf[CAN_DATA_MAX_PACKET_SIZE];
	uint8_t tx_buf[CAN_DATA_MAX_PACKET_SIZE];

	struct gs_host_config host_config;
	struct gs_device_mode device_mode;

} USBD_GS_CAN_HandleTypeDef;

static uint8_t USBD_GS_CAN_ReceivePacket(USBD_HandleTypeDef *pdev);

static uint8_t USBD_GS_CAN_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_GS_CAN_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_GS_CAN_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_GS_CAN_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_GS_CAN_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_GS_CAN_EP0_RxReady(USBD_HandleTypeDef *pdev);
static uint8_t *USBD_GS_CAN_GetCfgDesc(uint16_t *len);
static uint8_t *USBD_GS_CAN_GetDeviceQualifierDescriptor(uint16_t *length);


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
	0x02,   /* bNumInterfaces: 2 interface */
	0x01,   /* bConfigurationValue: Configuration value */
	0x00,   /* iConfiguration: Index of string descriptor describing the configuration */
	0xC0,   /* bmAttributes: self powered */
	0x32,   /* MaxPower 0 mA */

	/*---------------------------------------------------------------------------*/

	/*Interface Descriptor */
	0x09,   /* bLength: Interface Descriptor size */
	USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: Interface */
	/* Interface descriptor type */
	0x00,   /* bInterfaceNumber: Number of Interface */
	0x00,   /* bAlternateSetting: Alternate setting */
	0x01,   /* bNumEndpoints: One endpoints used */
	0xFF,   /* bInterfaceClass: Vendor Specific*/
	0x00,   /* bInterfaceSubClass: */
	0x00,   /* bInterfaceProtocol: */
	0x00,   /* iInterface: */

	/*Header Functional Descriptor*/
	0x05,   /* bLength: Endpoint Descriptor size */
	0x24,   /* bDescriptorType: CS_INTERFACE */
	0x00,   /* bDescriptorSubtype: Header Func Desc */
	0x10,   /* bcdCDC: spec release number */
	0x01,

	/*Call Management Functional Descriptor*/
	0x05,   /* bFunctionLength */
	0x24,   /* bDescriptorType: CS_INTERFACE */
	0x01,   /* bDescriptorSubtype: Call Management Func Desc */
	0x00,   /* bmCapabilities: D0+D1 */
	0x01,   /* bDataInterface: 1 */

	/*ACM Functional Descriptor*/
	0x04,   /* bFunctionLength */
	0x24,   /* bDescriptorType: CS_INTERFACE */
	0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
	0x02,   /* bmCapabilities */

	/*Union Functional Descriptor*/
	0x05,   /* bFunctionLength */
	0x24,   /* bDescriptorType: CS_INTERFACE */
	0x06,   /* bDescriptorSubtype: Union func desc */
	0x00,   /* bMasterInterface: Communication class interface */
	0x01,   /* bSlaveInterface0: Data Class Interface */

	/*Endpoint 2 Descriptor*/
	0x07,                           /* bLength: Endpoint Descriptor size */
	USB_DESC_TYPE_ENDPOINT,   /* bDescriptorType: Endpoint */
	CAN_CMD_EP,                     /* bEndpointAddress */
	0x03,                           /* bmAttributes: Interrupt */
	LOBYTE(CAN_CMD_PACKET_SIZE),     /* wMaxPacketSize: */
	HIBYTE(CAN_CMD_PACKET_SIZE),
	0x10,                           /* bInterval: */
	/*---------------------------------------------------------------------------*/

	/*Data class interface descriptor*/
	0x09,   /* bLength: Endpoint Descriptor size */
	USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: */
	0x01,   /* bInterfaceNumber: Number of Interface */
	0x00,   /* bAlternateSetting: Alternate setting */
	0x02,   /* bNumEndpoints: Two endpoints used */
	0x0A,   /* bInterfaceClass: CDC */
	0x00,   /* bInterfaceSubClass: */
	0x00,   /* bInterfaceProtocol: */
	0x00,   /* iInterface: */

	/*Endpoint OUT Descriptor*/
	0x07,   /* bLength: Endpoint Descriptor size */
	USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType: Endpoint */
	SLCAN_OUT_EP,                        /* bEndpointAddress */
	0x02,                              /* bmAttributes: Bulk */
	LOBYTE(CAN_DATA_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
	HIBYTE(CAN_DATA_MAX_PACKET_SIZE),
	0x00,                              /* bInterval: ignore for Bulk transfer */

	/*Endpoint IN Descriptor*/
	0x07,   /* bLength: Endpoint Descriptor size */
	USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType: Endpoint */
	SLCAN_IN_EP,                         /* bEndpointAddress */
	0x02,                              /* bmAttributes: Bulk */
	LOBYTE(CAN_DATA_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
	HIBYTE(CAN_DATA_MAX_PACKET_SIZE),
	0x00                               /* bInterval: ignore for Bulk transfer */
};

static uint8_t USBD_GS_CAN_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
	(void) cfgidx;
	uint8_t ret = 0;
  
	USBD_LL_OpenEP(pdev, SLCAN_IN_EP, USBD_EP_TYPE_BULK, CAN_DATA_MAX_PACKET_SIZE);
	USBD_LL_OpenEP(pdev, SLCAN_OUT_EP, USBD_EP_TYPE_BULK, CAN_DATA_MAX_PACKET_SIZE);
	USBD_LL_OpenEP(pdev, CAN_CMD_EP, USBD_EP_TYPE_INTR, CAN_CMD_PACKET_SIZE);

	USBD_GS_CAN_HandleTypeDef *hcan = USBD_malloc(sizeof(USBD_GS_CAN_HandleTypeDef));
  
	if(hcan == 0) {
		ret = 1;
	} else {
		USBD_memset(hcan, 0, sizeof(USBD_GS_CAN_HandleTypeDef));
		pdev->pClassData = hcan;
		USBD_LL_PrepareReceive(pdev, SLCAN_OUT_EP, hcan->rx_buf, CAN_DATA_MAX_PACKET_SIZE);
	}
	return ret;
}

static uint8_t USBD_GS_CAN_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
	(void) cfgidx;
	uint8_t ret = 0;

	USBD_LL_CloseEP(pdev, SLCAN_IN_EP);
	USBD_LL_CloseEP(pdev, SLCAN_OUT_EP);
	USBD_LL_CloseEP(pdev, CAN_CMD_EP);

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
    		// host data format received (expect 0x0000beef in byte_order)
    		memcpy(&hcan->host_config, hcan->cmd_buf, sizeof(hcan->host_config));
    		break;

    	case GS_USB_BREQ_MODE:
    		// set device mode (flags, start/reset...)
    		memcpy(&hcan->device_mode, hcan->cmd_buf, sizeof(hcan->device_mode));
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

	if(pdev->pClassData != NULL) {
		USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*)pdev->pClassData;
		hcan->TxState = 0;
		return USBD_OK;
	} else {
		return USBD_FAIL;
	}
}

static uint8_t USBD_GS_CAN_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum) {

	if (pdev->pClassData != NULL) {

		USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*)pdev->pClassData;
		hcan->RxLength = USBD_LL_GetRxDataSize(pdev, epnum);
  
		/* USB data will be immediately processed, this allow next USB traffic being
		NAKed till the end of the application Xfer */

	    for (uint32_t i=0; i<hcan->RxLength; i++) {
			if (hcan->rx_buf[i] == '\r') {
				//slcan_parse_str(hcan->slcan_str, hcan->slcan_str_index);
				hcan->slcan_str_index = 0;
			} else {
				hcan->slcan_str[hcan->slcan_str_index++] = hcan->rx_buf[i];
			}
	    }

	    // prepare for next read
	    USBD_GS_CAN_ReceivePacket(pdev);

	    return USBD_OK;
	} else {
		return USBD_FAIL;
	}
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

static uint8_t USBD_GS_CAN_ReceivePacket(USBD_HandleTypeDef *pdev)
{
	/* Suspend or Resume USB Out process */
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*)pdev->pClassData;
	if (hcan == 0) {
		return USBD_FAIL;
	} else {
		return USBD_LL_PrepareReceive(pdev, SLCAN_OUT_EP, hcan->rx_buf, CAN_DATA_MAX_PACKET_SIZE);
	}
}

uint8_t USBD_GS_CAN_Transmit(USBD_HandleTypeDef *pdev, uint8_t *buf, uint16_t len)
{
	USBD_GS_CAN_HandleTypeDef *hcan = (USBD_GS_CAN_HandleTypeDef*)pdev->pClassData;

	if (hcan->TxState == 0) {
		hcan->TxState = 1;
		hcan->TxLength = MIN(len, CAN_DATA_MAX_PACKET_SIZE);
		USBD_memset(hcan->tx_buf, 0, CAN_DATA_MAX_PACKET_SIZE);
		USBD_memcpy(hcan->tx_buf, buf, hcan->TxLength);
		USBD_LL_Transmit(pdev, SLCAN_IN_EP, hcan->tx_buf, (uint16_t)hcan->TxLength);
		return USBD_OK;
	} else {
		return USBD_BUSY;
	}

}
