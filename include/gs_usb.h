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

#pragma once

#include <stdint.h>

#include "compiler.h"

#define u32				   uint32_t
#define u8				   uint8_t

#define GSUSB_ENDPOINT_IN  0x81
#define GSUSB_ENDPOINT_OUT 0x02


#define GS_CAN_MODE_NORMAL		  0
#define GS_CAN_MODE_LISTEN_ONLY	  (1<<0)
#define GS_CAN_MODE_LOOP_BACK	  (1<<1)
#define GS_CAN_MODE_TRIPLE_SAMPLE (1<<2)
#define GS_CAN_MODE_ONE_SHOT	  (1<<3)
#define GS_CAN_MODE_HW_TIMESTAMP  (1<<4)
/* #define GS_CAN_FEATURE_IDENTIFY              (1<<5) */
/* #define GS_CAN_FEATURE_USER_ID               (1<<6) */
#define GS_CAN_MODE_PAD_PKTS_TO_MAX_PKT_SIZE (1<<7)
#define GS_CAN_MODE_FD						 (1<<8)    /* switch device to CAN-FD mode */
/* #define GS_CAN_FEATURE_REQ_USB_QUIRK_LPC546XX (1<<9) */
/* #define GS_CAN_FEATURE_BT_CONST_EXT          (1<<10) */
/* #define GS_CAN_FEATURE_TERMINATION           (1<<11) */
#define GS_CAN_MODE_BERR_REPORTING (1<<12)
/* GS_CAN_FEATURE_GET_STATE (1<<13) */

#define GS_CAN_FEATURE_LISTEN_ONLY				(1<<0)
#define GS_CAN_FEATURE_LOOP_BACK				(1<<1)
#define GS_CAN_FEATURE_TRIPLE_SAMPLE			(1<<2)
#define GS_CAN_FEATURE_ONE_SHOT					(1<<3)
#define GS_CAN_FEATURE_HW_TIMESTAMP				(1<<4)
#define GS_CAN_FEATURE_IDENTIFY					(1<<5)
#define GS_CAN_FEATURE_USER_ID					(1<<6)
#define GS_CAN_FEATURE_PAD_PKTS_TO_MAX_PKT_SIZE (1<<7)
#define GS_CAN_FEATURE_FD						(1<<8) /* device supports CAN-FD */
/* request workaround for LPC546XX erratum USB.15:
 * let host driver add a padding byte to each USB frame
 */
#define GS_CAN_FEATURE_REQ_USB_QUIRK_LPC546XX	(1<<9)
/* device supports separate bit timing constants for CAN-FD
 * arbitration and data phase, see:
 * GS_USB_BREQ_BT_CONST_EXT and struct gs_device_bt_const_extended
 */
#define GS_CAN_FEATURE_BT_CONST_EXT				(1<<10)
/* device supports switchable termination, see:
 * - GS_USB_BREQ_SET_TERMINATION
 * - GS_USB_BREQ_GET_TERMINATION
 * - struct gs_device_termination_state
 */
#define GS_CAN_FEATURE_TERMINATION				(1<<11)
#define GS_CAN_FEATURE_BERR_REPORTING			(1<<12)
#define GS_CAN_FEATURE_GET_STATE				(1<<13)

#define GS_CAN_FLAG_OVERFLOW					(1<<0)
#define GS_CAN_FLAG_FD							(1<<1) /* is a CAN-FD frame */
#define GS_CAN_FLAG_BRS							(1<<2) /* bit rate switch (for CAN-FD frames) */
#define GS_CAN_FLAG_ESI							(1<<3) /* error state indicator (for CAN-FD frames) */

#define CAN_EFF_FLAG							0x80000000U /* EFF/SFF is set in the MSB */
#define CAN_RTR_FLAG							0x40000000U /* remote transmission request */
#define CAN_ERR_FLAG							0x20000000U /* error message frame */

#define CAN_ERR_DLC								8 /* dlc for error message frames */

/* error class (mask) in can_id */
#define CAN_ERR_TX_TIMEOUT 0x00000001U   /* TX timeout (by netdevice driver) */
#define CAN_ERR_LOSTARB	   0x00000002U   /* lost arbitration    / data[0]    */
#define CAN_ERR_CRTL	   0x00000004U   /* controller problems / data[1]    */
#define CAN_ERR_PROT	   0x00000008U   /* protocol violations / data[2..3] */
#define CAN_ERR_TRX		   0x00000010U   /* transceiver status  / data[4]    */
#define CAN_ERR_ACK		   0x00000020U   /* received no ACK on transmission */
#define CAN_ERR_BUSOFF	   0x00000040U   /* bus off */
#define CAN_ERR_BUSERROR   0x00000080U   /* bus error (may flood!) */
#define CAN_ERR_RESTARTED  0x00000100U   /* controller restarted */

/* arbitration lost in bit ... / data[0] */
#define CAN_ERR_LOSTARB_UNSPEC 0x00   /* unspecified */
/* else bit number in bitstream */

/* error status of CAN-controller / data[1] */
#define CAN_ERR_CRTL_UNSPEC		 0x00 /* unspecified */
#define CAN_ERR_CRTL_RX_OVERFLOW 0x01 /* RX buffer overflow */
#define CAN_ERR_CRTL_TX_OVERFLOW 0x02 /* TX buffer overflow */
#define CAN_ERR_CRTL_RX_WARNING	 0x04 /* reached warning level for RX errors */
#define CAN_ERR_CRTL_TX_WARNING	 0x08 /* reached warning level for TX errors */
#define CAN_ERR_CRTL_RX_PASSIVE	 0x10 /* reached error passive status RX */
#define CAN_ERR_CRTL_TX_PASSIVE	 0x20 /* reached error passive status TX */
/* (at least one error counter exceeds */
/* the protocol-defined level of 127)  */
#define CAN_ERR_CRTL_ACTIVE 0x40      /* recovered to error active state */

/* error in CAN protocol (type) / data[2] */
#define CAN_ERR_PROT_UNSPEC	  0x00    /* unspecified */
#define CAN_ERR_PROT_BIT	  0x01    /* single bit error */
#define CAN_ERR_PROT_FORM	  0x02    /* frame format error */
#define CAN_ERR_PROT_STUFF	  0x04    /* bit stuffing error */
#define CAN_ERR_PROT_BIT0	  0x08    /* unable to send dominant bit */
#define CAN_ERR_PROT_BIT1	  0x10    /* unable to send recessive bit */
#define CAN_ERR_PROT_OVERLOAD 0x20    /* bus overload */
#define CAN_ERR_PROT_ACTIVE	  0x40    /* active error announcement */
#define CAN_ERR_PROT_TX		  0x80    /* error occurred on transmission */

/* error in CAN protocol (location) / data[3] */
#define CAN_ERR_PROT_LOC_UNSPEC	 0x00 /* unspecified */
#define CAN_ERR_PROT_LOC_SOF	 0x03 /* start of frame */
#define CAN_ERR_PROT_LOC_ID28_21 0x02 /* ID bits 28 - 21 (SFF: 10 - 3) */
#define CAN_ERR_PROT_LOC_ID20_18 0x06 /* ID bits 20 - 18 (SFF: 2 - 0 )*/
#define CAN_ERR_PROT_LOC_SRTR	 0x04 /* substitute RTR (SFF: RTR) */
#define CAN_ERR_PROT_LOC_IDE	 0x05 /* identifier extension */
#define CAN_ERR_PROT_LOC_ID17_13 0x07 /* ID bits 17-13 */
#define CAN_ERR_PROT_LOC_ID12_05 0x0F /* ID bits 12-5 */
#define CAN_ERR_PROT_LOC_ID04_00 0x0E /* ID bits 4-0 */
#define CAN_ERR_PROT_LOC_RTR	 0x0C /* RTR */
#define CAN_ERR_PROT_LOC_RES1	 0x0D /* reserved bit 1 */
#define CAN_ERR_PROT_LOC_RES0	 0x09 /* reserved bit 0 */
#define CAN_ERR_PROT_LOC_DLC	 0x0B /* data length code */
#define CAN_ERR_PROT_LOC_DATA	 0x0A /* data section */
#define CAN_ERR_PROT_LOC_CRC_SEQ 0x08 /* CRC sequence */
#define CAN_ERR_PROT_LOC_CRC_DEL 0x18 /* CRC delimiter */
#define CAN_ERR_PROT_LOC_ACK	 0x19 /* ACK slot */
#define CAN_ERR_PROT_LOC_ACK_DEL 0x1B /* ACK delimiter */
#define CAN_ERR_PROT_LOC_EOF	 0x1A /* end of frame */
#define CAN_ERR_PROT_LOC_INTERM	 0x12 /* intermission */

/* error status of CAN-transceiver / data[4] */
/*                                             CANH CANL */
#define CAN_ERR_TRX_UNSPEC			   0x00 /* 0000 0000 */
#define CAN_ERR_TRX_CANH_NO_WIRE	   0x04 /* 0000 0100 */
#define CAN_ERR_TRX_CANH_SHORT_TO_BAT  0x05 /* 0000 0101 */
#define CAN_ERR_TRX_CANH_SHORT_TO_VCC  0x06 /* 0000 0110 */
#define CAN_ERR_TRX_CANH_SHORT_TO_GND  0x07 /* 0000 0111 */
#define CAN_ERR_TRX_CANL_NO_WIRE	   0x40 /* 0100 0000 */
#define CAN_ERR_TRX_CANL_SHORT_TO_BAT  0x50 /* 0101 0000 */
#define CAN_ERR_TRX_CANL_SHORT_TO_VCC  0x60 /* 0110 0000 */
#define CAN_ERR_TRX_CANL_SHORT_TO_GND  0x70 /* 0111 0000 */
#define CAN_ERR_TRX_CANL_SHORT_TO_CANH 0x80 /* 1000 0000 */


enum gs_usb_breq {
	GS_USB_BREQ_HOST_FORMAT = 0,
	GS_USB_BREQ_BITTIMING,
	GS_USB_BREQ_MODE,
	GS_USB_BREQ_BERR,
	GS_USB_BREQ_BT_CONST,
	GS_USB_BREQ_DEVICE_CONFIG,
	GS_USB_BREQ_TIMESTAMP,
	GS_USB_BREQ_IDENTIFY,
	GS_USB_BREQ_GET_USER_ID,    //not implemented
	GS_USB_BREQ_SET_USER_ID,    //not implemented
	GS_USB_BREQ_DATA_BITTIMING,
	GS_USB_BREQ_BT_CONST_EXT,
	GS_USB_BREQ_SET_TERMINATION,
	GS_USB_BREQ_GET_TERMINATION,
	GS_USB_BREQ_GET_STATE,
};

enum gs_can_mode {
	/* reset a channel. turns it off */
	GS_CAN_MODE_RESET = 0,
	/* starts a channel */
	GS_CAN_MODE_START
};

enum gs_can_state {
	GS_CAN_STATE_ERROR_ACTIVE = 0,
	GS_CAN_STATE_ERROR_WARNING,
	GS_CAN_STATE_ERROR_PASSIVE,
	GS_CAN_STATE_BUS_OFF,
	GS_CAN_STATE_STOPPED,
	GS_CAN_STATE_SLEEPING
};

enum gs_can_termination_state {
	GS_CAN_TERMINATION_UNSUPPORTED = -1,    // private, not in kernel enum
	GS_CAN_TERMINATION_STATE_OFF = 0,
	GS_CAN_TERMINATION_STATE_ON,
};

/* data types passed between host and device */
struct gs_host_config {
	u32 byte_order;
} __packed __aligned(4);

/* The firmware on the original USB2CAN by Geschwister Schneider
 * Technologie Entwicklungs- und Vertriebs UG exchanges all data
 * between the host and the device in host byte order. This is done
 * with the struct gs_host_config::byte_order member, which is sent
 * first to indicate the desired byte order.
 *
 * The widely used open source firmware candleLight doesn't support
 * this feature and exchanges the data in little endian byte order.
 */
struct gs_device_config {
	u8 reserved1;
	u8 reserved2;
	u8 reserved3;
	u8 icount;
	u32 sw_version;
	u32 hw_version;
} __packed __aligned(4);

struct gs_device_mode {
	u32 mode;
	u32 flags;
} __packed __aligned(4);

struct gs_device_state {
	u32 state;
	u32 rxerr;
	u32 txerr;
} __packed __aligned(4);

struct gs_device_bittiming {
	u32 prop_seg;
	u32 phase_seg1;
	u32 phase_seg2;
	u32 sjw;
	u32 brp;
} __packed __aligned(4);

struct gs_device_bt_const {
	u32 feature;
	u32 fclk_can;
	u32 tseg1_min;
	u32 tseg1_max;
	u32 tseg2_min;
	u32 tseg2_max;
	u32 sjw_max;
	u32 brp_min;
	u32 brp_max;
	u32 brp_inc;
} __packed __aligned(4);

struct gs_device_bt_const_extended {
	u32 feature;
	u32 fclk_can;
	u32 tseg1_min;
	u32 tseg1_max;
	u32 tseg2_min;
	u32 tseg2_max;
	u32 sjw_max;
	u32 brp_min;
	u32 brp_max;
	u32 brp_inc;

	u32 dtseg1_min;
	u32 dtseg1_max;
	u32 dtseg2_min;
	u32 dtseg2_max;
	u32 dsjw_max;
	u32 dbrp_min;
	u32 dbrp_max;
	u32 dbrp_inc;
} __packed __aligned(4);

struct gs_identify_mode {
	u32 mode;
} __packed __aligned(4);

struct gs_device_termination_state {
	u32 state;
} __packed __aligned(4);

struct gs_host_frame {
	u32 echo_id;
	u32 can_id;

	u8 can_dlc;
	u8 channel;
	u8 flags;
	u8 reserved;

	u8 data[8];

	u32 timestamp_us;

} __packed __aligned(4);

struct gs_host_frame_canfd {
	u32 echo_id;
	u32 can_id;

	u8 can_dlc;
	u8 channel;
	u8 flags;
	u8 reserved;

	u8 data[64];
} __packed __aligned(4);
