#pragma once

#include  "usbd_def.h"


extern USBD_ClassTypeDef  USBD_GS_CAN;

void USBD_GS_CAN_MessageReceived(
	USBD_HandleTypeDef *pdev,
	uint32_t echo_id,
	uint32_t can_id,
	uint8_t dlc,
	uint8_t channel,
	uint8_t flags,
	uint8_t *data
);
