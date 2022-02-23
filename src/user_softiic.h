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






/* Private define ------------------------------------------------------------*/
/****** Private define begin ******/
/* sub bit sequence */
#define SIIC_SUBBIT_IDLE  (0x77u)
#define SIIC_SUBBIT_START (0x31u)
#define SIIC_SUBBIT_STOP  (0x64u)
#define SIIC_SUBBIT_ZERO  (0x20u)
#define SIIC_SUBBIT_ONE   (0x27u)
#define SIIC_SUBBIT_READ  (0xA7u)
/* some mask */
#define SIIC_SUBBIT_CPLT  (0x10u) /* finish flag */
#define SIIC_SUBBIT_RMSK  (0x01u) /* read data mask */

/****** Private define  end  ******/

/**
  * @brief
  */
typedef enum
{
	SIIC_XFER_CPLT_CB_ID  =0x00,
	SIIC_XFER_ERROR_CB_ID =0x01,
}SIIC_CallbackIDTypeDef;
/**
  * @brief
  */
typedef enum
{
	SIIC_STATE_RESET  =0x00,
	SIIC_STATE_READY  =0x01,
	SIIC_STATE_BUSY   =0x02,

}SIIC_State_TypeDef;
/**
  * @brief
  */
typedef enum
{
	SIIC_OK       = 0x00U,
	SIIC_ERROR    = 0x01U,
	SIIC_BUSY     = 0x02U,
}SIIC_Status_TypeDef;
/**
  * @brief
  */
typedef enum
{
	SIIC_ERR_NONE  =0x0000u,
	SIIC_ERR_ACK   =0x0100u,
}SIIC_Errcode_TypeDef;
/**
  * @brief
  */
typedef enum
{
	SIIC_CMD_WRITE          =0x00,
	SIIC_CMD_READ           =0x01,
	SIIC_CMD_WRITE_NOSTOP   =0x02,
	SIIC_CMD_READ_RANDOM    =0x03,
	SIIC_CMD_NUM,

}SIIC_CMD_TypeDef;
/**
  * @brief
  */
typedef enum
{
	SIIC_SM_STATE_IDLE     =0x00,
	SIIC_SM_STATE_START    =0x01,
	SIIC_SM_STATE_SADDW    =0x02,
	SIIC_SM_STATE_REGCMD   =0x03,
	SIIC_SM_STATE_STARTr   =0x04,
	SIIC_SM_STATE_SADDR    =0x05,
	SIIC_SM_STATE_DATA     =0x06,
	SIIC_SM_STATE_STOP     =0x07,
	SIIC_SM_STATE_ERROR    =0x08,
	SIIC_SM_STATE_NUM,

}SIIC_SM_State_TypeDef;
/**
  * @brief  XXX_handle Structure definition
  */
typedef struct
{
	uint8_t MainState;
	uint8_t BitState;
	uint8_t SubBitState;

	/* State edge detection */
	uint8_t PrevMainState;
}SIIC_State_Machine_TypeDef;
/**
  * @brief  XXX_handle Structure definition
  */
typedef struct
{
	void (*init)(void);

	void (*sdlin)(void);
	void (*sdlout)(void);

	void (*sdlset)(uint8_t);
	void (*sclset)(uint8_t);

	uint8_t (*sdlread)(void);

}SIIC_API_TypeDef;
/**
  * @brief  XXX_handle Structure definition
	* @attention maybe you need <volatile> before some item.
		* @attention maybe you need <volatile> before some item.
			* @attention maybe you need <volatile> before some item.
  */
typedef struct SIIC_Device
{
	SIIC_State_Machine_TypeDef StateMachine;
	SIIC_API_TypeDef API;
	SIIC_State_TypeDef State;
	uint16_t errcode;
	/* Device job */
	uint8_t *pBuffPtr;
	uint16_t XferSize;
	uint16_t XferCount;
	uint8_t  CurByte;
	/* device command */
	SIIC_CMD_TypeDef DevCMD;


	/* callbacks */
	void (*XferCpltCallback) (struct SIIC_Device *hSIIC);
	void (*XferErrorCallback) (struct SIIC_Device *hSIIC);


	/* list */
	struct SIIC_Device *next;
}SIIC_Device_TypeDef;






/* Exported functions --------------------------------------------------------*/
void siic_device_register(SIIC_Device_TypeDef *hsiicdevice);
void siic_device_unregister(SIIC_Device_TypeDef *hsiicdevice);
void siic_callback_register(SIIC_Device_TypeDef *hsiicdevice,SIIC_CallbackIDTypeDef CallbackID,void (*pCallback)(SIIC_Device_TypeDef *pDevice));
void siic_callback_unregister(SIIC_Device_TypeDef *hsiicdevice,SIIC_CallbackIDTypeDef CallbackID);

void siic_tick_handler(void);

SIIC_Status_TypeDef siic_device_init(SIIC_Device_TypeDef *hsiicdevice);
SIIC_Status_TypeDef siic_device_write_it(SIIC_Device_TypeDef *hsiicdevice,uint8_t *pdata,uint16_t size);
SIIC_Status_TypeDef siic_device_read_it(SIIC_Device_TypeDef *hsiicdevice,uint8_t *pdata,uint16_t size);
SIIC_Status_TypeDef siic_device_write_nostop_it(SIIC_Device_TypeDef *hsiicdevice,uint8_t *pdata,uint16_t size);
SIIC_Status_TypeDef siic_device_read_random_it(SIIC_Device_TypeDef *hsiicdevice,uint8_t *pdata,uint16_t size);


/* Exported functions --------------------------------------------------------*/
extern SIIC_Device_TypeDef hsiic1;///port
