#pragma once

#include <stdbool.h>
#include <usbd_def.h>
#include <queue.h>

extern USBD_ClassTypeDef USBD_GS_CAN;

uint8_t USBD_GS_CAN_Init(USBD_HandleTypeDef *pdev, queue_t *q_frame_pool, queue_t *q_from_host);
void USBD_GS_CAN_SetChannel(USBD_HandleTypeDef *pdev, uint8_t channel, CAN_HandleTypeDef* handle);
bool USBD_GS_CAN_TxReady(USBD_HandleTypeDef *pdev);
uint8_t USBD_GS_CAN_Transmit(USBD_HandleTypeDef *pdev, uint8_t *buf, uint16_t len);
