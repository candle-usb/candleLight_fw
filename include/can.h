#pragma once

#include <stdbool.h>
#include "stm32f0xx_hal.h"

void can_init(CAN_HandleTypeDef *hcan, CAN_TypeDef *instance);
void can_set_bittiming(CAN_HandleTypeDef *hcan, uint16_t brp, uint8_t phase_seg1, uint8_t phase_seg2, uint8_t sjw);
void can_enable(CAN_HandleTypeDef *hcan, bool loop_back, bool listen_only, bool one_shot);
void can_disable(CAN_HandleTypeDef *hcan);
