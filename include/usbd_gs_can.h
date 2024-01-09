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

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "can.h"
#include "compiler.h"
#include "config.h"
#include "gs_usb.h"
#include "led.h"
#include "list.h"
#include "usbd_def.h"

/* Define these here so they can be referenced in other files */

#define GS_CAN_EP0_BUF_SIZE \
	max5(sizeof(struct gs_host_config), \
		 sizeof(struct gs_device_bittiming), \
		 sizeof(struct gs_device_mode), \
		 sizeof(struct gs_identify_mode), \
		 sizeof(struct gs_device_termination_state))
#ifdef CONFIG_CANFD
#define CAN_DATA_MAX_PACKET_SIZE 64    /* Endpoint IN & OUT Packet size */
#else
#define CAN_DATA_MAX_PACKET_SIZE 32    /* Endpoint IN & OUT Packet size */
#endif
#define USB_CAN_CONFIG_DESC_SIZ	 50
#define USBD_GS_CAN_VENDOR_CODE	 0x20
#define DFU_INTERFACE_NUM		 1
#define DFU_INTERFACE_STR_INDEX	 0xE0

extern USBD_ClassTypeDef USBD_GS_CAN;

#ifdef CONFIG_CANFD
#define GS_HOST_FRAME_SIZE struct_size((struct gs_host_frame *)NULL, canfd_ts, 1)
#else
#define GS_HOST_FRAME_SIZE struct_size((struct gs_host_frame *)NULL, classic_can_ts, 1)
#endif

struct gs_host_frame_object {
	struct list_head list;
	union {
		uint8_t _buf[GS_HOST_FRAME_SIZE];
		struct gs_host_frame frame;
	};
};

typedef struct {
	uint8_t __aligned(4) ep0_buf[GS_CAN_EP0_BUF_SIZE];

	USBD_SetupReqTypedef last_setup_request;

	struct list_head list_frame_pool;
	struct list_head list_to_host;

	struct gs_host_frame_object *from_host_buf;
	struct gs_host_frame_object *to_host_buf;

	can_data_t channels[NUM_CAN_CHANNEL];

	bool dfu_detach_requested;

	bool timestamps_enabled;
	uint32_t sof_timestamp_us;

	bool pad_pkts_to_max_pkt_size;

	struct gs_host_frame_object msgbuf[CAN_QUEUE_SIZE];
} USBD_GS_CAN_HandleTypeDef __attribute__ ((aligned (4)));

#if defined(STM32F0)
# define USB_INTERFACE USB
# define USB_INTERRUPT USB_IRQn
#elif defined(STM32F4)
# define USB_INTERFACE USB_OTG_FS
# define USB_INTERRUPT OTG_FS_IRQn

// RX FIFO is defined in words, so divide bytes by 4
// RX FIFO size chosen according to reference manual RM0368 which suggests
// using (largest packet size / 4) + 1
# define USB_RX_FIFO_SIZE ((256U / 4U) + 1U)
#elif defined(STM32G0)
# define USB_INTERFACE	  USB_DRD_FS
# define USB_INTERRUPT	  USB_UCPD1_2_IRQn
#endif

uint8_t USBD_GS_CAN_Init(USBD_GS_CAN_HandleTypeDef *hcan, USBD_HandleTypeDef *pdev);
void USBD_GS_CAN_SuspendCallback(USBD_HandleTypeDef  *pdev);
void USBD_GS_CAN_ResumeCallback(USBD_HandleTypeDef  *pdev);
void USBD_GS_CAN_ReceiveFromHost(USBD_HandleTypeDef *pdev);
void USBD_GS_CAN_SendToHost(USBD_HandleTypeDef *pdev);
bool USBD_GS_CAN_CustomDeviceRequest(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
bool USBD_GS_CAN_CustomInterfaceRequest(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);

bool USBD_GS_CAN_DfuDetachRequested(USBD_HandleTypeDef *pdev);
