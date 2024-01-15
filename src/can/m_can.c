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

#include "board.h"
#include "can.h"
#include "hal_include.h"
#include "timer.h"

// bit timing constraints
const struct gs_device_bt_const CAN_btconst = {
	.feature =
		GS_CAN_FEATURE_LISTEN_ONLY |
		GS_CAN_FEATURE_LOOP_BACK |
		GS_CAN_FEATURE_HW_TIMESTAMP |
		GS_CAN_FEATURE_IDENTIFY |
		GS_CAN_FEATURE_PAD_PKTS_TO_MAX_PKT_SIZE |
		(IS_ENABLED(CONFIG_CANFD) ?
		 GS_CAN_FEATURE_FD | GS_CAN_FEATURE_BT_CONST_EXT : 0)
#ifdef TERM_Pin
		| GS_CAN_FEATURE_TERMINATION
#endif
	,
	.fclk_can = CAN_CLOCK_SPEED,
	.tseg1_min = 1,
	.tseg1_max = 256,
	.tseg2_min = 1,
	.tseg2_max = 128,
	.sjw_max = 128,
	.brp_min = 1,
	.brp_max = 512,
	.brp_inc = 1,
};

const struct gs_device_bt_const_extended CAN_btconst_ext = {
	.device_bt_const = CAN_btconst,

	.dtseg1_min = 1,
	.dtseg1_max = 32,
	.dtseg2_min = 1,
	.dtseg2_max = 16,
	.dsjw_max = 16,
	.dbrp_min = 1,
	.dbrp_max = 32,
	.dbrp_inc = 1,
};

void can_init(can_data_t *channel, FDCAN_GlobalTypeDef *instance)
{
	channel->channel.Instance = instance;
	channel->channel.Init.ClockDivider = FDCAN_CLOCK_DIV1;
	channel->channel.Init.FrameFormat = FDCAN_FRAME_FD_BRS;
	channel->channel.Init.Mode = FDCAN_MODE_NORMAL;
	channel->channel.Init.AutoRetransmission = ENABLE;
	channel->channel.Init.TransmitPause = DISABLE;
	channel->channel.Init.ProtocolException = ENABLE;
	channel->channel.Init.NominalPrescaler = 8;
	channel->channel.Init.NominalSyncJumpWidth = 1;
	channel->channel.Init.NominalTimeSeg1 = 13;
	channel->channel.Init.NominalTimeSeg2 = 2;
	channel->channel.Init.DataPrescaler = 2;
	channel->channel.Init.DataSyncJumpWidth = 4;
	channel->channel.Init.DataTimeSeg1 = 15;
	channel->channel.Init.DataTimeSeg2 = 4;
	channel->channel.Init.StdFiltersNbr = 0;
	channel->channel.Init.ExtFiltersNbr = 0;
	channel->channel.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
}

void can_set_bittiming(can_data_t *channel, const struct gs_device_bittiming *timing)
{
	const uint8_t tseg1 = timing->prop_seg + timing->phase_seg1;

	channel->channel.Init.NominalSyncJumpWidth = timing->sjw;
	channel->channel.Init.NominalTimeSeg1 = tseg1;
	channel->channel.Init.NominalTimeSeg2 = timing->phase_seg2;
	channel->channel.Init.NominalPrescaler = timing->brp;
}

void can_set_data_bittiming(can_data_t *channel, const struct gs_device_bittiming *timing)
{
	const uint8_t tseg1 = timing->prop_seg + timing->phase_seg1;

	channel->channel.Init.DataSyncJumpWidth = timing->sjw;
	channel->channel.Init.DataTimeSeg1 = tseg1;
	channel->channel.Init.DataTimeSeg2 = timing->phase_seg2;
	channel->channel.Init.DataPrescaler = timing->brp;
}

void can_enable(can_data_t *channel, uint32_t mode)
{
	if (mode & GS_CAN_MODE_ONE_SHOT) {
		channel->channel.Init.AutoRetransmission = DISABLE;
	} else {
		channel->channel.Init.AutoRetransmission = ENABLE;
	}

	if ((mode & (GS_CAN_MODE_LISTEN_ONLY | GS_CAN_MODE_LOOP_BACK)) ==
		(GS_CAN_MODE_LISTEN_ONLY | GS_CAN_MODE_LOOP_BACK)) {
		channel->channel.Init.Mode = FDCAN_MODE_INTERNAL_LOOPBACK;
	} else if (mode & GS_CAN_MODE_LISTEN_ONLY) {
		channel->channel.Init.Mode = FDCAN_MODE_BUS_MONITORING;
	} else if (mode & GS_CAN_MODE_LOOP_BACK) {
		channel->channel.Init.Mode = FDCAN_MODE_EXTERNAL_LOOPBACK;
	} else {
		channel->channel.Init.Mode = FDCAN_MODE_NORMAL;
	}

	if (mode & GS_CAN_MODE_FD) {
		channel->channel.Init.FrameFormat = FDCAN_FRAME_FD_BRS;
	} else {
		channel->channel.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
	}

	HAL_FDCAN_Init(&channel->channel);
	HAL_FDCAN_EnableISOMode(&channel->channel);

	/* Enable automatic transceiver delay compensation */
	HAL_FDCAN_ConfigTxDelayCompensation(&channel->channel,
										channel->channel.Init.DataTimeSeg1 * channel->channel.Init.DataPrescaler,
										0U);
	HAL_FDCAN_EnableTxDelayCompensation(&channel->channel);

	/* Configure reception filter to Rx FIFO 0 on both FDCAN instances */
	FDCAN_FilterTypeDef sFilterConfig = {
		.IdType = FDCAN_STANDARD_ID,
		.FilterIndex = 0,
		.FilterType = FDCAN_FILTER_RANGE,
		.FilterConfig = FDCAN_FILTER_DISABLE,
		.FilterID1 = 0x000,
		.FilterID2 = 0x7FF,
	};

	HAL_FDCAN_ConfigFilter(&channel->channel, &sFilterConfig);

	/*
	 * Configure global filter on both FDCAN instances:
	 * Filter all remote frames with STD and EXT ID
	 * Reject non matching frames with STD ID and EXT ID
	 */
	HAL_FDCAN_ConfigGlobalFilter(&channel->channel,
								 FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_ACCEPT_IN_RX_FIFO0,
								 FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE);

	if (config.phy_power_set) config.phy_power_set(channel, true);

	// Start CAN using HAL
	HAL_FDCAN_Start(&channel->channel);
}

void can_disable(can_data_t *channel)
{
	HAL_FDCAN_Stop(&channel->channel);

	if (config.phy_power_set) config.phy_power_set(channel, false);
}

bool can_is_enabled(can_data_t *channel)
{
	return channel->channel.State == HAL_FDCAN_STATE_BUSY;
}

bool can_receive(can_data_t *channel, struct gs_host_frame *rx_frame)
{
	FDCAN_RxHeaderTypeDef RxHeader;
	uint32_t timestamp_us = timer_get();

	if (HAL_FDCAN_GetRxMessage(&channel->channel, FDCAN_RX_FIFO0, &RxHeader, rx_frame->canfd->data) != HAL_OK) {
		return false;
	}

	rx_frame->channel = channel->nr;
	rx_frame->flags = 0;
	rx_frame->can_id = RxHeader.Identifier;

	if (RxHeader.IdType == FDCAN_EXTENDED_ID) {
		rx_frame->can_id |= CAN_EFF_FLAG;
	}

	if (RxHeader.RxFrameType == FDCAN_REMOTE_FRAME) {
		rx_frame->can_id |= CAN_RTR_FLAG;
	}

	#if defined(STM32G0)
	// G0 driver expects DataLength field to be pre-shifted into place for the controller
	rx_frame->can_dlc = RxHeader.DataLength >> 16;
	#else
	// G4 driver accepts the length as is and shifts internally where required
	rx_frame->can_dlc = RxHeader.DataLength;
	#endif
	rx_frame->can_dlc &= 0x0F;

	if (RxHeader.FDFormat == FDCAN_FD_CAN) {
		rx_frame->canfd_ts->timestamp_us = timestamp_us;

		/* this is a CAN-FD frame */
		rx_frame->flags = GS_CAN_FLAG_FD;
		if (RxHeader.BitRateSwitch == FDCAN_BRS_ON) {
			rx_frame->flags |= GS_CAN_FLAG_BRS;
		}

		if (RxHeader.ErrorStateIndicator == FDCAN_ESI_PASSIVE) {
			rx_frame->flags |= GS_CAN_FLAG_ESI;
		}
	} else {
		rx_frame->classic_can_ts->timestamp_us = timestamp_us;
	}

	return true;
}

bool can_is_rx_pending(can_data_t *channel)
{
	return (HAL_FDCAN_GetRxFifoFillLevel(&channel->channel, FDCAN_RX_FIFO0) >= 1);
}

bool can_send(can_data_t *channel, struct gs_host_frame *frame)
{
	FDCAN_TxHeaderTypeDef TxHeader = {
		#if defined(STM32G0)
		// G0 driver expects DataLength field to be pre-shifted into place for the controller
		.DataLength = frame->can_dlc << 16,
		#else
		// G4 driver accepts the length as is and shifts internally where required
		.DataLength = frame->can_dlc,
		#endif
		.TxEventFifoControl = FDCAN_NO_TX_EVENTS,
	};

	TxHeader.TxFrameType =
		frame->can_id & CAN_RTR_FLAG ? FDCAN_REMOTE_FRAME : FDCAN_DATA_FRAME;

	if (frame->can_id & CAN_EFF_FLAG) {
		TxHeader.IdType = FDCAN_EXTENDED_ID;
		TxHeader.Identifier = frame->can_id & 0x1FFFFFFF;
	} else {
		TxHeader.IdType = FDCAN_STANDARD_ID;
		TxHeader.Identifier = frame->can_id & 0x7FF;
	}

	if (frame->flags & GS_CAN_FLAG_FD) {
		TxHeader.FDFormat = FDCAN_FD_CAN;
		if (frame->flags & GS_CAN_FLAG_BRS) {
			TxHeader.BitRateSwitch = FDCAN_BRS_ON;
		} else {
			TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
		}

		if (frame->flags & GS_CAN_FLAG_ESI) {
			TxHeader.ErrorStateIndicator = FDCAN_ESI_PASSIVE;
		} else {
			TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
		}
	} else {
		TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
		TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
	}

	if (HAL_FDCAN_AddMessageToTxFifoQ(&channel->channel, &TxHeader, frame->canfd->data) != HAL_OK) {
		return false;
	}

	return true;
}

uint32_t can_get_error_status(can_data_t *channel)
{
	uint32_t err = channel->channel.Instance->PSR;

	/* No need to reset anything as the hardware automatically resets LEC
	 * (last error code) on read
	 */

	return err;
}

void can_manage_bus_off_recovery(can_data_t *channel, uint32_t err)
{
	/*
	 * bxcan manages recovery from bus-off automatically (if the ABOM bit is set), the m_can module does not
	 * If the module goes bus-off the init bit is automatically set by the hardware, placing the module back into configuration
	 * mode. Therefore if the PSR indicates we've gone bus off, but the module reports it's enabled, clear the init bit to start
	 * the bus-off recovery sequence.
	 */
	if (	(0U != (err & FDCAN_PSR_BO))
		 && can_is_enabled(channel)
		 && (0U != (channel->channel.Instance->CCCR & FDCAN_CCCR_INIT))
	   )
	{
		/* Clear init bit to re-start module */
		CLEAR_BIT(channel->channel.Instance->CCCR, FDCAN_CCCR_INIT);

		/* Wait for the bit to clear (it takes a while to cross clock domains) */
		for (uint32_t uiCounter = 0; uiCounter < 100U; uiCounter++)
		{
			if (0U == (channel->channel.Instance->CCCR & FDCAN_CCCR_INIT))
			{
				/* Init complete, stop waiting */
				break;
			}
		}
	}
}

bool can_has_error_status_changed(uint32_t last_err, uint32_t curr_err)
{
	uint8_t curr_lec = curr_err & FDCAN_PSR_LEC;

	if ((0x0 != curr_lec) && (0x7 != curr_lec))
	{
		// An error is being reported in last error code field
		return true;
	}

	// Error status reported by any other field has changed
	return (last_err & ~FDCAN_PSR_LEC) != (curr_err & ~FDCAN_PSR_LEC);
}

static bool status_is_active(uint32_t err)
{
	return !(err & (FDCAN_PSR_BO | FDCAN_PSR_EP));
}

bool can_parse_error_status(can_data_t *channel, struct gs_host_frame *frame, uint32_t last_err, uint32_t curr_err)
{
	/*
	 * We build up the detailed error information at the same time as
	 * we decide whether there's anything worth sending. This variable
	 * tracks that final result.
	 */
	bool should_send = false;

	frame->echo_id = 0xFFFFFFFF;
	frame->can_id  = CAN_ERR_FLAG;
	frame->can_dlc = CAN_ERR_DLC;
	frame->classic_can->data[0] = CAN_ERR_LOSTARB_UNSPEC;
	frame->classic_can->data[1] = CAN_ERR_CRTL_UNSPEC;
	frame->classic_can->data[2] = CAN_ERR_PROT_UNSPEC;
	frame->classic_can->data[3] = CAN_ERR_PROT_LOC_UNSPEC;
	frame->classic_can->data[4] = CAN_ERR_TRX_UNSPEC;
	frame->classic_can->data[5] = 0;
	frame->classic_can->data[6] = 0;
	frame->classic_can->data[7] = 0;

	/* We transitioned from passive/bus-off to active, so report the edge. */
	if (!status_is_active(last_err) && status_is_active(curr_err)) {
		frame->can_id |= CAN_ERR_CRTL;
		frame->classic_can->data[1] |= CAN_ERR_CRTL_ACTIVE;
		should_send = true;
	}

	if (curr_err & FDCAN_PSR_BO) {
		if (!(last_err & FDCAN_PSR_BO)) {
			/* We transitioned to bus-off. */
			frame->can_id |= CAN_ERR_BUSOFF;
			should_send = true;
		}
	}

	/*
	 * The Linux sja1000 driver puts these counters here. Seems like
	 * as good a place as any.
	 */
	// TX error count
	frame->classic_can->data[6] = ((channel->channel.Instance->ECR & FDCAN_ECR_TEC) >> FDCAN_ECR_TEC_Pos);
	// RX error count
	frame->classic_can->data[7] = ((channel->channel.Instance->ECR & FDCAN_ECR_REC) >> FDCAN_ECR_REC_Pos);

	if (curr_err & FDCAN_PSR_EP) {
		if (!(last_err & FDCAN_PSR_EP)) {
			frame->can_id |= CAN_ERR_CRTL;
			frame->classic_can->data[1] |= CAN_ERR_CRTL_RX_PASSIVE | CAN_ERR_CRTL_TX_PASSIVE;
			should_send = true;
		}
	} else if (curr_err & FDCAN_PSR_EW) {
		if (!(last_err & FDCAN_PSR_EW)) {
			frame->can_id |= CAN_ERR_CRTL;
			frame->classic_can->data[1] |= CAN_ERR_CRTL_RX_WARNING | CAN_ERR_CRTL_TX_WARNING;
			should_send = true;
		}
	}

	uint8_t lec = curr_err & FDCAN_PSR_LEC;
	switch (lec) {
		case 0x01: /* stuff error */
			frame->can_id |= CAN_ERR_PROT;
			frame->classic_can->data[2] |= CAN_ERR_PROT_STUFF;
			should_send = true;
			break;
		case 0x02: /* form error */
			frame->can_id |= CAN_ERR_PROT;
			frame->classic_can->data[2] |= CAN_ERR_PROT_FORM;
			should_send = true;
			break;
		case 0x03: /* ack error */
			frame->can_id |= CAN_ERR_ACK;
			should_send = true;
			break;
		case 0x04: /* bit recessive error */
			frame->can_id |= CAN_ERR_PROT;
			frame->classic_can->data[2] |= CAN_ERR_PROT_BIT1;
			should_send = true;
			break;
		case 0x05: /* bit dominant error */
			frame->can_id |= CAN_ERR_PROT;
			frame->classic_can->data[2] |= CAN_ERR_PROT_BIT0;
			should_send = true;
			break;
		case 0x06: /* CRC error */
			frame->can_id |= CAN_ERR_PROT;
			frame->classic_can->data[3] |= CAN_ERR_PROT_LOC_CRC_SEQ;
			should_send = true;
			break;
		default: /* 0=no error, 7=no change */
			break;
	}

	return should_send;
}
