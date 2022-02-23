/**
  ******************************************************************************
  * @file    :
  * @author  :@GAOJUN @email:GOJUN@outlook.com @attention:All rights reserved.
  * @date    :2022/2/20
  * @brief   :The IIC driver lib implemented by software through timing method
  ******************************************************************************
	*/
/* Includes ------------------------------------------------------------------*/
/****** include begin ******/
///#include "user_lib.h"  maybe you need some other header files
#include "user_softiic.h"
#include "user_softiic_port.h"
/****** include  end  ******/

/* Private variables ---------------------------------------------------------*/
/****** Private variables begin ******/
TIM_HandleTypeDef htimer7;
SIIC_Device_TypeDef hsiic1;

static uint8_t siic_tick_run =0x00u;
static uint8_t siic1_sdlin   =0x00u;
/****** Private variables  end  ******/




/*
===============================================================================
              ##### siic tick relationed functions #####
===============================================================================
*/
/**
  * @brief
  * @note   225DMIPS     <225 I/uS> <1125 I/5uS>
  * @param
  * @param
  * @retval None
  */
void siic_tick_init(void)
{
	///modify this func according to your hardware
	TIM_ClockConfigTypeDef clkcfg ={0};

	htimer7.Instance = TIM7;
	htimer7.Init.Prescaler = (9-1);/* 90Mhz/9 =10Mhz,<+1/0.1uS> */
	htimer7.Init.Period    =50;/* 5uS tick */
	htimer7.Init.CounterMode =TIM_COUNTERMODE_UP;
	htimer7.Init.AutoReloadPreload =TIM_AUTORELOAD_PRELOAD_ENABLE;
	HAL_TIM_Base_Init(&htimer7);

	clkcfg.ClockSource =TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&htimer7,&clkcfg);
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void siic_tick_start(void)
{
	///modify this func according to your hardware
	if(siic_tick_run ==0x00u)
	{
		HAL_TIM_Base_Start_IT(&htimer7);
		/* already run */
		siic_tick_run =0x01u;
	}
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void siic_tick_stop(void)
{
	///modify this func according to your hardware
	if(siic_tick_run)
	{
		HAL_TIM_Base_Stop_IT(&htimer7);
		/* already stop */
		siic_tick_run =0x00u;
	}
}
/**
  * @brief  Period elapsed callback in non-blocking mode
  * @param  htim TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	///modify this func according to your hardware
  if(htim->Instance ==TIM7)
	{
		siic_tick_handler();
	}
	/* other timer's callback */
	else if(htim->Instance ==TIM6)
	{

	}
}
/*
===============================================================================
              ##### device1 relationed functions #####
===============================================================================
*/
/*
*
*	siic1_scl = PH6  OD
* siic1_sdl = PI3  OD
*/
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void siic1_api_init(void)
{
	///modify this func according to your hardware
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOI_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOI, GPIO_PIN_3, GPIO_PIN_SET);

	/*Configure GPIO pins */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_3;
	HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void siic1_api_sdlin(void)
{
	///modify this func according to your hardware
	if(siic1_sdlin ==0x00u)
	{
		GPIOI->MODER &= ~(0xC0ul);///PI3

		siic1_sdlin =0x01u;
	}
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void siic1_api_sdlout(void)
{
	///modify this func according to your hardware
	if(siic1_sdlin ==0x01u)
	{
		///GPIOI->MODER &= ~(0xC0ul);///PI3
		GPIOI->MODER |=  (0x40ul);///PI3

		siic1_sdlin =0x00u;
	}
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void siic1_api_sdlset(uint8_t iostate)
{
	///modify this func according to your hardware
	///HAL_GPIO_WritePin(GPIOI, GPIO_PIN_3, iostate?GPIO_PIN_SET:GPIO_PIN_RESET);
	GPIOI->BSRR = iostate?GPIO_PIN_3:(GPIO_PIN_3<<16U);
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void siic1_api_sclset(uint8_t iostate)
{
	///modify this func according to your hardware
	///HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, iostate?GPIO_PIN_SET:GPIO_PIN_RESET);
	GPIOH->BSRR = iostate?GPIO_PIN_6:(GPIO_PIN_6<<16U);
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval uint8_t
  */
uint8_t siic1_api_sdlread(void)
{
	///modify this func according to your hardware
	///return HAL_GPIO_ReadPin(GPIOI, GPIO_PIN_3);
	return ((GPIOI->IDR&GPIO_PIN_3)?1U:0U);
}
/*
===============================================================================
              ##### device2 relationed functions #####
===============================================================================
*/
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void siic2_api_init(void)
{
	///another device
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void siic2_api_sdlin(void)
{
	///another device
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void siic2_api_sdlout(void)
{
	///another device
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void siic2_api_sdlset(uint8_t iostate)
{
	///another device
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void siic2_api_sclset(uint8_t iostate)
{
	///another device
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval uint8_t
  */
uint8_t siic2_api_sdlread(void)
{
	uint8_t iostate =0x00u;

	///another device


	return iostate;
}
/*
===============================================================================
              ##### Initialization  functions #####
===============================================================================
*/
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void hsiic1_CpltCallback(SIIC_Device_TypeDef *hSIIC)
{
	///modify this func according to your hardware
	osSemaphoreRelease(myBinarySem01Handle);
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void hsiic1_wait_cplt(uint16_t timeout)
{
	///modify this func according to your hardware
	osSemaphoreAcquire(myBinarySem01Handle,timeout);
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void hsiic1_ErrorCallback(SIIC_Device_TypeDef *hSIIC)
{
	///modify this func according to your hardware

}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval uint8_t
  */
void siic_init(void)
{
	///modify this func according to your need.
	/* tick init  */
	siic_tick_init();

	/* hsiic1 init */
	hsiic1.API.init =siic1_api_init;
	hsiic1.API.sdlin =siic1_api_sdlin;
	hsiic1.API.sdlout =siic1_api_sdlout;
	hsiic1.API.sclset =siic1_api_sclset;
	hsiic1.API.sdlset =siic1_api_sdlset;
	hsiic1.API.sdlread =siic1_api_sdlread;
	siic_device_init(&hsiic1);


	/* hsiic2 and more init */
	/* blank statement */


	/* register hsiic1's callbcak  */
	siic_callback_register(&hsiic1,SIIC_XFER_CPLT_CB_ID,hsiic1_CpltCallback);
	siic_callback_register(&hsiic1,SIIC_XFER_ERROR_CB_ID,hsiic1_ErrorCallback);

	/* register siic device  */
	siic_device_register(&hsiic1);
}
