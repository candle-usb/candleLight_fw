#pragma once

#include  "usbd_def.h"


extern USBD_ClassTypeDef USBD_GS_CAN;

void USBD_GS_CAN_SetChannel(USBD_HandleTypeDef *pdev, uint8_t channel, CAN_HandleTypeDef* handle);

void USBD_GS_CAN_SendFrameToHost(
	USBD_HandleTypeDef *pdev,
	uint32_t echo_id,
	uint32_t can_id,
	uint8_t dlc,
	uint8_t channel,
	uint8_t flags,
	uint8_t *data
);
