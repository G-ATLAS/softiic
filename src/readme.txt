++++++++++++++++++++++++++++++++++++++++++++++++++
                                  一个简单易用的 软件IIC组件
++++++++++++++++++++++++++++++++++++++++++++++++++

#softiic v1.0  纯软件IIC组件

## 组件参考文档
-IIC-bus specification and user manual.
https://www.nxp.com/docs/en/user-guide/UM10204.pdf
-IIC-bus resource.
https://www.i2c-bus.org/


## 简介
softiic 是一个纯软件IIC组件，纯C语言构建。面向嵌入式应用，用于简化软件设计流程。
实现了IIC协议的作为master的基本功能。内部由状态机构建。非阻塞式实现。可自由扩展
IIC设备，只要有充足的GPIO和CPU速度。由软件模拟硬件外设的设计思想，这得益于素有
DC Bus之称的IIC总线协议。具有极为简单的API接口，易使用。


## 特点
-纯C语言构建
-非阻塞式软件IIC设备
-无限扩展虚拟化软件IIC设备，如果有充足的GPIO和CPU速度
-通信速度可调，如果有充足的CPU速度
-极为简单的API接口
-自动生成START,STOP,ACK,NACK,STARTr条件
-只支持7位地址，和几个主机常用命令模式
-不支持时钟同步
-不支持总线仲裁
-不支持时钟拉扯
-需要硬件定时器节拍


## 功能
-主机写从设备
-主机写从设备+不带Stop条件
-主机读从设备
-主机读从设备+不指定寄存器偏移
-更多

## 应用功能API
-<主机写从设备>
/**
  * @brief  
  * @note   pdata       -pdata[0]=<Slave address+write> ,pdata[1]=<register offset or command> ,pdata[2--n]=the data to be written.
  * @param  hsiicdevice -a pointer to the structure which contains information  about siic. 
  * @param  pdata       -the data pointer. 
  * @param  size        -the size of data<include pdata[0]pdata[1]>. in byte. should be >=2.
  * @retval SIIC_Status_TypeDef
  */
SIIC_Status_TypeDef siic_device_write_it(SIIC_Device_TypeDef *hsiicdevice,uint8_t *pdata,uint16_t size)

-<主机读从设备>
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


-<主机写从设备+不带Stop条件>
/**
  * @brief  the same as func<siic_device_write_it>,just without the stop condition.
  * @note   pdata       -pdata[0]=<Slave address+write> ,pdata[1]=<register offset or command> ,pdata[2--n]=the data to be written.
  * @param  hsiicdevice -a pointer to the structure which contains information  about siic. 
  * @param  pdata       -the data pointer. 
  * @param  size        -the size of data<include pdata[0]pdata[1]>. in byte. should be >=2.
  * @retval SIIC_Status_TypeDef
  */
SIIC_Status_TypeDef siic_device_write_nostop_it(SIIC_Device_TypeDef *hsiicdevice,uint8_t *pdata,uint16_t size)


-<主机读从设备+不指定寄存器偏移>
/**
  * @brief  the same as func<siic_device_read_it>,just without indicate the offset of the internal register.
  * @note   pdata       -pdata[0]=<Slave address+read>  ,pdata[1--n]= the data to be read.
  * @param  hsiicdevice -a pointer to the structure which contains information  about siic. 
  * @param  pdata       -the data pointer.
  * @param  size        -the size of data<include pdata[0]>. in byte.should be >=2.
  * @retval SIIC_Status_TypeDef
  */
SIIC_Status_TypeDef siic_device_read_random_it(SIIC_Device_TypeDef *hsiicdevice,uint8_t *pdata,uint16_t size)


-<软件IIC组件初始化>
void siic_init(void)


## 移植说明
1-把目光转移到xx_port.c文件中，为软件IIC准备 硬件定时器 相关接口函数。
void siic_tick_init(void)-初始化硬件定时器，使用其 更新中断。配置中断优先级等等。中断频率5-50uS为宜，根据平台性能。
void siic_tick_start(void) -启动硬件定时器。可做重复启动优化，见源文件。
void siic_tick_stop(void) -停止硬件定时器。可做重复停止优化，见源文件。
siic_tick_handler(); -在 定时器更新中断 的服务程序中调用，原则尽量快。

2-为 某个软件IIC设备<siic1> 准备 GPIO 相关接口函数。
void siic1_api_init(void) -初始化SDL,SCL相关io口，注意默认高电平，OD输出，上拉。
void siic1_api_sdlin(void) -切换SDL方向为输入。
void siic1_api_sdlout(void) -切换SDL方向为输出。
void siic1_api_sdlset(uint8_t iostate) -设置SDL输出电平。
void siic1_api_sclset(uint8_t iostate) -设置SCL输出电平 。
uint8_t siic1_api_sdlread(void) -读入SDL输入电平。


3-为 更多软件IIC设备<siic2.3.4> 准备 GPIO 相关接口函数。如果需要。
void siic2_api_init(void) -初始化SDL,SCL相关io口，注意默认高电平，OD输出，上拉。
void siic2_api_sdlin(void) -切换SDL方向为输入。
void siic2_api_sdlout(void) -切换SDL方向为输出。
void siic2_api_sdlset(uint8_t iostate) -设置SDL输出电平。
void siic2_api_sclset(uint8_t iostate) -设置SCL输出电平 。
uint8_t siic2_api_sdlread(void) -读入SDL输入电平。

4-实现软件IIC组件 初始化函数。如下
SIIC_Device_TypeDef hsiic1;
///SIIC_Device_TypeDef hsiic2;and more device
void siic_init(void)
{
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
	
	/* register hsiic2's callbcak  */
	/* blank statement */

	/* register siic device  */
	siic_device_register(&hsiic1);
	///siic_device_register(&hsiic2); and more device
}
void hsiic1_CpltCallback(SIIC_Device_TypeDef *hSIIC)
{
	///osSemaphoreRelease(myBinarySem01Handle);
}
void hsiic1_wait_cplt(uint16_t timeout)
{
	///osSemaphoreAcquire(myBinarySem01Handle,timeout);
}
void hsiic1_ErrorCallback(SIIC_Device_TypeDef *hSIIC)
{
	
}

5-注意！！！
本组件测试平台STM32F429IGT6  180Mhz，性能 225DMIPS, <225 I/uS><1125 I/5uS>
本硬件定时器中断周期的 设置为5uS。 一个位的传输需要3个tick<15uS>
！！！！！！！！！！！！！！
不同平台性能不同，你必须确保 <从 中断发生 到 扫描完一遍siic_tick_handler(); >
所用时间 < 硬件定时器中断周期。
总的来说 性能越好，中断周期可以更短。性能越差，中断周期必须更长。
！！！！！！！！！！！！！！



6-注意！！！
建议CPU>32Mhz的平台移植，频率过低 从 这种实现的软件IIC组件 得不到任何好处。
虽然也可以工作，通信速度变慢，效率会降低。由于过多的中断开销，还不如传统的
CPU阻塞式软件模拟方案。




7-组件设计意义
非阻塞式工作方案，尽可能的节约CPU的时钟。
更适合有RTOS的软件结构。
更适合具有高频率的CPU的平台。


## 使用方式

void StartTask02(void *argument)
{
	uint8_t data[5];

	///init this module
	siic_init();
	
	///read
	data[0] = 0x70u; ///slave add+write
	data[1] = 0x02u; ///register
	data[2] = 0x71u; ///slave add+read
	data[3] = 0x00u; ///the read data will be here
	data[4] = 0x00u;
	siic_device_read_it(&hsiic1,&data[0],4);
	hsiic1_wait_cplt(100);

	///write	
	data[0] = 0x70u; ///slave add+write
	data[1] = 0x80u; ///register
	data[2] = 22u;     ///value to be written
	siic_device_write_it(&hsiic1,&data[0],3);
	hsiic1_wait_cplt(100);

	
	for(;;)
	{
	osDelay(100);
	}
}

## 结束





























