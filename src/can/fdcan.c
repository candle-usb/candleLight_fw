/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Hubert Denkmair
 * Copyright (c) 2026 Pengutronix,
 *               Marc Kleine-Budde <kernel@pengutronix.de>
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
#include "can.h"
#include "can_common.h"
#include "config.h"
#include "device.h"
#include "gs_usb.h"
#include "timer.h"

#define FDCAN_FEATURES \
		(GS_CAN_FEATURE_LISTEN_ONLY | \
		 GS_CAN_FEATURE_LOOP_BACK | \
		 GS_CAN_FEATURE_ONE_SHOT | \
		 GS_CAN_FEATURE_HW_TIMESTAMP | \
		 GS_CAN_FEATURE_IDENTIFY | \
		 GS_CAN_FEATURE_PAD_PKTS_TO_MAX_PKT_SIZE | \
		 GS_CAN_FEATURE_FD | \
		 GS_CAN_FEATURE_BT_CONST_EXT | \
		 (IS_ENABLED(CONFIG_TERMINATION) ? \
		  GS_CAN_FEATURE_TERMINATION : 0) | \
		 0)

#define FDCAN_NOMINAL_BITTIMING \
		{ \
			.tseg1_min = 2, \
			.tseg1_max = 256, \
			.tseg2_min = 2, \
			.tseg2_max = 128, \
			.sjw_max = 128, \
			.brp_min = 1, \
			.brp_max = 512, \
			.brp_inc = 1, \
		}

const struct gs_device_bt_const CAN_btconst = {
	.feature = FDCAN_FEATURES,
	.fclk_can = CAN_CLOCK_SPEED,
	.btc = FDCAN_NOMINAL_BITTIMING,
};

const struct gs_device_bt_const_extended CAN_btconst_ext = {
	.feature = FDCAN_FEATURES,
	.fclk_can = CAN_CLOCK_SPEED,
	.btc = FDCAN_NOMINAL_BITTIMING,
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

static bool fdcan_is_dlc_valid(uint32_t dlc)
{
	/* HAL_FDCAN DataLength is the DLC code, not the shifted register field. */
	return dlc <= FDCAN_DLC_BYTES_64;
}

void can_init(can_data_t *channel, const struct board_channel_config *channel_config)
{
	FDCAN_HandleTypeDef *hfdcan = &channel->hfdcan;

	device_can_init(channel, channel_config);

	hfdcan->Init.ClockDivider = FDCAN_CLOCK_DIV1;
	hfdcan->Init.FrameFormat = FDCAN_FRAME_FD_BRS;
	hfdcan->Init.Mode = FDCAN_MODE_NORMAL;
	hfdcan->Init.AutoRetransmission = ENABLE;
	hfdcan->Init.TransmitPause = DISABLE;
	hfdcan->Init.ProtocolException = ENABLE;
	hfdcan->Init.NominalPrescaler = 8;
	hfdcan->Init.NominalSyncJumpWidth = 1;
	hfdcan->Init.NominalTimeSeg1 = 13;
	hfdcan->Init.NominalTimeSeg2 = 2;
	hfdcan->Init.DataPrescaler = 2;
	hfdcan->Init.DataSyncJumpWidth = 4;
	hfdcan->Init.DataTimeSeg1 = 15;
	hfdcan->Init.DataTimeSeg2 = 4;
	hfdcan->Init.StdFiltersNbr = 0;
	hfdcan->Init.ExtFiltersNbr = 0;
	hfdcan->Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
}

void can_set_bittiming(can_data_t *channel, const struct gs_device_bittiming *timing)
{
	FDCAN_HandleTypeDef *hfdcan = &channel->hfdcan;

	hfdcan->Init.NominalSyncJumpWidth = timing->sjw;
	hfdcan->Init.NominalTimeSeg1 = timing->prop_seg + timing->phase_seg1;
	hfdcan->Init.NominalTimeSeg2 = timing->phase_seg2;
	hfdcan->Init.NominalPrescaler = timing->brp;
}

void can_set_data_bittiming(can_data_t *channel, const struct gs_device_bittiming *timing)
{
	FDCAN_HandleTypeDef *hfdcan = &channel->hfdcan;

	hfdcan->Init.DataSyncJumpWidth = timing->sjw;
	hfdcan->Init.DataTimeSeg1 = timing->prop_seg + timing->phase_seg1;
	hfdcan->Init.DataTimeSeg2 = timing->phase_seg2;
	hfdcan->Init.DataPrescaler = timing->brp;
}

bool can_enable(can_data_t *channel)
{
	FDCAN_HandleTypeDef *hfdcan = &channel->hfdcan;
	const uint32_t feature = channel->feature;

	hfdcan->Init.AutoRetransmission = feature & GS_CAN_FEATURE_ONE_SHOT ?
									  DISABLE : ENABLE;

	if ((feature & (GS_CAN_FEATURE_LISTEN_ONLY | GS_CAN_FEATURE_LOOP_BACK)) ==
		(GS_CAN_FEATURE_LISTEN_ONLY | GS_CAN_FEATURE_LOOP_BACK)) {
		hfdcan->Init.Mode = FDCAN_MODE_INTERNAL_LOOPBACK;
	} else if (feature & GS_CAN_FEATURE_LISTEN_ONLY) {
		hfdcan->Init.Mode = FDCAN_MODE_BUS_MONITORING;
	} else if (feature & GS_CAN_FEATURE_LOOP_BACK) {
		hfdcan->Init.Mode = FDCAN_MODE_EXTERNAL_LOOPBACK;
	} else {
		hfdcan->Init.Mode = FDCAN_MODE_NORMAL;
	}

	hfdcan->Init.FrameFormat = feature & GS_CAN_FEATURE_FD ?
							   FDCAN_FRAME_FD_BRS : FDCAN_FRAME_CLASSIC;

	if (HAL_FDCAN_Init(hfdcan) != HAL_OK)
		return false;

	if (HAL_FDCAN_EnableISOMode(hfdcan) != HAL_OK)
		goto out_deinit;

	if (HAL_FDCAN_ConfigGlobalFilter(hfdcan,
									 FDCAN_ACCEPT_IN_RX_FIFO0,
									 FDCAN_ACCEPT_IN_RX_FIFO0,
									 FDCAN_FILTER_REMOTE,
									 FDCAN_FILTER_REMOTE) != HAL_OK) {
		goto out_deinit;
	}

	board_phy_power_set(channel, true);

	if (HAL_FDCAN_Start(hfdcan) != HAL_OK) {
		board_phy_power_set(channel, false);
		goto out_deinit;
	}

	return true;

out_deinit:
	HAL_FDCAN_DeInit(hfdcan);

	return false;
}

void can_disable(can_data_t *channel)
{
	HAL_FDCAN_Stop(&channel->hfdcan);
	board_phy_power_set(channel, false);
}

bool can_is_enabled(can_data_t *channel)
{
	return channel->hfdcan.State == HAL_FDCAN_STATE_BUSY;
}

bool can_receive(can_data_t *channel, struct gs_host_frame *rx_frame)
{
	FDCAN_RxHeaderTypeDef RxHeader;
	const uint32_t timestamp_us = timer_get();

	if (HAL_FDCAN_GetRxMessage(&channel->hfdcan, FDCAN_RX_FIFO0, &RxHeader,
							   rx_frame->canfd->data) != HAL_OK) {
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

	if (!fdcan_is_dlc_valid(RxHeader.DataLength))
		return false;

	rx_frame->can_dlc = RxHeader.DataLength;

	if (RxHeader.FDFormat == FDCAN_FD_CAN) {
		rx_frame->canfd_ts->timestamp_us = timestamp_us;
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
	return HAL_FDCAN_GetRxFifoFillLevel(&channel->hfdcan, FDCAN_RX_FIFO0) >= 1;
}

bool can_send(can_data_t *channel, struct gs_host_frame *frame)
{
	FDCAN_TxHeaderTypeDef TxHeader = {
		.ErrorStateIndicator = FDCAN_ESI_ACTIVE,
		.BitRateSwitch = FDCAN_BRS_OFF,
		.FDFormat = FDCAN_CLASSIC_CAN,
		.TxEventFifoControl = FDCAN_NO_TX_EVENTS,
		.MessageMarker = 0,
	};

	if (!fdcan_is_dlc_valid(frame->can_dlc))
		return false;

	TxHeader.DataLength = frame->can_dlc;

	TxHeader.TxFrameType = frame->can_id & CAN_RTR_FLAG ?
						   FDCAN_REMOTE_FRAME : FDCAN_DATA_FRAME;

	if (frame->can_id & CAN_EFF_FLAG) {
		TxHeader.IdType = FDCAN_EXTENDED_ID;
		TxHeader.Identifier = frame->can_id & 0x1fffffff;
	} else {
		TxHeader.IdType = FDCAN_STANDARD_ID;
		TxHeader.Identifier = frame->can_id & 0x7ff;
	}

	if (frame->flags & GS_CAN_FLAG_FD) {
		TxHeader.FDFormat = FDCAN_FD_CAN;
		TxHeader.BitRateSwitch = frame->flags & GS_CAN_FLAG_BRS ?
								 FDCAN_BRS_ON : FDCAN_BRS_OFF;
		TxHeader.ErrorStateIndicator = frame->flags & GS_CAN_FLAG_ESI ?
									   FDCAN_ESI_PASSIVE : FDCAN_ESI_ACTIVE;
	}

	return HAL_FDCAN_AddMessageToTxFifoQ(&channel->hfdcan, &TxHeader,
										 frame->canfd->data) == HAL_OK;
}

uint32_t can_get_error_status(can_data_t *channel)
{
	uint32_t err = channel->hfdcan.Instance->PSR;

	channel->hfdcan.Instance->PSR = 7;

	return err;
}

static bool status_is_active(uint32_t err)
{
	return !(err & (FDCAN_PSR_BO | FDCAN_PSR_EP));
}

bool can_parse_error_status(can_data_t *channel, struct gs_host_frame *frame, uint32_t err)
{
	const uint32_t last_err = channel->reg_esr_old;
	bool should_send = false;

	channel->reg_esr_old = err;

	frame->echo_id = 0xffffffff;
	frame->can_id = CAN_ERR_FLAG;
	frame->can_dlc = CAN_ERR_DLC;
	frame->classic_can->data[0] = CAN_ERR_LOSTARB_UNSPEC;
	frame->classic_can->data[1] = CAN_ERR_CRTL_UNSPEC;
	frame->classic_can->data[2] = CAN_ERR_PROT_UNSPEC;
	frame->classic_can->data[3] = CAN_ERR_PROT_LOC_UNSPEC;
	frame->classic_can->data[4] = CAN_ERR_TRX_UNSPEC;
	frame->classic_can->data[5] = 0;
	frame->classic_can->data[6] = 0;
	frame->classic_can->data[7] = 0;

	if (!status_is_active(last_err) && status_is_active(err)) {
		frame->can_id |= CAN_ERR_CRTL;
		frame->classic_can->data[1] |= CAN_ERR_CRTL_ACTIVE;
		should_send = true;
	}

	if (err & FDCAN_PSR_BO) {
		if (!(last_err & FDCAN_PSR_BO)) {
			frame->can_id |= CAN_ERR_BUSOFF;
			should_send = true;
		}

		return should_send;
	}

	frame->classic_can->data[6] =
		(channel->hfdcan.Instance->ECR & FDCAN_ECR_TEC) >> FDCAN_ECR_TEC_Pos;
	frame->classic_can->data[7] =
		(channel->hfdcan.Instance->ECR & FDCAN_ECR_REC) >> FDCAN_ECR_REC_Pos;

	if (err & FDCAN_PSR_EP) {
		if (!(last_err & FDCAN_PSR_EP)) {
			frame->can_id |= CAN_ERR_CRTL;
			frame->classic_can->data[1] |= CAN_ERR_CRTL_RX_PASSIVE |
										   CAN_ERR_CRTL_TX_PASSIVE;
			should_send = true;
		}
	} else if (err & FDCAN_PSR_EW) {
		if (!(last_err & FDCAN_PSR_EW)) {
			frame->can_id |= CAN_ERR_CRTL;
			frame->classic_can->data[1] |= CAN_ERR_CRTL_RX_WARNING |
										   CAN_ERR_CRTL_TX_WARNING;
			should_send = true;
		}
	}

	switch (err & FDCAN_PSR_LEC) {
		case 0x01:
			frame->can_id |= CAN_ERR_PROT;
			frame->classic_can->data[2] |= CAN_ERR_PROT_STUFF;
			should_send = true;
			break;
		case 0x02:
			frame->can_id |= CAN_ERR_PROT;
			frame->classic_can->data[2] |= CAN_ERR_PROT_FORM;
			should_send = true;
			break;
		case 0x03:
			frame->can_id |= CAN_ERR_ACK;
			should_send = true;
			break;
		case 0x04:
			frame->can_id |= CAN_ERR_PROT;
			frame->classic_can->data[2] |= CAN_ERR_PROT_BIT1;
			should_send = true;
			break;
		case 0x05:
			frame->can_id |= CAN_ERR_PROT;
			frame->classic_can->data[2] |= CAN_ERR_PROT_BIT0;
			should_send = true;
			break;
		case 0x06:
			frame->can_id |= CAN_ERR_PROT;
			frame->classic_can->data[3] |= CAN_ERR_PROT_LOC_CRC_SEQ;
			should_send = true;
			break;
		default:
			break;
	}

	return should_send;
}
