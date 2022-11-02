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

#include <stdbool.h>
#include <stdint.h>
#include "usbd_core.h"
#include "usbd_ctlreq.h"
#include "usbd_def.h"
#include "usbd_gs_can.h"

PCD_HandleTypeDef hpcd_USB_FS;

void HAL_PCD_MspInit(PCD_HandleTypeDef* hpcd)
{
	if (hpcd->Instance==USB_INTERFACE) {

#if defined(USB)
		__HAL_RCC_USB_CLK_ENABLE();
#elif defined(USB_OTG_FS)
		__HAL_RCC_USB_OTG_FS_CLK_ENABLE();
#elif defined(USB_DRD_FS)
		__HAL_RCC_USB_CLK_ENABLE();
		HAL_SYSCFG_StrobeDBattpinsConfig(SYSCFG_CFGR1_UCPD1_STROBE);
		/* Enable VDDUSB */
		if (__HAL_RCC_PWR_IS_CLK_DISABLED())
		{
			__HAL_RCC_PWR_CLK_ENABLE();
			HAL_PWREx_EnableVddUSB();
			__HAL_RCC_PWR_CLK_DISABLE();
		}
		else
		{
			HAL_PWREx_EnableVddUSB();
		}
#endif
		HAL_NVIC_SetPriority(USB_INTERRUPT, 1, 0);
		HAL_NVIC_EnableIRQ(USB_INTERRUPT);
	}
}

void HAL_PCD_MspDeInit(PCD_HandleTypeDef* hpcd)
{
	if (hpcd->Instance==USB_INTERFACE) {
#if defined(USB) || defined(USB_DRD_FS)
		__HAL_RCC_USB_CLK_DISABLE();
#elif defined(USB_OTG_FS)
		__HAL_RCC_USB_OTG_FS_CLK_DISABLE();
#endif
		HAL_NVIC_DisableIRQ(USB_INTERRUPT);
	}
}

void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd)
{
	USBD_HandleTypeDef *pdev = (USBD_HandleTypeDef*)hpcd->pData;
	USBD_ParseSetupRequest((USBD_SetupReqTypedef*)&pdev->request, (uint8_t*)hpcd->Setup);

	bool request_was_handled = false;

	if ((pdev->request.bmRequest & 0x1F) == USB_REQ_RECIPIENT_DEVICE ) { // device request
		request_was_handled = USBD_GS_CAN_CustomDeviceRequest(pdev, &pdev->request);
	}

	if ((pdev->request.bmRequest & 0x1F) == USB_REQ_RECIPIENT_INTERFACE ) { // interface request
		request_was_handled = USBD_GS_CAN_CustomInterfaceRequest(pdev, &pdev->request);
	}

	if (!request_was_handled) {
		USBD_LL_SetupStage((USBD_HandleTypeDef*)hpcd->pData, (uint8_t *)hpcd->Setup);
	}
}

void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
	USBD_LL_DataOutStage((USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->OUT_ep[epnum].xfer_buff);
}

void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
	USBD_LL_DataInStage((USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->IN_ep[epnum].xfer_buff);
}

void HAL_PCD_SOFCallback(PCD_HandleTypeDef *hpcd)
{
	USBD_LL_SOF((USBD_HandleTypeDef*)hpcd->pData);
}

void HAL_PCD_ResetCallback(PCD_HandleTypeDef *hpcd)
{
	USBD_LL_SetSpeed((USBD_HandleTypeDef*)hpcd->pData, USBD_SPEED_FULL);
	USBD_LL_Reset((USBD_HandleTypeDef*)hpcd->pData);
}

void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *hpcd)
{
	USBD_GS_CAN_SuspendCallback((USBD_HandleTypeDef*)hpcd->pData);
	USBD_LL_Suspend((USBD_HandleTypeDef*)hpcd->pData);
}

void HAL_PCD_ResumeCallback(PCD_HandleTypeDef *hpcd)
{
	USBD_LL_Resume((USBD_HandleTypeDef*) hpcd->pData);
	USBD_GS_CAN_ResumeCallback((USBD_HandleTypeDef*)hpcd->pData);
}

USBD_StatusTypeDef USBD_LL_Init(USBD_HandleTypeDef *pdev)
{
	/* Init USB_IP */
	/* Link The driver to the stack */
	hpcd_USB_FS.pData = pdev;
	pdev->pData = &hpcd_USB_FS;

	hpcd_USB_FS.Instance = USB_INTERFACE;
	hpcd_USB_FS.Init.dev_endpoints = 5U;
	hpcd_USB_FS.Init.speed = PCD_SPEED_FULL;
	hpcd_USB_FS.Init.ep0_mps = EP_MPS_64;
	hpcd_USB_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
	hpcd_USB_FS.Init.low_power_enable = DISABLE;
	hpcd_USB_FS.Init.lpm_enable = DISABLE;
#if defined(STM32F4)
	hpcd_USB_FS.Init.dma_enable = DISABLE;
	hpcd_USB_FS.Init.Sof_enable = DISABLE;
	hpcd_USB_FS.Init.vbus_sensing_enable = DISABLE;
	hpcd_USB_FS.Init.use_dedicated_ep1 = DISABLE;
#elif defined(STM32G0)
	hpcd_USB_FS.Init.Sof_enable = DISABLE;
	hpcd_USB_FS.Init.battery_charging_enable = DISABLE;
	hpcd_USB_FS.Init.vbus_sensing_enable = DISABLE;
	hpcd_USB_FS.Init.bulk_doublebuffer_enable = ENABLE;
	hpcd_USB_FS.Init.iso_singlebuffer_enable = DISABLE;
#endif
	HAL_PCD_Init(&hpcd_USB_FS);
	/*
	* PMA layout
	*  0x00 -  0x17 (24 bytes) metadata?
	*  0x18 -  0x57 (64 bytes) EP0 OUT
	*  0x58 -  0x97 (64 bytes) EP0 IN
	*  0x98 -  0xD7 (64 bytes) EP1 IN
	*  0xD8 - 0x157 (128 bytes) EP1 OUT (buffer 1)
	* 0x158 - 0x1D7 (128 bytes) EP1 OUT (buffer 2)
	*/
#if defined(USB) || defined(USB_DRD_FS)
	HAL_PCDEx_PMAConfig((PCD_HandleTypeDef*)pdev->pData, 0x00, PCD_SNG_BUF, 0x18);
	HAL_PCDEx_PMAConfig((PCD_HandleTypeDef*)pdev->pData, 0x80, PCD_SNG_BUF, 0x58);
	HAL_PCDEx_PMAConfig((PCD_HandleTypeDef*)pdev->pData, 0x81, PCD_SNG_BUF, 0x98);
	HAL_PCDEx_PMAConfig((PCD_HandleTypeDef*)pdev->pData, 0x02, PCD_DBL_BUF, 0x00D80158);
#elif defined(USB_OTG_FS)
	HAL_PCDEx_SetRxFiFo((PCD_HandleTypeDef*)pdev->pData, USB_RX_FIFO_SIZE); // shared RX FIFO
	HAL_PCDEx_SetTxFiFo((PCD_HandleTypeDef*)pdev->pData, 0U, 64U / 4U);     // 0x80, 64 bytes (div by 4 for words)
	HAL_PCDEx_SetTxFiFo((PCD_HandleTypeDef*)pdev->pData, 1U, 64U / 4U);     // 0x81, 64 bytes (div by 4 for words)
#endif

	return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_DeInit(USBD_HandleTypeDef *pdev)
{
	HAL_PCD_DeInit((PCD_HandleTypeDef*)pdev->pData);
	return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_Start(USBD_HandleTypeDef *pdev)
{
	HAL_PCD_Start((PCD_HandleTypeDef*)pdev->pData);
	return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_Stop(USBD_HandleTypeDef *pdev)
{
	HAL_PCD_Stop((PCD_HandleTypeDef*) pdev->pData);
	return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_OpenEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t ep_type, uint16_t ep_mps)
{
	HAL_PCD_EP_Open((PCD_HandleTypeDef*) pdev->pData, ep_addr, ep_mps, ep_type);
	return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_CloseEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
	HAL_PCD_EP_Close((PCD_HandleTypeDef*) pdev->pData, ep_addr);
	return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_FlushEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
	HAL_PCD_EP_Flush((PCD_HandleTypeDef*) pdev->pData, ep_addr);
	return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_StallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
	HAL_PCD_EP_SetStall((PCD_HandleTypeDef*) pdev->pData, ep_addr);
	return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_ClearStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
	HAL_PCD_EP_ClrStall((PCD_HandleTypeDef*) pdev->pData, ep_addr);
	return USBD_OK;
}

uint8_t USBD_LL_IsStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
	PCD_HandleTypeDef *hpcd = (PCD_HandleTypeDef*) pdev->pData;
	return ((ep_addr & 0x80) == 0x80)
			? hpcd->IN_ep[ep_addr & 0x7F].is_stall
			: hpcd->OUT_ep[ep_addr & 0x7F].is_stall;
}

USBD_StatusTypeDef USBD_LL_SetUSBAddress(USBD_HandleTypeDef *pdev, uint8_t dev_addr)
{
	HAL_PCD_SetAddress((PCD_HandleTypeDef*) pdev->pData, dev_addr);
	return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_Transmit(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint16_t size)
{
	HAL_PCD_EP_Transmit((PCD_HandleTypeDef*) pdev->pData, ep_addr, pbuf, size);
	return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_PrepareReceive(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint16_t size)
{
	HAL_PCD_EP_Receive((PCD_HandleTypeDef*) pdev->pData, ep_addr, pbuf, size);
	return USBD_OK;
}

uint32_t USBD_LL_GetRxDataSize(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
	return HAL_PCD_EP_GetRxCount((PCD_HandleTypeDef*) pdev->pData, ep_addr);
}
