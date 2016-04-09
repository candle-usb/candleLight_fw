#include "can.h"

void can_init(CAN_HandleTypeDef *hcan, CAN_TypeDef *instance)
{
/*	hcan->Instance = instance;
	hcan->Init.Prescaler = 6-1;
	hcan->Init.Mode = CAN_MODE_SILENT;
	hcan->Init.SJW = CAN_SJW_1TQ;
	hcan->Init.BS1 = CAN_BS1_13TQ;
	hcan->Init.BS2 = CAN_BS2_2TQ;
	hcan->Init.TTCM = DISABLE;
	hcan->Init.ABOM = DISABLE;
	hcan->Init.AWUM = DISABLE;
	hcan->Init.NART = DISABLE;
	hcan->Init.RFLM = DISABLE;
	hcan->Init.TXFP = DISABLE;
	HAL_CAN_Init(hcan); */
}

void can_set_bittiming(CAN_HandleTypeDef *hcan, uint16_t brp, uint8_t phase_seg1, uint8_t phase_seg2, uint8_t sjw)
{
/*	HAL_CAN_DeInit(hcan);

	hcan->Init.Prescaler = brp & 0x3FF;

	if ((phase_seg1>0) && (phase_seg1<17)) {
		hcan->Init.BS1 = (phase_seg1-1)<<16;
	}

	if ((phase_seg2>0) && (phase_seg2<9)) {
		hcan->Init.BS2 = (phase_seg2-1)<<20;
	}

	if ((sjw>0) && (sjw<5)) {
		hcan->Init.SJW = (sjw-1)<<24;
	}

	HAL_CAN_Init(hcan); */
}

void can_enable(CAN_HandleTypeDef *hcan, bool loop_back, bool listen_only, bool one_shot)
{
/*	hcan->Init.Mode = 0;
	if (loop_back) {
		hcan->Init.Mode |= CAN_MODE_LOOPBACK;
	}
	if (listen_only) {
		hcan->Init.Mode |= CAN_MODE_SILENT;
	}

	hcan->Init.NART = one_shot ? ENABLE : DISABLE;
	HAL_CAN_Init(hcan); */
}

void can_disable(CAN_HandleTypeDef *hcan)
{
	//HAL_CAN_DeInit(hcan);
}
