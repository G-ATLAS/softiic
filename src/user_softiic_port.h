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
#include "stdint.h"

/****** include  end  ******/


/* Exported functions --------------------------------------------------------*/
void siic_init(void);
void hsiic1_wait_cplt(uint16_t timeout);
