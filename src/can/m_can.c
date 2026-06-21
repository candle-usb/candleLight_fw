/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2026 Marc Kleine-Budde <kernel@pengutronix.de>
 * Copyright (c) 2016 Hubert Denkmair
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "board.h"
#include "can_common.h"
#include "can_drv.h"
#include "timer.h"

#define M_CAN_PSR_ACT_SYNC		  0
#define M_CAN_PSR_ACT_IDLE		  1
#define M_CAN_PSR_ACT_RECEIVER	  2
#define M_CAN_PSR_ACT_TRANSMITTER 3

#define M_CAN_SYNC_BUS_TIMEOUT_MS 100

const struct gs_device_bt_const CAN_btconst = {
	.feature =
		GS_CAN_FEATURE_LISTEN_ONLY |
		GS_CAN_FEATURE_LOOP_BACK |
		GS_CAN_FEATURE_HW_TIMESTAMP |
		GS_CAN_FEATURE_IDENTIFY |
		GS_CAN_FEATURE_PAD_PKTS_TO_MAX_PKT_SIZE |
		(IS_ENABLED(CONFIG_CANFD) ?
		 GS_CAN_FEATURE_FD | GS_CAN_FEATURE_BT_CONST_EXT : 0) |
		(IS_ENABLED(CONFIG_TERMINATION) ?
		 GS_CAN_FEATURE_TERMINATION : 0) |
		GS_CAN_FEATURE_BERR_REPORTING |
		GS_CAN_FEATURE_GET_STATE |
		(IS_ENABLED(CONFIG_CANFD) ?
		 GS_CAN_FEATURE_TDC : 0) |
		GS_CAN_FEATURE_BUS_OFF_RECOVERY |
		0,
	.fclk_can = CAN_CLOCK_SPEED,
	.btc = {
		.tseg1_min = 1,
		.tseg1_max = 256,
		.tseg2_min = 1,
		.tseg2_max = 128,
		.sjw_max = 128,
		.brp_min = 1,
		.brp_max = 512,
		.brp_inc = 1,
	},
};

const struct gs_device_bt_const_extended CAN_btconst_ext = {
	.feature =
		GS_CAN_FEATURE_LISTEN_ONLY |
		GS_CAN_FEATURE_LOOP_BACK |
		GS_CAN_FEATURE_HW_TIMESTAMP |
		GS_CAN_FEATURE_IDENTIFY |
		GS_CAN_FEATURE_PAD_PKTS_TO_MAX_PKT_SIZE |
		(IS_ENABLED(CONFIG_CANFD) ?
		 GS_CAN_FEATURE_FD | GS_CAN_FEATURE_BT_CONST_EXT : 0) |
		(IS_ENABLED(CONFIG_TERMINATION) ?
		 GS_CAN_FEATURE_TERMINATION : 0) |
		GS_CAN_FEATURE_BERR_REPORTING |
		GS_CAN_FEATURE_GET_STATE |
		(IS_ENABLED(CONFIG_CANFD) ?
		 GS_CAN_FEATURE_TDC : 0) |
		GS_CAN_FEATURE_BUS_OFF_RECOVERY |
		0,
	.fclk_can = CAN_CLOCK_SPEED,
	.btc = {
		.tseg1_min = 1,
		.tseg1_max = 256,
		.tseg2_min = 1,
		.tseg2_max = 128,
		.sjw_max = 128,
		.brp_min = 1,
		.brp_max = 512,
		.brp_inc = 1,
	},
	.dbtc = {
		.tseg1_min = 1,
		.tseg1_max = 32,
		.tseg2_min = 1,
		.tseg2_max = 16,
		.sjw_max = 16,
		.brp_min = 1,
		.brp_max = 32,
		.brp_inc = 1,
	},
};

const struct gs_device_tdc_const CAN_tdc_const = {
	.tdco_min = 0,
	.tdco_max = 127,
	.tdcf_min = 0,
	.tdcf_max = 127,
	.mode = GS_CAN_TDC_MODE_OFF | GS_CAN_TDC_MODE_AUTO,
};

void can_init(struct can_channel *channel, const struct board_channel_config *config)
{
	channel->channel.Instance = config->interface;
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

static void m_can_set_bittiming(struct can_channel *channel)
{
	const struct gs_device_bittiming *bt = &channel->bittiming;
	const uint8_t tseg1 = bt->prop_seg + bt->phase_seg1;

	channel->channel.Init.NominalSyncJumpWidth = bt->sjw;
	channel->channel.Init.NominalTimeSeg1 = tseg1;
	channel->channel.Init.NominalTimeSeg2 = bt->phase_seg2;
	channel->channel.Init.NominalPrescaler = bt->brp;

	const struct gs_device_bittiming *dbt = &channel->data_bittiming;
	const uint8_t dtseg1 = dbt->prop_seg + dbt->phase_seg1;

	channel->channel.Init.DataSyncJumpWidth = dbt->sjw;
	channel->channel.Init.DataTimeSeg1 = dtseg1;
	channel->channel.Init.DataTimeSeg2 = dbt->phase_seg2;
	channel->channel.Init.DataPrescaler = dbt->brp;
}

void can_drv_enable(struct can_channel *channel)
{
	m_can_set_bittiming(channel);

	const uint32_t feature = channel->feature;
	if ((feature & (GS_CAN_FEATURE_LISTEN_ONLY | GS_CAN_FEATURE_LOOP_BACK)) ==
		(GS_CAN_FEATURE_LISTEN_ONLY | GS_CAN_FEATURE_LOOP_BACK)) {
		channel->channel.Init.Mode = FDCAN_MODE_INTERNAL_LOOPBACK;
	} else if (feature & GS_CAN_FEATURE_LISTEN_ONLY) {
		channel->channel.Init.Mode = FDCAN_MODE_BUS_MONITORING;
	} else if (feature & GS_CAN_FEATURE_LOOP_BACK) {
		channel->channel.Init.Mode = FDCAN_MODE_EXTERNAL_LOOPBACK;
	} else {
		channel->channel.Init.Mode = FDCAN_MODE_NORMAL;
	}

	if (feature & GS_CAN_FEATURE_ONE_SHOT) {
		channel->channel.Init.AutoRetransmission = DISABLE;
	} else {
		channel->channel.Init.AutoRetransmission = ENABLE;
	}

	if (feature & GS_CAN_FEATURE_FD) {
		channel->channel.Init.FrameFormat = FDCAN_FRAME_FD_BRS;
	} else {
		channel->channel.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
	}

	channel->channel.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;

	HAL_FDCAN_Init(&channel->channel);

	HAL_FDCAN_EnableISOMode(&channel->channel);

	/* Configure reception filter to Rx FIFO 0 on both FDCAN instances */
	const FDCAN_FilterTypeDef sFilterConfig = {
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

	if (channel->tdc.mode & GS_CAN_TDC_MODE_AUTO) {
		HAL_FDCAN_ConfigTxDelayCompensation(&channel->channel, channel->tdc.tdco, channel->tdc.tdcf);
		HAL_FDCAN_EnableTxDelayCompensation(&channel->channel);
	} else {
		HAL_FDCAN_DisableTxDelayCompensation(&channel->channel);
	}

	HAL_FDCAN_Start(&channel->channel);
}

void can_drv_disable(struct can_channel *channel)
{
	HAL_FDCAN_Stop(&channel->channel);
}

bool can_receive(struct can_channel *channel, struct gs_host_frame *rx_frame)
{
	FDCAN_RxHeaderTypeDef RxHeader;
	uint32_t timestamp_us = timer_get();

	if (HAL_FDCAN_GetRxMessage(&channel->channel, FDCAN_RX_FIFO0, &RxHeader, rx_frame->canfd->data) != HAL_OK) {
		return false;
	}

	rx_frame->channel = can_channel_get_nr(channel);
	rx_frame->flags = 0;
	rx_frame->can_id = RxHeader.Identifier;

	if (RxHeader.IdType == FDCAN_EXTENDED_ID) {
		rx_frame->can_id |= CAN_EFF_FLAG;
	}

	if (RxHeader.RxFrameType == FDCAN_REMOTE_FRAME) {
		rx_frame->can_id |= CAN_RTR_FLAG;
	}

	rx_frame->can_dlc = RxHeader.DataLength & 0x0f;

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

bool can_is_rx_pending(struct can_channel *channel)
{
	return (HAL_FDCAN_GetRxFifoFillLevel(&channel->channel, FDCAN_RX_FIFO0) >= 1);
}

bool can_send(struct can_channel *channel, struct gs_host_frame *frame)
{
	FDCAN_TxHeaderTypeDef TxHeader = {
		.DataLength = frame->can_dlc,
		.TxEventFifoControl = FDCAN_NO_TX_EVENTS,
	};

	if (frame->can_id & CAN_RTR_FLAG) {
		TxHeader.TxFrameType = FDCAN_REMOTE_FRAME;
	} else {
		TxHeader.TxFrameType = FDCAN_DATA_FRAME;
	}

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

void can_drv_get_device_tdc(const struct can_channel *channel, struct gs_device_tdc *tdc)
{
	const uint32_t tdcv = FIELD_GET(FDCAN_PSR_TDCV, channel->reg_status.psr);

	/* tdco is 0, if no CAN-FD frame has been send, yet */
	if (channel->tdc.tdco >= tdcv)
		return;

	tdc->tdcv = tdcv - channel->tdc.tdco;
}

bool can_drv_bus_error_pending(const struct can_channel *channel)
{
	const uint32_t reg_psr = channel->reg_status.psr;
	const uint8_t lec = FIELD_GET(FDCAN_PSR_LEC, reg_psr);
	const uint8_t dlec = FIELD_GET(FDCAN_PSR_DLEC, reg_psr);

	return can_is_lec_error(lec) || can_is_lec_error(dlec);
}

void can_drv_read_reg_status(struct can_channel *channel)
{
	channel->reg_status.ecr = channel->channel.Instance->ECR;
	channel->reg_status.psr = channel->channel.Instance->PSR;  /* DLEC and LEC are clear on read */
}

bool can_drv_handle_bus_error(const struct can_channel *channel, struct gs_host_frame *frame)
{
	const uint32_t reg_ecr = channel->reg_status.ecr;

	const uint8_t tx_err = FIELD_GET(FDCAN_ECR_TEC, reg_ecr);
	const uint8_t rx_err = FIELD_GET(FDCAN_ECR_REC, reg_ecr);

	if (tx_err == 0 && rx_err == 0) {
		return false;
	}

	frame->classic_can->data[6] = tx_err;
	frame->classic_can->data[7] = rx_err;

	frame->can_id |= CAN_ERR_PROT | CAN_ERR_BUSERROR | CAN_ERR_CNT;

	const uint32_t reg_psr = channel->reg_status.psr;
	can_lec_error_to_frame(frame, FIELD_GET(FDCAN_PSR_LEC, reg_psr));
	can_lec_error_to_frame(frame, FIELD_GET(FDCAN_PSR_DLEC, reg_psr));

	return true;
}

enum gs_can_state can_drv_get_state(const struct can_channel *channel)
{
	const uint32_t reg_psr = channel->reg_status.psr;

	if (!(reg_psr & (FDCAN_PSR_BO | FDCAN_PSR_EW | FDCAN_PSR_EP))) {
		return GS_CAN_STATE_ERROR_ACTIVE;
	}

	if (reg_psr & FDCAN_PSR_BO) {
		return GS_CAN_STATE_BUS_OFF;
	}

	if (reg_psr & FDCAN_PSR_EP) {
		return GS_CAN_STATE_ERROR_PASSIVE;
	}

	return GS_CAN_STATE_ERROR_WARNING;
}

void can_drv_get_device_state(const struct can_channel *channel, struct gs_device_state *state)
{
	const uint32_t reg_ecr = channel->reg_status.ecr;

	state->state = can_drv_get_state(channel);
	state->rxerr = FIELD_GET(FDCAN_ECR_REC, reg_ecr);
	state->txerr = FIELD_GET(FDCAN_ECR_TEC, reg_ecr);
}

void can_drv_handle_state_change(const struct can_channel *channel, struct gs_host_frame *frame)
{
	const uint32_t reg_ecr = channel->reg_status.ecr;

	const uint8_t tx_err = FIELD_GET(FDCAN_ECR_TEC, reg_ecr);
	const uint8_t rx_err = FIELD_GET(FDCAN_ECR_REC, reg_ecr);

	const enum gs_can_state tx_state = can_err_to_state(tx_err);
	const enum gs_can_state rx_state = can_err_to_state(rx_err);

	if (tx_state >= rx_state) {
		frame->classic_can->data[1] |= gs_can_tx_state_to_frame(tx_state);
	}

	if (tx_state <= rx_state) {
		frame->classic_can->data[1] |= gs_can_rx_state_to_frame(rx_state);
	}
}

static void m_can_synchronize_bus(struct can_channel *channel)
{
	const uint32_t timeout = HAL_GetTick() + M_CAN_SYNC_BUS_TIMEOUT_MS;
	uint32_t now;

	do {
		can_drv_read_reg_status(channel);
		if (FIELD_GET(FDCAN_PSR_ACT, channel->reg_status.psr) != M_CAN_PSR_ACT_SYNC) {
			return;
		}

		now = HAL_GetTick();
	} while (!time_after(now, timeout));
}

void can_drv_handle_bus_off_recovery(struct can_channel *channel)
{
	HAL_FDCAN_AbortTxRequest(&channel->channel, FDCAN_TX_BUFFER0 | FDCAN_TX_BUFFER1 | FDCAN_TX_BUFFER2);
	HAL_FDCAN_Stop(&channel->channel);
	HAL_FDCAN_Start(&channel->channel);

	m_can_synchronize_bus(channel);
}
