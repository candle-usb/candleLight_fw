#include "usbd_gs_can.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"
#include "usbd_ioreq.h"

#define SLCAN_IN_EP              0x81  /* EP1 for data IN */
#define SLCAN_OUT_EP             0x01  /* EP1 for data OUT */
#define CAN_CMD_EP               0x82  /* EP2 for CDC commands */
#define CAN_DATA_MAX_PACKET_SIZE 64  /* Endpoint IN & OUT Packet size */
#define CAN_CMD_PACKET_SIZE      8   /* Control Endpoint Packet size */

#define USB_CAN_CONFIG_DESC_SIZ  67
#define CDC_GET_LINE_CODING      0x21

typedef struct {
	uint32_t RxLength;
	uint32_t TxLength;
	__IO uint32_t TxState;
	__IO uint32_t RxState;

	uint8_t CmdOpCode;
	uint8_t CmdLength;
	uint8_t slcan_str_index;
	uint8_t _dummy;

	uint8_t slcan_str[32];
	uint8_t cmd_buf[CAN_CMD_PACKET_SIZE];
	uint8_t rx_buf[CAN_DATA_MAX_PACKET_SIZE];
	uint8_t tx_buf[CAN_DATA_MAX_PACKET_SIZE];
} USBD_CAN_HandleTypeDef;

static uint8_t USBD_CAN_ReceivePacket(USBD_HandleTypeDef *pdev);

static uint8_t USBD_CAN_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_CAN_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_CAN_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_CAN_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_CAN_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_CAN_EP0_RxReady(USBD_HandleTypeDef *pdev);
static uint8_t *USBD_CAN_GetCfgDesc(uint16_t *len);
static uint8_t *USBD_CAN_GetDeviceQualifierDescriptor(uint16_t *length);


/* CAN interface class callbacks structure */
USBD_ClassTypeDef USBD_CAN = {
	USBD_CAN_Init,
	USBD_CAN_DeInit,
	USBD_CAN_Setup,
	NULL,                 /* EP0_TxSent, */
	USBD_CAN_EP0_RxReady,
	USBD_CAN_DataIn,
	USBD_CAN_DataOut,
	NULL,
	NULL,
	NULL,
	USBD_CAN_GetCfgDesc,
	USBD_CAN_GetCfgDesc,
	USBD_CAN_GetCfgDesc,
	USBD_CAN_GetDeviceQualifierDescriptor,
};


/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_CAN_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
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


/* USB CDC device Configuration Descriptor */
__ALIGN_BEGIN uint8_t USBD_CAN_CfgDesc[USB_CAN_CONFIG_DESC_SIZ] __ALIGN_END =
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
	0x02,   /* bInterfaceClass: Communication Interface Class */
	0x02,   /* bInterfaceSubClass: Abstract Control Model */
	0x01,   /* bInterfaceProtocol: Common AT commands */
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

static int8_t USBD_CAN_ControlReq(uint8_t cmd, uint8_t *pbuf, uint16_t len)
{
	(void) len;

    switch (cmd) {
		case CDC_GET_LINE_CODING:
			pbuf[0] = (uint8_t)(115200);
			pbuf[1] = (uint8_t)(115200 >> 8);
			pbuf[2] = (uint8_t)(115200 >> 16);
			pbuf[3] = (uint8_t)(115200 >> 24);
			pbuf[4] = 0; // stop bits (1)
			pbuf[5] = 0; // parity (none)
			pbuf[6] = 8; // number of bits (8)
			break;

		default:
			break;
    }

    return (USBD_OK);
}

static uint8_t USBD_CAN_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
	(void) cfgidx;
	uint8_t ret = 0;
  
	USBD_LL_OpenEP(pdev, SLCAN_IN_EP, USBD_EP_TYPE_BULK, CAN_DATA_MAX_PACKET_SIZE);
	USBD_LL_OpenEP(pdev, SLCAN_OUT_EP, USBD_EP_TYPE_BULK, CAN_DATA_MAX_PACKET_SIZE);
	USBD_LL_OpenEP(pdev, CAN_CMD_EP, USBD_EP_TYPE_INTR, CAN_CMD_PACKET_SIZE);

	USBD_CAN_HandleTypeDef *hcan = USBD_malloc(sizeof(USBD_CAN_HandleTypeDef));
  
	if(hcan == 0) {
		ret = 1;
	} else {
		USBD_memset(hcan, 0, sizeof(USBD_CAN_HandleTypeDef));
		pdev->pClassData = hcan;
		USBD_LL_PrepareReceive(pdev, SLCAN_OUT_EP, hcan->rx_buf, CAN_DATA_MAX_PACKET_SIZE);
	}
	return ret;
}

static uint8_t USBD_CAN_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
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

static uint8_t USBD_CAN_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{

	USBD_CAN_HandleTypeDef *hcan = (USBD_CAN_HandleTypeDef*) pdev->pClassData;
	static uint8_t ifalt = 0;

	switch (req->bmRequest & USB_REQ_TYPE_MASK) {

		case USB_REQ_TYPE_CLASS:
			if (req->wLength) {
				if (req->bmRequest & 0x80) {
					USBD_CAN_ControlReq(req->bRequest, hcan->cmd_buf, req->wLength);
					USBD_CtlSendData(pdev, hcan->cmd_buf, req->wLength);
				} else {
					hcan->CmdOpCode = req->bRequest;
					hcan->CmdLength = (uint8_t)req->wLength;
					USBD_CtlPrepareRx(pdev, hcan->cmd_buf, req->wLength);
				}

			} else {
				USBD_CAN_ControlReq(req->bRequest, (uint8_t*)req, 0);
			}
			break;


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

static uint8_t USBD_CAN_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum) {
	(void) epnum;

	if(pdev->pClassData != NULL) {
		USBD_CAN_HandleTypeDef *hcan = (USBD_CAN_HandleTypeDef*)pdev->pClassData;
		hcan->TxState = 0;
		return USBD_OK;
	} else {
		return USBD_FAIL;
	}
}

static uint8_t USBD_CAN_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum) {

	if (pdev->pClassData != NULL) {

		USBD_CAN_HandleTypeDef *hcan = (USBD_CAN_HandleTypeDef*)pdev->pClassData;
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
	    USBD_CAN_ReceivePacket(pdev);

	    return USBD_OK;
	} else {
		return USBD_FAIL;
	}
}

static uint8_t USBD_CAN_EP0_RxReady(USBD_HandleTypeDef *pdev) {

	if (pdev->pClassData != NULL) {

		USBD_CAN_HandleTypeDef *hcan = (USBD_CAN_HandleTypeDef*) pdev->pClassData;

		if (hcan->CmdOpCode != 0xFF) {
			USBD_CAN_ControlReq(hcan->CmdOpCode, hcan->cmd_buf, (uint16_t)hcan->RxLength);
			hcan->CmdOpCode = 0xFF;
		}

	}

	return USBD_OK;
}

static uint8_t *USBD_CAN_GetCfgDesc(uint16_t *len)
{
	*len = sizeof(USBD_CAN_CfgDesc);
	return USBD_CAN_CfgDesc;
}

uint8_t *USBD_CAN_GetDeviceQualifierDescriptor(uint16_t *length)
{
	*length = sizeof(USBD_CAN_DeviceQualifierDesc);
	return USBD_CAN_DeviceQualifierDesc;
}

static uint8_t USBD_CAN_ReceivePacket(USBD_HandleTypeDef *pdev)
{
	/* Suspend or Resume USB Out process */
	USBD_CAN_HandleTypeDef *hcan = (USBD_CAN_HandleTypeDef*)pdev->pClassData;
	if (hcan == 0) {
		return USBD_FAIL;
	} else {
		return USBD_LL_PrepareReceive(pdev, SLCAN_OUT_EP, hcan->rx_buf, CAN_DATA_MAX_PACKET_SIZE);
	}
}

uint8_t USBD_CAN_Transmit(USBD_HandleTypeDef *pdev, uint8_t *buf, uint16_t len)
{
	USBD_CAN_HandleTypeDef *hcan = (USBD_CAN_HandleTypeDef*)pdev->pClassData;

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
