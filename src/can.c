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

#include <string.h>
#include "can.h"
#include "config.h"
#include "gs_usb.h"
#include "hal_include.h"


#if defined(FDCAN1)
uint8_t can_rx_data_buff[64];
uint8_t can_tx_data_buff[64];
#endif

#if !defined(FDCAN1)
// The STM32F0 only has one CAN interface, define it as CAN1 as
// well, so it doesn't need to be handled separately.
#if !defined(CAN1) && defined(CAN)
#define CAN1 CAN
#endif

// Completely reset the CAN pheriperal, including bus-state and error counters
static void rcc_reset(CAN_TypeDef *instance)
{
#ifdef CAN1
	if (instance == CAN1) {
		__HAL_RCC_CAN1_FORCE_RESET();
		__HAL_RCC_CAN1_RELEASE_RESET();
	}
#endif

#ifdef CAN2
	if (instance == CAN2) {
		__HAL_RCC_CAN2_FORCE_RESET();
		__HAL_RCC_CAN2_RELEASE_RESET();
	}
#endif
}
#endif

#if defined(FDCAN1)
void can_init(can_data_t *hcan, FDCAN_GlobalTypeDef *instance)
#else
void can_init(can_data_t *hcan, CAN_TypeDef *instance)
#endif
{

	GPIO_InitTypeDef itd;
#if defined(STM32F0)
	__HAL_RCC_CAN1_CLK_ENABLE();
	itd.Pin = GPIO_PIN_8|GPIO_PIN_9;
	itd.Mode = GPIO_MODE_AF_PP;
	itd.Pull = GPIO_NOPULL;
	itd.Speed = GPIO_SPEED_FREQ_HIGH;
	itd.Alternate = GPIO_AF4_CAN;
	HAL_GPIO_Init(GPIOB, &itd);
#elif defined(STM32F4)
	__HAL_RCC_CAN1_CLK_ENABLE();
	itd.Pin = GPIO_PIN_0|GPIO_PIN_1;
	itd.Mode = GPIO_MODE_AF_PP;
	itd.Pull = GPIO_NOPULL;
	itd.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	itd.Alternate = GPIO_AF9_CAN1;
	HAL_GPIO_Init(GPIOD, &itd);
#elif defined(STM32G0)
	RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
	PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PCLK1;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
	__HAL_RCC_FDCAN_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	itd.Pin = GPIO_PIN_9|GPIO_PIN_8;
	itd.Mode = GPIO_MODE_AF_PP;
	itd.Pull = GPIO_NOPULL;
	itd.Speed = GPIO_SPEED_FREQ_LOW;
	itd.Alternate = GPIO_AF3_FDCAN1;
	HAL_GPIO_Init(GPIOB, &itd);
#endif /* STM32G0 */

	hcan->channel.Instance = instance;

	/* all values for the bxCAN init are -1 and shifted */
#if defined(STM32F0)
	hcan->channel.Init.SyncJumpWidth = ((1)-1) << CAN_BTR_SJW_Pos;
	hcan->channel.Init.Prescaler = ((6)-1);
	hcan->channel.Init.TimeSeg1 = ((13)-1) << CAN_BTR_TS1_Pos;
	hcan->channel.Init.TimeSeg2 = ((2)-1) << CAN_BTR_TS2_Pos;
#elif defined(STM32F4)
	hcan->channel.Init.SyncJumpWidth = ((1)-1) << CAN_BTR_SJW_Pos;
	hcan->channel.Init.Prescaler = ((6)-1);
	hcan->channel.Init.TimeSeg1 = ((12)-1) << CAN_BTR_TS1_Pos;
	hcan->channel.Init.TimeSeg2 = ((1)-1) << CAN_BTR_TS2_Pos;
#elif defined(STM32G0)
	hcan->channel.Instance = instance;
	hcan->channel.Init.ClockDivider = FDCAN_CLOCK_DIV1;
	hcan->channel.Init.FrameFormat = FDCAN_FRAME_FD_BRS;
	hcan->channel.Init.Mode = FDCAN_MODE_NORMAL;
	hcan->channel.Init.AutoRetransmission = DISABLE;
	hcan->channel.Init.TransmitPause = DISABLE;
	hcan->channel.Init.ProtocolException = ENABLE;
	hcan->channel.Init.NominalPrescaler = 8;
	hcan->channel.Init.NominalSyncJumpWidth = 1;
	hcan->channel.Init.NominalTimeSeg1 = 13;
	hcan->channel.Init.NominalTimeSeg2 = 2;
	hcan->channel.Init.DataPrescaler = 2;
	hcan->channel.Init.DataSyncJumpWidth = 4;
	hcan->channel.Init.DataTimeSeg1 = 15;
	hcan->channel.Init.DataTimeSeg2 = 4;
	hcan->channel.Init.StdFiltersNbr = 0;
	hcan->channel.Init.ExtFiltersNbr = 0;
	hcan->channel.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
	HAL_FDCAN_Init(&hcan->channel);
#endif
}

bool can_set_bittiming(can_data_t *hcan, uint16_t brp, uint8_t phase_seg1, uint8_t phase_seg2, uint8_t sjw)
{
	if (  (brp>0) && (brp<=1024)
	   && (phase_seg1>0) && (phase_seg1<=16)
	   && (phase_seg2>0) && (phase_seg2<=8)
	   && (sjw>0) && (sjw<=4)
		  ) {
#if defined(FDCAN1)
		hcan->channel.Init.NominalSyncJumpWidth = sjw;
		hcan->channel.Init.NominalTimeSeg1 = phase_seg1;
		hcan->channel.Init.NominalTimeSeg2 = phase_seg2;
		hcan->channel.Init.NominalPrescaler = brp;
#else
		hcan->channel.Init.SyncJumpWidth = (sjw-1) << CAN_BTR_SJW_Pos;
		hcan->channel.Init.TimeSeg1 = (phase_seg1-1) << CAN_BTR_TS1_Pos;
		hcan->channel.Init.TimeSeg2 = (phase_seg2-1) << CAN_BTR_TS2_Pos;
		hcan->channel.Init.Prescaler = (brp-1);
#endif
		return true;
	} else {
		return false;
	}
}

#if defined(FDCAN1)
bool can_set_data_bittiming(can_data_t *hcan, uint16_t brp, uint8_t phase_seg1, uint8_t phase_seg2, uint8_t sjw)
{
	if (  (brp>0) && (brp<=1024)
	   && (phase_seg1>0) && (phase_seg1<=16)
	   && (phase_seg2>0) && (phase_seg2<=8)
	   && (sjw>0) && (sjw<=4)
		  ) {

		hcan->channel.Init.DataSyncJumpWidth = sjw;
		hcan->channel.Init.DataTimeSeg1 = phase_seg1;
		hcan->channel.Init.DataTimeSeg2 = phase_seg2;
		hcan->channel.Init.DataPrescaler = brp;
		return true;
	} else {
		return false;
	}
}
#endif

#if defined(FDCAN1)
void can_enable(can_data_t *hcan, bool loop_back, bool listen_only, bool one_shot, bool can_mode_fd)
{
	hcan->channel.Init.AutoRetransmission = one_shot ? DISABLE : ENABLE;
	if (loop_back && listen_only) hcan->channel.Init.Mode = FDCAN_MODE_INTERNAL_LOOPBACK;
	else if (loop_back) hcan->channel.Init.Mode = FDCAN_MODE_EXTERNAL_LOOPBACK;
	else if (listen_only) hcan->channel.Init.Mode = FDCAN_MODE_BUS_MONITORING;
	else hcan->channel.Init.Mode = FDCAN_MODE_NORMAL;
	hcan->channel.Init.FrameFormat = can_mode_fd ? FDCAN_FRAME_FD_BRS : FDCAN_FRAME_CLASSIC;

	HAL_FDCAN_Init(&hcan->channel);

	/* Configure reception filter to Rx FIFO 0 on both FDCAN instances */
	FDCAN_FilterTypeDef sFilterConfig;
	sFilterConfig.IdType = FDCAN_STANDARD_ID;
	sFilterConfig.FilterIndex = 0;
	sFilterConfig.FilterType = FDCAN_FILTER_RANGE;
	sFilterConfig.FilterConfig = FDCAN_FILTER_DISABLE;
	sFilterConfig.FilterID1 = 0x000;
	sFilterConfig.FilterID2 = 0x7FF;

	HAL_FDCAN_ConfigFilter(&hcan->channel, &sFilterConfig);

	/* Configure global filter on both FDCAN instances:
		 Filter all remote frames with STD and EXT ID
		 Reject non matching frames with STD ID and EXT ID */
	HAL_FDCAN_ConfigGlobalFilter(&hcan->channel, FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE);

	// Start CAN using HAL
	HAL_FDCAN_Start(&hcan->channel);
#else
void can_enable(can_data_t *hcan, bool loop_back, bool listen_only, bool one_shot)
{
	CAN_TypeDef *can = hcan->channel.Instance;

	uint32_t mcr = CAN_MCR_INRQ
				   | CAN_MCR_ABOM
				   | CAN_MCR_TXFP
				   | (one_shot ? CAN_MCR_NART : 0);

	uint32_t btr = (uint32_t)(hcan->channel.Init.SyncJumpWidth
							  | hcan->channel.Init.TimeSeg1
							  | hcan->channel.Init.TimeSeg2
							  | hcan->channel.Init.Prescaler
							  | (loop_back ? CAN_MODE_LOOPBACK : 0)
							  | (listen_only ? CAN_MODE_SILENT : 0));


	// Reset CAN peripheral
	can->MCR |= CAN_MCR_RESET;
	while ((can->MCR & CAN_MCR_RESET) != 0);                                                 // reset bit is set to zero after reset
	while ((can->MSR & CAN_MSR_SLAK) == 0);                                                // should be in sleep mode after reset

	// Completely reset while being of the bus
	rcc_reset(can);

	can->MCR |= CAN_MCR_INRQ;
	while ((can->MSR & CAN_MSR_INAK) == 0);

	can->MCR = mcr;
	can->BTR = btr;

	can->MCR &= ~CAN_MCR_INRQ;
	while ((can->MSR & CAN_MSR_INAK) != 0);

	uint32_t filter_bit = 0x00000001;
	can->FMR |= CAN_FMR_FINIT;
	can->FMR &= ~CAN_FMR_CAN2SB;
	can->FA1R &= ~filter_bit;        // disable filter
	can->FS1R |= filter_bit;         // set to single 32-bit filter mode
	can->FM1R &= ~filter_bit;        // set filter mask mode for filter 0
	can->sFilterRegister[0].FR1 = 0;     // filter ID = 0
	can->sFilterRegister[0].FR2 = 0;     // filter Mask = 0
	can->FFA1R &= ~filter_bit;       // assign filter 0 to FIFO 0
	can->FA1R |= filter_bit;         // enable filter
	can->FMR &= ~CAN_FMR_FINIT;
#endif

#ifdef nCANSTBY_Pin
	HAL_GPIO_WritePin(nCANSTBY_Port, nCANSTBY_Pin, nCANSTBY_Active_High == 0 ? GPIO_PIN_RESET : GPIO_PIN_RESET);
#endif
}

void can_disable(can_data_t *hcan)
{
#if defined(FDCAN1)
	HAL_FDCAN_Stop(&hcan->channel);
#else
	CAN_TypeDef *can = hcan->channel.Instance;
	can->MCR |= CAN_MCR_INRQ;     // send can controller into initialization mode
#endif
#ifdef nCANSTBY_Pin
	HAL_GPIO_WritePin(nCANSTBY_Port, nCANSTBY_Pin, nCANSTBY_Active_High == 0 ? GPIO_PIN_RESET : GPIO_PIN_RESET);
#endif
}

bool can_is_enabled(can_data_t *hcan)
{
#if defined(FDCAN1)
	return hcan->channel.State == HAL_FDCAN_STATE_BUSY;
#else
	CAN_TypeDef *can = hcan->channel.Instance;
	return (can->MCR & CAN_MCR_INRQ) == 0;
#endif
}

bool can_is_rx_pending(can_data_t *hcan)
{
#if defined(FDCAN1)
	return (HAL_FDCAN_GetRxFifoFillLevel(&hcan->channel, FDCAN_RX_FIFO0) >= 1);
#else
	CAN_TypeDef *can = hcan->channel.Instance;
	return ((can->RF0R & CAN_RF0R_FMP0) != 0);
#endif
}

bool can_receive(can_data_t *hcan, struct GS_HOST_FRAME *rx_frame)
{
#if defined(FDCAN1)
	FDCAN_RxHeaderTypeDef RxHeader;

	if (HAL_FDCAN_GetRxMessage(&hcan->channel, FDCAN_RX_FIFO0, &RxHeader, can_rx_data_buff) != HAL_OK) {
		return false;
	}

	rx_frame->channel = 0;
	rx_frame->flags = 0;
	
	rx_frame->can_id = RxHeader.Identifier;

	if (RxHeader.IdType == FDCAN_EXTENDED_ID) {
		rx_frame->can_id |= CAN_EFF_FLAG;
	}

	if (RxHeader.RxFrameType == FDCAN_REMOTE_FRAME) {
		rx_frame->can_id |= CAN_RTR_FLAG;
	}

	rx_frame->can_dlc = (RxHeader.DataLength & 0x000F0000) >> 16;

	if (RxHeader.FDFormat == FDCAN_FD_CAN) {
		/* this is a CAN-FD frame */
		rx_frame->flags = GS_CAN_FLAG_FD;
		if (RxHeader.BitRateSwitch == FDCAN_BRS_ON) {
			rx_frame->flags |= GS_CAN_FLAG_BRS;
		}
		memcpy(rx_frame->data, can_rx_data_buff, 64);
	}
	else {
		memcpy(rx_frame->data, can_rx_data_buff, 8);
	}

	return true;
#else
	CAN_TypeDef *can = hcan->channel.Instance;

	if (can_is_rx_pending(hcan)) {
		CAN_FIFOMailBox_TypeDef *fifo = &can->sFIFOMailBox[0];

		rx_frame->channel = 0;
		rx_frame->flags = 0;

		if (fifo->RIR &  CAN_RI0R_IDE) {
			rx_frame->can_id = CAN_EFF_FLAG | ((fifo->RIR >> 3) & 0x1FFFFFFF);
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

		can->RF0R |= CAN_RF0R_RFOM0;         // release FIFO

		return true;
	} else {
		return false;
	}
#endif
}

#if !defined(FDCAN1)
static CAN_TxMailBox_TypeDef *can_find_free_mailbox(can_data_t *hcan)
{
	CAN_TypeDef *can = hcan->channel.Instance;

	uint32_t tsr = can->TSR;
	if ( tsr & CAN_TSR_TME0 ) {
		return &can->sTxMailBox[0];
	} else if ( tsr & CAN_TSR_TME1 ) {
		return &can->sTxMailBox[1];
	} else if ( tsr & CAN_TSR_TME2 ) {
		return &can->sTxMailBox[2];
	} else {
		return 0;
	}
}
#endif

bool can_send(can_data_t *hcan, struct GS_HOST_FRAME *frame)
{
#if defined(FDCAN1)
	FDCAN_TxHeaderTypeDef TxHeader;

	TxHeader.DataLength = frame->can_dlc << 16;
	TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeader.MessageMarker = 0;

	TxHeader.TxFrameType = frame->can_id & CAN_RTR_FLAG ? FDCAN_REMOTE_FRAME : FDCAN_DATA_FRAME;


	if (frame->can_id & CAN_EFF_FLAG) {
		TxHeader.IdType = FDCAN_EXTENDED_ID;
		TxHeader.Identifier = frame->can_id & 0x1FFFFFFF;
	}
	else {
		TxHeader.IdType = FDCAN_STANDARD_ID;
		TxHeader.Identifier = frame->can_id & 0x7FF;
	}

	if (frame->flags & GS_CAN_FLAG_FD) {
		TxHeader.FDFormat = FDCAN_FD_CAN;
		if (frame->flags & GS_CAN_FLAG_BRS) {
			TxHeader.BitRateSwitch = FDCAN_BRS_ON;
		}
		else {
			TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
		}
		memcpy(can_tx_data_buff, frame->data, 64);
	}
	else {
		TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
		TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
		memcpy(can_tx_data_buff, frame->data, 8);
	}

	if (HAL_FDCAN_AddMessageToTxFifoQ(&hcan->channel, &TxHeader, can_tx_data_buff) != HAL_OK) {
		return false;
	}
	else {
		return true;
	}
#else
	CAN_TxMailBox_TypeDef *mb = can_find_free_mailbox(hcan);
	if (mb != 0) {

		/* first, clear transmission request */
		mb->TIR &= CAN_TI0R_TXRQ;

		if (frame->can_id & CAN_EFF_FLAG) {         // extended id
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

		return true;
	} else {
		return false;
	}
#endif
}

uint32_t can_get_error_status(can_data_t *hcan)
{
#if defined(FDCAN1)
	uint32_t err = hcan->channel.Instance->PSR;
	/* Write 7 to LEC so we know if it gets set to the same thing again */
	hcan->channel.Instance->PSR = 7;
#else
	CAN_TypeDef *can = hcan->channel.Instance;

	uint32_t err = can->ESR;

	/* Write 7 to LEC so we know if it gets set to the same thing again */
	can->ESR = 7<<4;

#endif
	return err;
}

static bool status_is_active(uint32_t err)
{
#if defined(FDCAN1)
	return !(err & (FDCAN_PSR_BO | FDCAN_PSR_EP));
#else
	return !(err & (CAN_ESR_BOFF | CAN_ESR_EPVF));
#endif

}

bool can_parse_error_status(uint32_t err, uint32_t last_err, can_data_t *hcan, struct GS_HOST_FRAME *frame)
{
	/* We build up the detailed error information at the same time as we decide
	 * whether there's anything worth sending. This variable tracks that final
	 * result. */
	bool should_send = false;
	(void) hcan;

	frame->echo_id = 0xFFFFFFFF;
	frame->can_id  = CAN_ERR_FLAG;
	frame->can_dlc = CAN_ERR_DLC;
	frame->data[0] = CAN_ERR_LOSTARB_UNSPEC;
	frame->data[1] = CAN_ERR_CRTL_UNSPEC;
	frame->data[2] = CAN_ERR_PROT_UNSPEC;
	frame->data[3] = CAN_ERR_PROT_LOC_UNSPEC;
	frame->data[4] = CAN_ERR_TRX_UNSPEC;
	frame->data[5] = 0;
	frame->data[6] = 0;
	frame->data[7] = 0;

	/* We transitioned from passive/bus-off to active, so report the edge. */
	if (!status_is_active(last_err) && status_is_active(err)) {
		frame->can_id |= CAN_ERR_CRTL;
		frame->data[1] |= CAN_ERR_CRTL_ACTIVE;
		should_send = true;
	}

#if defined (FDCAN1)
	if (err & FDCAN_PSR_BO) {
		if (!(last_err & FDCAN_PSR_BO)) {
			/* We transitioned to bus-off. */
			frame->can_id |= CAN_ERR_BUSOFF;
			should_send = true;
		}
	}
	/* The Linux sja1000 driver puts these counters here. Seems like as good a
	* place as any. */
	// TX error count
	frame->data[6] = ((hcan->channel.Instance->ECR & FDCAN_ECR_TEC) >> FDCAN_ECR_TEC_Pos);
	// RX error count
	frame->data[7] = ((hcan->channel.Instance->ECR & FDCAN_ECR_REC) >> FDCAN_ECR_REC_Pos);

	if (err & FDCAN_PSR_EP) {
		if (!(last_err & FDCAN_PSR_EP)) {
			frame->can_id |= CAN_ERR_CRTL;
			frame->data[1] |= CAN_ERR_CRTL_RX_PASSIVE | CAN_ERR_CRTL_TX_PASSIVE;
			should_send = true;
		}
	}
	else if (err & FDCAN_PSR_EW) {
		if (!(last_err & FDCAN_PSR_EW)) {
			frame->can_id |= CAN_ERR_CRTL;
			frame->data[1] |= CAN_ERR_CRTL_RX_WARNING | CAN_ERR_CRTL_TX_WARNING;
			should_send = true;
		}
	}

	uint8_t lec = err & FDCAN_PSR_LEC;
#else
	if (err & CAN_ESR_BOFF) {
		if (!(last_err & CAN_ESR_BOFF)) {
			/* We transitioned to bus-off. */
			frame->can_id |= CAN_ERR_BUSOFF;
			should_send = true;
		}
		// - tec (overflowed) / rec (looping, likely used for recessive counting)
		//   are not valid in the bus-off state.
		// - The warning flags remains set, error passive will cleared.
		// - LEC errors will be reported, while the device isn't even allowed to send.
		//
		// Hence only report bus-off, ignore everything else.
		return should_send;
	}

	uint8_t tx_error_cnt = (err>>16) & 0xFF;
	uint8_t rx_error_cnt = (err>>24) & 0xFF;
	/* The Linux sja1000 driver puts these counters here. Seems like as good a
	 * place as any. */
	frame->data[6] = tx_error_cnt;
	frame->data[7] = rx_error_cnt;

	if (err & CAN_ESR_EPVF) {
		if (!(last_err & CAN_ESR_EPVF)) {
			frame->can_id |= CAN_ERR_CRTL;
			frame->data[1] |= CAN_ERR_CRTL_RX_PASSIVE | CAN_ERR_CRTL_TX_PASSIVE;
			should_send = true;
		}
	} else if (err & CAN_ESR_EWGF) {
		if (!(last_err & CAN_ESR_EWGF)) {
			frame->can_id |= CAN_ERR_CRTL;
			frame->data[1] |= CAN_ERR_CRTL_RX_WARNING | CAN_ERR_CRTL_TX_WARNING;
			should_send = true;
		}
	}

	uint8_t lec = (err>>4) & 0x07;
#endif
	switch (lec) {
		case 0x01: /* stuff error */
			frame->can_id |= CAN_ERR_PROT;
			frame->data[2] |= CAN_ERR_PROT_STUFF;
			should_send = true;
			break;
		case 0x02: /* form error */
			frame->can_id |= CAN_ERR_PROT;
			frame->data[2] |= CAN_ERR_PROT_FORM;
			should_send = true;
			break;
		case 0x03: /* ack error */
			frame->can_id |= CAN_ERR_ACK;
			should_send = true;
			break;
		case 0x04: /* bit recessive error */
			frame->can_id |= CAN_ERR_PROT;
			frame->data[2] |= CAN_ERR_PROT_BIT1;
			should_send = true;
			break;
		case 0x05: /* bit dominant error */
			frame->can_id |= CAN_ERR_PROT;
			frame->data[2] |= CAN_ERR_PROT_BIT0;
			should_send = true;
			break;
		case 0x06: /* CRC error */
			frame->can_id |= CAN_ERR_PROT;
			frame->data[3] |= CAN_ERR_PROT_LOC_CRC_SEQ;
			should_send = true;
			break;
		default: /* 0=no error, 7=no change */
			break;
	}

	return should_send;
}
