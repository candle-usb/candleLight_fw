#include "can.h"

void can_init(CAN_HandleTypeDef *hcan, CAN_TypeDef *instance)
{
	hcan->Instance = instance;
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
	hcan->Init.TXFP = ENABLE;
}

void can_set_bittiming(CAN_HandleTypeDef *hcan, uint16_t brp, uint8_t phase_seg1, uint8_t phase_seg2, uint8_t sjw)
{
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
}

void can_enable(CAN_HandleTypeDef *hcan, bool loop_back, bool listen_only, bool one_shot)
{
	hcan->Init.Mode = 0;
	if (loop_back) {
		hcan->Init.Mode |= CAN_MODE_LOOPBACK;
	}
	if (listen_only) {
		hcan->Init.Mode |= CAN_MODE_SILENT;
	}

	hcan->Init.NART = one_shot ? ENABLE : DISABLE;
	HAL_CAN_Init(hcan);

	CAN_FilterConfTypeDef fd;
	fd.FilterNumber = 0;
	fd.FilterMode = CAN_FILTERMODE_IDMASK;
	fd.FilterScale = CAN_FILTERSCALE_32BIT;
	fd.BankNumber = 0;
	fd.FilterIdLow = 0;
	fd.FilterIdHigh = 0;
	fd.FilterMaskIdLow = 0;
	fd.FilterMaskIdHigh = 0;
	fd.FilterActivation = ENABLE;
	fd.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	HAL_CAN_ConfigFilter(hcan, &fd);
}

void can_disable(CAN_HandleTypeDef *hcan)
{
	HAL_CAN_DeInit(hcan);
}

bool can_send(CAN_HandleTypeDef *hcan, CanTxMsgTypeDef *tx_msg, uint32_t timeout)
{
	hcan->pTxMsg = tx_msg;
	return HAL_CAN_Transmit(hcan, timeout) == HAL_OK;
}

bool can_receive(CAN_HandleTypeDef *hcan, CanRxMsgTypeDef *rx_msg, uint32_t timeout)
{
	hcan->pRxMsg = rx_msg;
	return HAL_CAN_Receive(hcan, CAN_FIFO0, timeout) == HAL_OK;
}

bool can_is_rx_pending(CAN_HandleTypeDef *hcan)
{
	return (__HAL_CAN_MSG_PENDING(hcan, CAN_FIFO0) > 0);
}
