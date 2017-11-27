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
#include <usbd_def.h>
#include <queue.h>
#include <led.h>
#include <can.h>
#include <gs_usb.h>

/* Define these here so they can be referenced in other files */

#define CAN_DATA_MAX_PACKET_SIZE   32  /* Endpoint IN & OUT Packet size */
#define CAN_CMD_PACKET_SIZE        64  /* Control Endpoint Packet size */
#define USB_CAN_CONFIG_DESC_SIZ    50
#define NUM_CAN_CHANNEL             1
#define USBD_GS_CAN_VENDOR_CODE  0x20
#define DFU_INTERFACE_NUM           1
#define DFU_INTERFACE_STR_INDEX  0xE0

extern USBD_ClassTypeDef USBD_GS_CAN;

uint8_t USBD_GS_CAN_Init(USBD_HandleTypeDef *pdev, queue_t *q_frame_pool, queue_t *q_from_host, led_data_t *leds);
void USBD_GS_CAN_SetChannel(USBD_HandleTypeDef *pdev, uint8_t channel, can_data_t* handle);
bool USBD_GS_CAN_TxReady(USBD_HandleTypeDef *pdev);
uint8_t USBD_GS_CAN_PrepareReceive(USBD_HandleTypeDef *pdev);
bool USBD_GS_CAN_CustomDeviceRequest(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
bool USBD_GS_CAN_CustomInterfaceRequest(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);

bool USBD_GS_CAN_DfuDetachRequested(USBD_HandleTypeDef *pdev);
uint8_t USBD_GS_CAN_SendFrame(USBD_HandleTypeDef *pdev, struct gs_host_frame *frame);
uint8_t USBD_GS_CAN_Transmit(USBD_HandleTypeDef *pdev, uint8_t *buf, uint16_t len);
uint8_t USBD_GS_CAN_GetProtocolVersion(USBD_HandleTypeDef *pdev);
uint8_t USBD_GS_CAN_GetPadPacketsToMaxPacketSize(USBD_HandleTypeDef *pdev);
