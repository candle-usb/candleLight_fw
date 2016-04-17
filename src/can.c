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

#include "can.h"

void HAL_CAN_MspInit(CAN_HandleTypeDef* hcan)
{
	if(hcan->Instance==CAN)
	{
		__HAL_RCC_CAN1_CLK_ENABLE();

		GPIO_InitTypeDef itd;
		itd.Pin = GPIO_PIN_8|GPIO_PIN_9;
		itd.Mode = GPIO_MODE_AF_PP;
		itd.Pull = GPIO_NOPULL;
		itd.Speed = GPIO_SPEED_FREQ_HIGH;
		itd.Alternate = GPIO_AF4_CAN;
		HAL_GPIO_Init(GPIOB, &itd);
	}
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* hcan)
{
	if(hcan->Instance==CAN) {
		__HAL_RCC_CAN1_CLK_DISABLE();
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8|GPIO_PIN_9);
	}
}

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

bool can_is_rx_pending(CAN_HandleTypeDef *hcan)
{
	return ((hcan->Instance->RF0R & CAN_RF0R_FMP0)!=0);
}

bool can_receive(CAN_HandleTypeDef *hcan, struct gs_host_frame *rx_frame)
{
	bool retval = false;

	__HAL_LOCK(hcan);
	if (can_is_rx_pending(hcan)) {

		CAN_FIFOMailBox_TypeDef *fifo = &hcan->Instance->sFIFOMailBox[0];

		if (fifo->RIR &  CAN_RI0R_IDE) {
			rx_frame->can_id =  CAN_EFF_FLAG | ((fifo->RIR >> 3) & 0x1FFFFFFF);
		} else {
			rx_frame->can_id = (fifo->RIR >> 21) & 0x7FF;
		}

		if (fifo->RIR & CAN_RI0R_RTR)  {
			rx_frame->can_id |= CAN_RTR_FLAG;
		}

		rx_frame->can_dlc = fifo->RDTR & CAN_RDT0R_DLC;

		rx_frame->data[0] = (fifo->RDLR >>  0) & 0xFF;
		rx_frame->data[1] = (fifo->RDLR >>  8) & 0xFF;
		rx_frame->data[2] = (fifo->RDLR >> 16) & 0xFF;
		rx_frame->data[3] = (fifo->RDLR >> 24) & 0xFF;
		rx_frame->data[4] = (fifo->RDHR >>  0) & 0xFF;
		rx_frame->data[5] = (fifo->RDHR >>  8) & 0xFF;
		rx_frame->data[6] = (fifo->RDHR >> 16) & 0xFF;
		rx_frame->data[7] = (fifo->RDHR >> 24) & 0xFF;

		hcan->Instance->RF0R |= CAN_RF0R_RFOM0; // release FIFO

	    retval = true;
	}
	__HAL_UNLOCK(hcan);

	return retval;
}

static CAN_TxMailBox_TypeDef *can_find_free_mailbox(CAN_HandleTypeDef *hcan)
{
	uint32_t tsr = hcan->Instance->TSR;
	if ( tsr & CAN_TSR_TME0 ) {
		return &hcan->Instance->sTxMailBox[0];
	} else if ( tsr & CAN_TSR_TME1 ) {
		return &hcan->Instance->sTxMailBox[1];
	} else if ( tsr & CAN_TSR_TME2 ) {
		return &hcan->Instance->sTxMailBox[2];
	} else {
		return 0;
	}
}

bool can_send(CAN_HandleTypeDef *hcan, struct gs_host_frame *frame)
{
	bool retval;

	__HAL_LOCK(hcan);

	CAN_TxMailBox_TypeDef *mb = can_find_free_mailbox(hcan);
	if (mb != 0) {

		/* first, clear transmission request */
		mb->TIR &= CAN_TI0R_TXRQ;

		if (frame->can_id & CAN_EFF_FLAG) { // extended id
			mb->TIR = CAN_ID_EXT | (frame->can_id & 0x1FFFFFFF) << 3;
		} else {
			mb->TIR = (frame->can_id & 0x7FF) << 21;
		}

		if (frame->can_id & CAN_RTR_FLAG) {
			mb->TIR |= CAN_RTR_REMOTE;
		}

		mb->TDTR &= 0xFFFFFFF0;
		mb->TDTR |= frame->can_dlc & 0x0F;

		mb->TDLR =
			  ( frame->data[3] << 24 )
			| ( frame->data[2] << 16 )
			| ( frame->data[1] <<  8 )
			| ( frame->data[0] <<  0 );

		mb->TDHR =
			  ( frame->data[7] << 24 )
			| ( frame->data[6] << 16 )
			| ( frame->data[5] <<  8 )
			| ( frame->data[4] <<  0 );

		/* request transmission */
		mb->TIR |= CAN_TI0R_TXRQ;

		retval = true;
	} else {
		retval = false;
	}

	__HAL_UNLOCK(hcan);

	return retval;
}

uint32_t can_get_error_status(CAN_HandleTypeDef *hcan)
{
	return hcan->Instance->ESR;
}

bool can_parse_error_status(uint32_t err, struct gs_host_frame *frame)
{
	frame->can_id  = CAN_ERR_FLAG;
	frame->can_dlc = CAN_ERR_DLC;
	frame->data[1] = CAN_ERR_CRTL_UNSPEC;
	frame->data[2] = CAN_ERR_PROT_UNSPEC;
	frame->data[3] = CAN_ERR_PROT_LOC_UNSPEC;

	if ((err & 0x04) != 0) { /* bus off flag */
		frame->can_id |= CAN_ERR_BUSOFF;
	}

	uint8_t tx_error_cnt = (err>>16) & 0xFF;
	if (tx_error_cnt >= 96) { /* tx error warning level reached */
		frame->can_id  |= CAN_ERR_CRTL;
		frame->data[1] |= CAN_ERR_CRTL_TX_WARNING;
	}
	if (tx_error_cnt > 127) { /* tx error passive level reached */
		frame->can_id  |= CAN_ERR_CRTL;
		frame->data[1] |= CAN_ERR_CRTL_TX_PASSIVE;
	}

	uint8_t rx_error_cnt = (err>>24) & 0xFF;
	if (rx_error_cnt >= 96) { /* rx error warning level reached */
		frame->can_id  |= CAN_ERR_CRTL;
		frame->data[1] |= CAN_ERR_CRTL_RX_WARNING;
	}
	if (rx_error_cnt > 127) { /* rx error passive level reached */
		frame->can_id  |= CAN_ERR_CRTL;
		frame->data[1] |= CAN_ERR_CRTL_RX_PASSIVE;
	}

	uint8_t lec = (err>>4) & 0x07;
	if (lec!=0) { /* protocol error */
		switch (lec) {
			case 0x01: /* stuff error */
				frame->can_id |= CAN_ERR_PROT;
				frame->data[2] |= CAN_ERR_PROT_STUFF;
				break;
			case 0x02: /* form error */
				frame->can_id |= CAN_ERR_PROT;
				frame->data[2] |= CAN_ERR_PROT_FORM;
				break;
			case 0x03: /* ack error */
				frame->can_id |= CAN_ERR_ACK;
				break;
			case 0x04: /* bit recessive error */
				frame->can_id |= CAN_ERR_PROT;
				frame->data[2] |= CAN_ERR_PROT_BIT1;
				break;
			case 0x05: /* bit dominant error */
				frame->can_id |= CAN_ERR_PROT;
				frame->data[2] |= CAN_ERR_PROT_BIT0;
				break;
			case 0x06: /* CRC error */
				frame->can_id |= CAN_ERR_PROT;
				frame->data[3] |= CAN_ERR_PROT_LOC_CRC_SEQ;
				break;
			default:
				break;
		}
	}

	return true;
}
