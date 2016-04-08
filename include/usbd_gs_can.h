#pragma once

#include  "usbd_def.h"


extern USBD_ClassTypeDef  USBD_GS_CAN;
uint8_t USBD_GS_CAN_Transmit(USBD_HandleTypeDef *pdev, uint8_t *buf, uint16_t len);
