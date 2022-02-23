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
/****** include  end  ******/

/* Private variables ---------------------------------------------------------*/
/****** Private variables begin ******/
struct SIIC_Device *pSIICWorkList =NULL;

/****** Private variables  end  ******/

/* Private function prototypes -----------------------------------------------*/
/****** Private function begin ******/
void siic_state_machine_run(SIIC_Device_TypeDef *hsiicdevice);

/* main state branch */
void siic_state_machine_idle(SIIC_Device_TypeDef *hsiicdevice);
void siic_state_machine_start(SIIC_Device_TypeDef *hsiicdevice);
void siic_state_machine_saddw(SIIC_Device_TypeDef *hsiicdevice);
void siic_state_machine_regcmd(SIIC_Device_TypeDef *hsiicdevice);
void siic_state_machine_startr(SIIC_Device_TypeDef *hsiicdevice);
void siic_state_machine_saddr(SIIC_Device_TypeDef *hsiicdevice);
void siic_state_machine_data(SIIC_Device_TypeDef *hsiicdevice);
void siic_state_machine_stop(SIIC_Device_TypeDef *hsiicdevice);
void siic_state_machine_error(SIIC_Device_TypeDef *hsiicdevice);
/* sub bit */
uint8_t siic_sub_bit_sequence(SIIC_Device_TypeDef *hsiicdevice,uint8_t subbit_seq);
/****** Private function  end  ******/

/* Private variables ---------------------------------------------------------*/
/****** Private variables begin ******/
/* path table <in flash slow speed><in ram fast but waste ram> */
const uint8_t siic_sm_path[SIIC_SM_STATE_NUM][SIIC_CMD_NUM] =
{
/* SIIC_CMD_WRITE *//* SIIC_CMD_READ *//* SIIC_CMD_WRITE_NOSTOP *//* SIIC_CMD_READ_RANDOM */
{SIIC_SM_STATE_IDLE  ,SIIC_SM_STATE_IDLE  ,SIIC_SM_STATE_IDLE  ,SIIC_SM_STATE_IDLE },/* SIIC_SM_STATE_IDLE */
{SIIC_SM_STATE_SADDW ,SIIC_SM_STATE_SADDW ,SIIC_SM_STATE_SADDW ,SIIC_SM_STATE_SADDR},/* SIIC_SM_STATE_START */
{SIIC_SM_STATE_REGCMD,SIIC_SM_STATE_REGCMD,SIIC_SM_STATE_REGCMD,SIIC_SM_STATE_ERROR},/* SIIC_SM_STATE_SADDW */
{SIIC_SM_STATE_DATA  ,SIIC_SM_STATE_STARTr,SIIC_SM_STATE_DATA  ,SIIC_SM_STATE_ERROR},/* SIIC_SM_STATE_REGCMD */
{SIIC_SM_STATE_ERROR ,SIIC_SM_STATE_SADDR ,SIIC_SM_STATE_ERROR ,SIIC_SM_STATE_ERROR},/* SIIC_SM_STATE_STARTr */
{SIIC_SM_STATE_ERROR ,SIIC_SM_STATE_DATA  ,SIIC_SM_STATE_ERROR ,SIIC_SM_STATE_DATA },/* SIIC_SM_STATE_SADDR */
{SIIC_SM_STATE_STOP  ,SIIC_SM_STATE_STOP  ,SIIC_SM_STATE_IDLE  ,SIIC_SM_STATE_STOP },/* SIIC_SM_STATE_DATA */
{SIIC_SM_STATE_IDLE  ,SIIC_SM_STATE_IDLE  ,SIIC_SM_STATE_IDLE  ,SIIC_SM_STATE_IDLE },/* SIIC_SM_STATE_STOP */
{SIIC_SM_STATE_STOP  ,SIIC_SM_STATE_STOP  ,SIIC_SM_STATE_STOP  ,SIIC_SM_STATE_STOP },/* SIIC_SM_STATE_ERROR */
};

/****** Private variables  end  ******/

/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void siic_device_register(SIIC_Device_TypeDef *hsiicdevice)
{
	/* Automatic Variable */
	SIIC_Device_TypeDef *pDevice =pSIICWorkList;

	/* init gpio */
	if(hsiicdevice->State ==SIIC_STATE_RESET)
	{
		hsiicdevice->API.init();
		hsiicdevice->State =SIIC_STATE_READY;
	}

	/* add to list */
	/* exist check */
	while(pDevice)
	{
		if(pDevice==hsiicdevice)return;/* already done */
		pDevice = pDevice->next;
	}
	hsiicdevice->next = pSIICWorkList;
	pSIICWorkList = hsiicdevice;/* register done */
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void siic_device_unregister(SIIC_Device_TypeDef *hsiicdevice)
{
	/* Automatic Variable */
	uint8_t PreState = hsiicdevice->State;
	SIIC_Device_TypeDef *pDevice =pSIICWorkList;
	SIIC_Device_TypeDef *pPreDevice =pSIICWorkList;

	/* reset device */
	hsiicdevice->State =SIIC_STATE_RESET;

	/* release bus lines */
	if(PreState !=SIIC_STATE_RESET)
	{
		hsiicdevice->API.sdlout();
		hsiicdevice->API.sclset(1);
		hsiicdevice->API.sdlset(1);
	}

	/* remove device */
	while(pDevice)
	{
		if(pDevice ==hsiicdevice)
		{
			if(pDevice ==pPreDevice)
			{
				pSIICWorkList =pDevice->next;
				return;
			}
			pPreDevice->next =pDevice->next;
			return;
		}
		pPreDevice =pDevice;
		pDevice =pDevice->next;
	}
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void siic_callback_register(SIIC_Device_TypeDef *hsiicdevice,SIIC_CallbackIDTypeDef CallbackID,void (*pCallback)(SIIC_Device_TypeDef *pDevice))
{
	switch (CallbackID)
  {
  	case SIIC_XFER_CPLT_CB_ID:
			hsiicdevice->XferCpltCallback =pCallback;
  		break;
  	case SIIC_XFER_ERROR_CB_ID:
			hsiicdevice->XferErrorCallback =pCallback;
  		break;

  	default:
  		break;
  }
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void siic_callback_unregister(SIIC_Device_TypeDef *hsiicdevice,SIIC_CallbackIDTypeDef CallbackID)
{
	switch (CallbackID)
  {
  	case SIIC_XFER_CPLT_CB_ID:
			hsiicdevice->XferCpltCallback =NULL;
  		break;
  	case SIIC_XFER_ERROR_CB_ID:
			hsiicdevice->XferErrorCallback =NULL;
  		break;

  	default:
  		break;
  }
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
__weak void siic_tick_init(void)
{
	/* NOTE : This function should not be modified, when the func is needed,
            the siic_tick_init could be implemented in the port file
   */
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
__weak void siic_tick_start(void)
{
	/* NOTE : This function should not be modified, when the func is needed,
            the siic_tick_start could be implemented in the port file
   */
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
__weak void siic_tick_stop(void)
{
	/* NOTE : This function should not be modified, when the func is needed,
            the siic_tick_stop could be implemented in the port file
   */
}
/**
  * @brief
  * @note   running in isr
  * @param
  * @param
  * @retval None
  */
void siic_tick_handler(void)
{
	/* Automatic Variable */
	SIIC_Device_TypeDef *pDevice =pSIICWorkList;
	uint8_t DeviceBusyCnt =0x00;

	/* run every device */
	while(pDevice)
	{
		if(pDevice->State ==SIIC_STATE_BUSY)
		{
			siic_state_machine_run(pDevice);
			DeviceBusyCnt++;
		}
		pDevice =pDevice->next;
	}

	/* tick sleep */
	if(DeviceBusyCnt ==0x00)
	{
		siic_tick_stop();
	}
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void siic_state_machine_run(SIIC_Device_TypeDef *hsiicdevice)
{
	switch (hsiicdevice->StateMachine.MainState)
  {
  	case SIIC_SM_STATE_IDLE:
			siic_state_machine_idle(hsiicdevice);
  		break;
  	case SIIC_SM_STATE_START:
			siic_state_machine_start(hsiicdevice);
  		break;
		case SIIC_SM_STATE_SADDW:
			siic_state_machine_saddw(hsiicdevice);
  		break;
  	case SIIC_SM_STATE_REGCMD:
			siic_state_machine_regcmd(hsiicdevice);
  		break;
		case SIIC_SM_STATE_STARTr:
			siic_state_machine_startr(hsiicdevice);
  		break;
  	case SIIC_SM_STATE_SADDR:
			siic_state_machine_saddr(hsiicdevice);
  		break;
		case SIIC_SM_STATE_DATA:
			siic_state_machine_data(hsiicdevice);
  		break;
  	case SIIC_SM_STATE_STOP:
			siic_state_machine_stop(hsiicdevice);
  		break;
		case SIIC_SM_STATE_ERROR:
			siic_state_machine_error(hsiicdevice);
  		break;

  	default:
  		break;
  }
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void siic_sm_state_switch(SIIC_Device_TypeDef *hsiicdevice,SIIC_SM_State_TypeDef next_state)
{
	/* flush old state */
	hsiicdevice->StateMachine.PrevMainState = hsiicdevice->StateMachine.MainState;
	/* switch to the next state */
	hsiicdevice->StateMachine.MainState = next_state;
	/* reset sub state */
	hsiicdevice->StateMachine.BitState = 0x00u;
	hsiicdevice->StateMachine.SubBitState = 0x00u;
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void siic_sm_write_byte(SIIC_Device_TypeDef *hsiicdevice)
{
	/* Automatic Variable */
	uint8_t retdata =0x00u;

	switch (hsiicdevice->StateMachine.BitState)
  {
  	case 0x00u:
			/* flush the current byte */
			hsiicdevice->CurByte = hsiicdevice->pBuffPtr[hsiicdevice->XferCount];
			/* bit action */
			retdata =siic_sub_bit_sequence(hsiicdevice,((hsiicdevice->CurByte&0x80u)?SIIC_SUBBIT_ONE:SIIC_SUBBIT_ZERO));
			if(retdata&SIIC_SUBBIT_CPLT){hsiicdevice->StateMachine.BitState =0x01u;}
  		break;
  	case 0x01u:
			/* bit action */
			retdata =siic_sub_bit_sequence(hsiicdevice,((hsiicdevice->CurByte&0x40u)?SIIC_SUBBIT_ONE:SIIC_SUBBIT_ZERO));
			if(retdata&SIIC_SUBBIT_CPLT){hsiicdevice->StateMachine.BitState =0x02u;}
  		break;
		case 0x02u:
			/* bit action */
			retdata =siic_sub_bit_sequence(hsiicdevice,((hsiicdevice->CurByte&0x20u)?SIIC_SUBBIT_ONE:SIIC_SUBBIT_ZERO));
			if(retdata&SIIC_SUBBIT_CPLT){hsiicdevice->StateMachine.BitState =0x03u;}
  		break;
		case 0x03u:
			/* bit action */
			retdata =siic_sub_bit_sequence(hsiicdevice,((hsiicdevice->CurByte&0x10u)?SIIC_SUBBIT_ONE:SIIC_SUBBIT_ZERO));
			if(retdata&SIIC_SUBBIT_CPLT){hsiicdevice->StateMachine.BitState =0x04u;}
  		break;
  	case 0x04u:
			/* bit action */
			retdata =siic_sub_bit_sequence(hsiicdevice,((hsiicdevice->CurByte&0x08u)?SIIC_SUBBIT_ONE:SIIC_SUBBIT_ZERO));
			if(retdata&SIIC_SUBBIT_CPLT){hsiicdevice->StateMachine.BitState =0x05u;}
  		break;
		case 0x05u:
			/* bit action */
			retdata =siic_sub_bit_sequence(hsiicdevice,((hsiicdevice->CurByte&0x04u)?SIIC_SUBBIT_ONE:SIIC_SUBBIT_ZERO));
			if(retdata&SIIC_SUBBIT_CPLT){hsiicdevice->StateMachine.BitState =0x06u;}
  		break;
		case 0x06u:
			/* bit action */
			retdata =siic_sub_bit_sequence(hsiicdevice,((hsiicdevice->CurByte&0x02u)?SIIC_SUBBIT_ONE:SIIC_SUBBIT_ZERO));
			if(retdata&SIIC_SUBBIT_CPLT){hsiicdevice->StateMachine.BitState =0x07u;}
  		break;
  	case 0x07u:
			/* bit action */
			retdata =siic_sub_bit_sequence(hsiicdevice,((hsiicdevice->CurByte&0x01u)?SIIC_SUBBIT_ONE:SIIC_SUBBIT_ZERO));
			if(retdata&SIIC_SUBBIT_CPLT){hsiicdevice->StateMachine.BitState =0x08u;}
  		break;
		case 0x08u:
			/* need ACK */
			retdata =siic_sub_bit_sequence(hsiicdevice,SIIC_SUBBIT_READ);
			if(retdata&SIIC_SUBBIT_CPLT)
			{
				if(retdata&SIIC_SUBBIT_RMSK)
				{
					/* NACK report error */
					hsiicdevice->errcode |= hsiicdevice->StateMachine.MainState;
					hsiicdevice->errcode |= SIIC_ERR_ACK;
					siic_sm_state_switch(hsiicdevice,SIIC_SM_STATE_ERROR);
				}
				else
				{
					/* ACK continue*/
					hsiicdevice->XferCount++;/* eat a byte */
					if(hsiicdevice->XferCount >=hsiicdevice->XferSize)
          {
						if(hsiicdevice->DevCMD ==SIIC_CMD_WRITE_NOSTOP)
						{siic_sm_state_switch(hsiicdevice,SIIC_SM_STATE_IDLE);}
						else{siic_sm_state_switch(hsiicdevice,SIIC_SM_STATE_STOP);}
					}
					else
					{
						if(hsiicdevice->StateMachine.MainState ==SIIC_SM_STATE_DATA){hsiicdevice->StateMachine.BitState =0x00u;/* self-circulation */}
						else{siic_sm_state_switch(hsiicdevice,siic_sm_path[hsiicdevice->StateMachine.MainState][hsiicdevice->DevCMD]);}
					}
				}
			}
  		break;

  	default:
  		break;
  }
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void siic_sm_read_byte(SIIC_Device_TypeDef *hsiicdevice)
{
	/* Automatic Variable */
	uint8_t retdata =0x00u;

	switch (hsiicdevice->StateMachine.BitState)
  {
  	case 0x00u:
			/* flush the current byte */
			hsiicdevice->CurByte = 0x00u;
			/* bit action */
			retdata =siic_sub_bit_sequence(hsiicdevice,SIIC_SUBBIT_READ);
			if(retdata&SIIC_SUBBIT_CPLT){if(retdata&SIIC_SUBBIT_RMSK){hsiicdevice->CurByte |=0x80u;}hsiicdevice->StateMachine.BitState =0x01u;}
  		break;
  	case 0x01u:
			/* bit action */
			retdata =siic_sub_bit_sequence(hsiicdevice,SIIC_SUBBIT_READ);
			if(retdata&SIIC_SUBBIT_CPLT){if(retdata&SIIC_SUBBIT_RMSK){hsiicdevice->CurByte |=0x40u;}hsiicdevice->StateMachine.BitState =0x02u;}
  		break;
		case 0x02u:
			/* bit action */
			retdata =siic_sub_bit_sequence(hsiicdevice,SIIC_SUBBIT_READ);
			if(retdata&SIIC_SUBBIT_CPLT){if(retdata&SIIC_SUBBIT_RMSK){hsiicdevice->CurByte |=0x20u;}hsiicdevice->StateMachine.BitState =0x03u;}
  		break;
		case 0x03u:
			/* bit action */
			retdata =siic_sub_bit_sequence(hsiicdevice,SIIC_SUBBIT_READ);
			if(retdata&SIIC_SUBBIT_CPLT){if(retdata&SIIC_SUBBIT_RMSK){hsiicdevice->CurByte |=0x10u;}hsiicdevice->StateMachine.BitState =0x04u;}
  		break;
  	case 0x04u:
			/* bit action */
			retdata =siic_sub_bit_sequence(hsiicdevice,SIIC_SUBBIT_READ);
			if(retdata&SIIC_SUBBIT_CPLT){if(retdata&SIIC_SUBBIT_RMSK){hsiicdevice->CurByte |=0x08u;}hsiicdevice->StateMachine.BitState =0x05u;}
  		break;
		case 0x05u:
			/* bit action */
			retdata =siic_sub_bit_sequence(hsiicdevice,SIIC_SUBBIT_READ);
			if(retdata&SIIC_SUBBIT_CPLT){if(retdata&SIIC_SUBBIT_RMSK){hsiicdevice->CurByte |=0x04u;}hsiicdevice->StateMachine.BitState =0x06u;}
  		break;
		case 0x06u:
			/* bit action */
			retdata =siic_sub_bit_sequence(hsiicdevice,SIIC_SUBBIT_READ);
			if(retdata&SIIC_SUBBIT_CPLT){if(retdata&SIIC_SUBBIT_RMSK){hsiicdevice->CurByte |=0x02u;}hsiicdevice->StateMachine.BitState =0x07u;}
  		break;
  	case 0x07u:
			/* bit action */
			retdata =siic_sub_bit_sequence(hsiicdevice,SIIC_SUBBIT_READ);
			if(retdata&SIIC_SUBBIT_CPLT){if(retdata&SIIC_SUBBIT_RMSK){hsiicdevice->CurByte |=0x01u;}hsiicdevice->StateMachine.BitState =0x08u;}
  		break;
		case 0x08u:
			/* need ACK ? */
			if((hsiicdevice->XferCount+1)>=hsiicdevice->XferSize)
			{
				retdata =siic_sub_bit_sequence(hsiicdevice,SIIC_SUBBIT_ONE);/* NACK */
				if(retdata&SIIC_SUBBIT_CPLT)
				{
					hsiicdevice->pBuffPtr[hsiicdevice->XferCount] =hsiicdevice->CurByte;/* save the last byte */
					hsiicdevice->XferCount++;/* inc index */
					siic_sm_state_switch(hsiicdevice,siic_sm_path[hsiicdevice->StateMachine.MainState][hsiicdevice->DevCMD]);
				}
			}
			else
			{
				retdata =siic_sub_bit_sequence(hsiicdevice,SIIC_SUBBIT_ZERO);/* ACK */
				if(retdata&SIIC_SUBBIT_CPLT)
				{
					hsiicdevice->pBuffPtr[hsiicdevice->XferCount] =hsiicdevice->CurByte;/* save the current byte */
					hsiicdevice->XferCount++;/* inc index */
					hsiicdevice->StateMachine.BitState =0x00u;/* self-circulation */
				}
			}
  		break;

  	default:
  		break;
  }
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void siic_state_machine_idle(SIIC_Device_TypeDef *hsiicdevice)
{
	switch (hsiicdevice->StateMachine.BitState)
  {
  	case 0x00u:
			/* idle init */
			hsiicdevice->API.sdlout();
			hsiicdevice->API.sclset(1);
			hsiicdevice->API.sdlset(1);

			/* clear busy */
			hsiicdevice->State =SIIC_STATE_READY;

			/* cplt cb */
			if(hsiicdevice->XferCpltCallback)
			{
				hsiicdevice->XferCpltCallback(hsiicdevice);
			}

			/* next state */
			hsiicdevice->StateMachine.BitState = 0x01u;
  		break;
  	case 0x01u:
			/* idle loop */
			/* blank statement */
  		break;

  	default:
  		break;
  }
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void siic_state_machine_start(SIIC_Device_TypeDef *hsiicdevice)
{
	/* Automatic Variable */
	uint8_t retdata =0x00u;

	switch (hsiicdevice->StateMachine.BitState)
  {
  	case 0x00u:
			/* start init */
			retdata=siic_sub_bit_sequence(hsiicdevice,SIIC_SUBBIT_IDLE);
			if(retdata&SIIC_SUBBIT_CPLT){hsiicdevice->StateMachine.BitState =0x01u;}
  		break;
  	case 0x01u:
			/* true start */
			retdata=siic_sub_bit_sequence(hsiicdevice,SIIC_SUBBIT_START);
			if(retdata&SIIC_SUBBIT_CPLT)
			{siic_sm_state_switch(hsiicdevice,siic_sm_path[SIIC_SM_STATE_START][hsiicdevice->DevCMD]);}
  		break;

  	default:
  		break;
  }
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void siic_state_machine_saddw(SIIC_Device_TypeDef *hsiicdevice)
{
	siic_sm_write_byte(hsiicdevice);
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void siic_state_machine_regcmd(SIIC_Device_TypeDef *hsiicdevice)
{
	siic_sm_write_byte(hsiicdevice);
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void siic_state_machine_startr(SIIC_Device_TypeDef *hsiicdevice)
{
	/* Automatic Variable */
	uint8_t retdata =0x00u;

	switch (hsiicdevice->StateMachine.BitState)
  {
  	case 0x00u:
			/* true start repetition */
			retdata=siic_sub_bit_sequence(hsiicdevice,SIIC_SUBBIT_START);
			if(retdata&SIIC_SUBBIT_CPLT)
			{siic_sm_state_switch(hsiicdevice,siic_sm_path[SIIC_SM_STATE_STARTr][hsiicdevice->DevCMD]);}
  		break;
  	case 0x01u:
  		break;

  	default:
  		break;
  }
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void siic_state_machine_saddr(SIIC_Device_TypeDef *hsiicdevice)
{
	siic_sm_write_byte(hsiicdevice);
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void siic_state_machine_data(SIIC_Device_TypeDef *hsiicdevice)
{
	if(hsiicdevice->DevCMD ==SIIC_CMD_READ||(hsiicdevice->DevCMD ==SIIC_CMD_READ_RANDOM))
	{siic_sm_read_byte(hsiicdevice);}
	else{siic_sm_write_byte(hsiicdevice);}
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void siic_state_machine_stop(SIIC_Device_TypeDef *hsiicdevice)
{
	/* Automatic Variable */
	uint8_t retdata =0x00u;

	switch (hsiicdevice->StateMachine.BitState)
  {
  	case 0x00u:
			/* true stop */
			retdata=siic_sub_bit_sequence(hsiicdevice,SIIC_SUBBIT_STOP);
			if(retdata&SIIC_SUBBIT_CPLT)
			{siic_sm_state_switch(hsiicdevice,siic_sm_path[SIIC_SM_STATE_STOP][hsiicdevice->DevCMD]);}
  		break;
  	case 0x01u:
  		break;

  	default:
  		break;
  }
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
void siic_state_machine_error(SIIC_Device_TypeDef *hsiicdevice)
{
	/* error state -save some err info */
	///prepare err data

	/* error callback */
	if(hsiicdevice->XferErrorCallback)
	{
		hsiicdevice->XferErrorCallback(hsiicdevice);
	}
	/* switch state */
	siic_sm_state_switch(hsiicdevice,siic_sm_path[hsiicdevice->StateMachine.MainState][hsiicdevice->DevCMD]);
}
/**
  * @brief
  * @note
  * @param
  * @param  subbit_seq -<sub bit sequence>
  * @retval uint8_t<0b0001-0001> -bit0<read data> -bit4<sequence finish flag>
  */
uint8_t siic_sub_bit_sequence(SIIC_Device_TypeDef *hsiicdevice,uint8_t subbit_seq)
{
	/* Automatic Variable */
	uint8_t retdata = 0x00u;

	switch (hsiicdevice->StateMachine.SubBitState)
  {
  	case 0x00u:
			/* input/output check */
			if(subbit_seq &0x80u){hsiicdevice->API.sdlin();/* input mode */}
			else{hsiicdevice->API.sdlout();/* output mode */}
			/* level action */
			hsiicdevice->API.sdlset((subbit_seq&0x01u)?1:0);
			hsiicdevice->API.sclset((subbit_seq&0x10u)?1:0);
			hsiicdevice->StateMachine.SubBitState = 0x01u;
  		break;
  	case 0x01u:
			/* level action */
			hsiicdevice->API.sdlset((subbit_seq&0x02u)?1:0);
			hsiicdevice->API.sclset((subbit_seq&0x20u)?1:0);
			hsiicdevice->StateMachine.SubBitState = 0x02u;
  		break;
		case 0x02u:
			/* if read action */
			if(subbit_seq &0x80u){retdata |= (hsiicdevice->API.sdlread()?1:0);}
			/* level action */
			hsiicdevice->API.sdlset((subbit_seq&0x04u)?1:0);
			hsiicdevice->API.sclset((subbit_seq&0x40u)?1:0);
			hsiicdevice->StateMachine.SubBitState = 0x00u;/* roll back feature */
			/* sequence finish */
			retdata |= 0x10u;/* insert finish flag */
  		break;

  	default:
  		break;
  }
	return retdata;
}
/**
  * @brief
  * @note
  * @param
  * @param
  * @retval None
  */
SIIC_Status_TypeDef siic_device_init(SIIC_Device_TypeDef *hsiicdevice)
{
	SIIC_Status_TypeDef ret =SIIC_OK;

	if(hsiicdevice->API.init ==NULL)
	{
		ret =SIIC_ERROR;
	}
	if(hsiicdevice->API.sclset ==NULL)
	{
		ret =SIIC_ERROR;
	}
	if(hsiicdevice->API.sdlin ==NULL)
	{
		ret =SIIC_ERROR;
	}
	if(hsiicdevice->API.sdlout ==NULL)
	{
		ret =SIIC_ERROR;
	}
	if(hsiicdevice->API.sdlread ==NULL)
	{
		ret =SIIC_ERROR;
	}
	if(hsiicdevice->API.sdlset ==NULL)
	{
		ret =SIIC_ERROR;
	}

	/* default */
	hsiicdevice->CurByte =0x00u;
	hsiicdevice->DevCMD  =0x00u;
	hsiicdevice->errcode =0x00u;
	hsiicdevice->pBuffPtr =NULL;
	hsiicdevice->XferCount=0x00u;
	hsiicdevice->XferSize =0x00u;
	hsiicdevice->StateMachine.MainState =SIIC_SM_STATE_IDLE;
	hsiicdevice->StateMachine.BitState  =0x01;/* true idle loop */
	hsiicdevice->StateMachine.SubBitState =0x00u;
	hsiicdevice->StateMachine.PrevMainState =SIIC_SM_STATE_IDLE;
	hsiicdevice->XferCpltCallback  =NULL;
	hsiicdevice->XferErrorCallback =NULL;
	hsiicdevice->State =SIIC_STATE_RESET;

	return ret;
}

/**
  * @brief
  * @note   pdata       -pdata[0]=<Slave address+write> ,pdata[1]=<register offset or command> ,pdata[2--n]=the data to be written.
  * @param  hsiicdevice -a pointer to the structure which contains information  about siic.
  * @param  pdata       -the data pointer.
  * @param  size        -the size of data<include pdata[0]pdata[1]>. in byte. should be >=2.
  * @retval SIIC_Status_TypeDef
  */
SIIC_Status_TypeDef siic_device_write_it(SIIC_Device_TypeDef *hsiicdevice,uint8_t *pdata,uint16_t size)
{
	/* busy and init check? */
	if(hsiicdevice->State ==SIIC_STATE_BUSY){return SIIC_BUSY;}
	if(hsiicdevice->State ==SIIC_STATE_RESET){return SIIC_ERROR;}

	/* prepare task info */
	hsiicdevice->pBuffPtr  =pdata;
	hsiicdevice->XferSize  =size;
	hsiicdevice->XferCount =0x00u;
	hsiicdevice->errcode   =SIIC_ERR_NONE;
	hsiicdevice->DevCMD    =SIIC_CMD_WRITE;

	/* reset state machine */
	siic_sm_state_switch(hsiicdevice,SIIC_SM_STATE_START);

	/* enable sm scan */
	hsiicdevice->State = SIIC_STATE_BUSY;

	/* start tick */
	siic_tick_start();

	return SIIC_OK;
}
/**
  * @brief
  * @note   pdata       -pdata[0]=<Slave address+write> ,pdata[1]=<register offset or command> ,
                         pdata[2]=<Slave address+read>  ,pdata[3--n]= the data to be read.
  * @param  hsiicdevice -a pointer to the structure which contains information  about siic.
  * @param  pdata       -the data pointer.
  * @param  size        -the size of data<include pdata[0]pdata[1]pdata[2]>. in byte.should be >=3.
  * @retval SIIC_Status_TypeDef
  */
SIIC_Status_TypeDef siic_device_read_it(SIIC_Device_TypeDef *hsiicdevice,uint8_t *pdata,uint16_t size)
{
	/* busy and init check? */
	if(hsiicdevice->State ==SIIC_STATE_BUSY){return SIIC_BUSY;}
	if(hsiicdevice->State ==SIIC_STATE_RESET){return SIIC_ERROR;}

	/* prepare task info */
	hsiicdevice->pBuffPtr  =pdata;
	hsiicdevice->XferSize  =size;
	hsiicdevice->XferCount =0x00u;
	hsiicdevice->errcode   =SIIC_ERR_NONE;
	hsiicdevice->DevCMD    =SIIC_CMD_READ;

	/* reset state machine */
	siic_sm_state_switch(hsiicdevice,SIIC_SM_STATE_START);

	/* enable sm scan */
	hsiicdevice->State = SIIC_STATE_BUSY;

	/* start tick */
	siic_tick_start();

	return SIIC_OK;
}
/**
  * @brief  the same as func<siic_device_write_it>,just without the stop condition.
  * @note   pdata       -pdata[0]=<Slave address+write> ,pdata[1]=<register offset or command> ,pdata[2--n]=the data to be written.
  * @param  hsiicdevice -a pointer to the structure which contains information  about siic.
  * @param  pdata       -the data pointer.
  * @param  size        -the size of data<include pdata[0]pdata[1]>. in byte. should be >=2.
  * @retval SIIC_Status_TypeDef
  */
SIIC_Status_TypeDef siic_device_write_nostop_it(SIIC_Device_TypeDef *hsiicdevice,uint8_t *pdata,uint16_t size)
{
	/* busy and init check? */
	if(hsiicdevice->State ==SIIC_STATE_BUSY){return SIIC_BUSY;}
	if(hsiicdevice->State ==SIIC_STATE_RESET){return SIIC_ERROR;}

	/* prepare task info */
	hsiicdevice->pBuffPtr  =pdata;
	hsiicdevice->XferSize  =size;
	hsiicdevice->XferCount =0x00u;
	hsiicdevice->errcode   =SIIC_ERR_NONE;
	hsiicdevice->DevCMD    =SIIC_CMD_WRITE_NOSTOP;

	/* reset state machine */
	siic_sm_state_switch(hsiicdevice,SIIC_SM_STATE_START);

	/* enable sm scan */
	hsiicdevice->State = SIIC_STATE_BUSY;

	/* start tick */
	siic_tick_start();

	return SIIC_OK;
}
/**
  * @brief  the same as func<siic_device_read_it>,just without indicate the offset of the internal register.
  * @note   pdata       -pdata[0]=<Slave address+read>  ,pdata[1--n]= the data to be read.
  * @param  hsiicdevice -a pointer to the structure which contains information  about siic.
  * @param  pdata       -the data pointer.
  * @param  size        -the size of data<include pdata[0]>. in byte.should be >=2.
  * @retval SIIC_Status_TypeDef
  */
SIIC_Status_TypeDef siic_device_read_random_it(SIIC_Device_TypeDef *hsiicdevice,uint8_t *pdata,uint16_t size)
{
	/* busy and init check? */
	if(hsiicdevice->State ==SIIC_STATE_BUSY){return SIIC_BUSY;}
	if(hsiicdevice->State ==SIIC_STATE_RESET){return SIIC_ERROR;}

	/* prepare task info */
	hsiicdevice->pBuffPtr  =pdata;
	hsiicdevice->XferSize  =size;
	hsiicdevice->XferCount =0x00u;
	hsiicdevice->errcode   =SIIC_ERR_NONE;
	hsiicdevice->DevCMD    =SIIC_CMD_READ_RANDOM;

	/* reset state machine */
	siic_sm_state_switch(hsiicdevice,SIIC_SM_STATE_START);

	/* enable sm scan */
	hsiicdevice->State = SIIC_STATE_BUSY;

	/* start tick */
	siic_tick_start();

	return SIIC_OK;
}
