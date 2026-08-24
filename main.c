#include "stm32f10x.h"
#include "i2c.h"
#include "oled.h"
#include "delay.h"

void fengmingqi_init(void);
void i2c_init(void);
void oled_init(void);
void My_hcsr04_init(void);
int i2c_send_byte(uint8_t addr,const uint8_t *pData,uint16_t Size);
OLED_TypeDef oled = {0};
float distance = 0;
int main(void)
{
	fengmingqi_init();
	i2c_init();
	oled_init();
	OLED_SetPen(&oled,PEN_COLOR_WHITE,1);
	OLED_SetBrush(&oled,BRUSH_TRANSPARENT);//将背景设置为透明色
	OLED_SetCursor(&oled,24,35);
	OLED_DrawString(&oled,"dis:");
	OLED_SetFont(&oled,&default_font);
	OLED_SetCursor(&oled,52,35);
	OLED_Printf(&oled,"%.2f",distance);
	OLED_SendBuffer(&oled);
	My_hcsr04_init();
	while(1)
	{
		//1.向CNT写0
		TIM_SetCounter(TIM1,0);
		//2.清空cc1,cc2标志位
		TIM_ClearFlag(TIM1,TIM_FLAG_CC1);
		TIM_ClearFlag(TIM1,TIM_FLAG_CC2);
		//3.开启定时器
		TIM_Cmd(TIM1,ENABLE);
		//4.通过trig发送10微秒脉冲信号
		GPIO_WriteBit(GPIOB,GPIO_Pin_0,Bit_SET);
		DelayUs(10);
		GPIO_WriteBit(GPIOB,GPIO_Pin_0,Bit_RESET);
		//5.等待测量完成
		while(TIM_GetFlagStatus(TIM1,TIM_FLAG_CC1) == RESET);
		while(TIM_GetFlagStatus(TIM1,TIM_FLAG_CC2) == RESET);		
		//6.关闭定时器
		TIM_Cmd(TIM1,DISABLE);
		
		uint16_t ccr1 = TIM_GetCapture1(TIM1);
		uint16_t ccr2 = TIM_GetCapture2(TIM1);
		distance = (ccr2 - ccr1)*1.0e-6f*340.0f/2;
		OLED_SetCursor(&oled,24,35);
		OLED_DrawString(&oled,"dis:   m");
		OLED_SetFont(&oled,&default_font);
		OLED_SetCursor(&oled,52,35);
		OLED_Printf(&oled,"%.2f",distance);
		OLED_SendBuffer(&oled);
		if(distance <= 0.05)
		{
			GPIO_WriteBit(GPIOB,GPIO_Pin_12,Bit_RESET);			
		}
		else
		{
			GPIO_WriteBit(GPIOB,GPIO_Pin_12,Bit_SET);	
		}
		Delay(300);
		OLED_Clear(&oled);
	}
}











void fengmingqi_init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitTypeDef gpio_i = {0};
	gpio_i.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio_i.GPIO_Pin = GPIO_Pin_12;
	gpio_i.GPIO_Speed = GPIO_Speed_10MHz;
	
	GPIO_Init(GPIOB,&gpio_i);
	
	GPIO_WriteBit(GPIOB,GPIO_Pin_12,Bit_SET);
}
void i2c_init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitTypeDef gpio_i = {0};
	gpio_i.GPIO_Mode = GPIO_Mode_AF_OD;
	gpio_i.GPIO_Pin = GPIO_Pin_6;
	gpio_i.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB,&gpio_i);
	gpio_i.GPIO_Mode = GPIO_Mode_AF_OD;
	gpio_i.GPIO_Pin = GPIO_Pin_7;
	gpio_i.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB,&gpio_i);
	I2C_InitTypeDef i2c_i = {0};
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,ENABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1,ENABLE);//施加复位信号
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1,DISABLE);//释放复位信号
	i2c_i.I2C_ClockSpeed = 400000;
	i2c_i.I2C_Mode = I2C_Mode_I2C;
	i2c_i.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_Init(I2C1,&i2c_i);
	
	I2C_Cmd(I2C1,ENABLE);
}
int i2c_send_byte(uint8_t addr,const uint8_t *pData,uint16_t Size)
{
	My_I2C_SendBytes(I2C1,addr,pData,Size);
	return 0;
}
void oled_init(void)
{
	OLED_InitTypeDef oled_i = {0};
	oled_i.i2c_write_cb = i2c_send_byte;
	OLED_Init(&oled,&oled_i);
}
void My_hcsr04_init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);
	TIM_TimeBaseInitTypeDef tim_i = {0};
	tim_i.TIM_CounterMode = TIM_CounterMode_Up;
	tim_i.TIM_Period = 65535;
	tim_i.TIM_Prescaler = 71;
	tim_i.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(TIM1,&tim_i);
	
	//初始化输入捕获
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitTypeDef gpio_i = {0};
	gpio_i.GPIO_Mode = GPIO_Mode_IPD;
	gpio_i.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOA,&gpio_i);
	
	TIM_ICInitTypeDef tim_ici = {0};
	tim_ici.TIM_Channel = TIM_Channel_1;
	tim_ici.TIM_ICFilter = 0;//输入滤波器
	tim_ici.TIM_ICPolarity = TIM_ICPolarity_Rising;
	tim_ici.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	tim_ici.TIM_ICSelection = TIM_ICSelection_DirectTI;
	
	TIM_ICInit(TIM1,&tim_ici);
	
	tim_ici.TIM_Channel = TIM_Channel_2;
	tim_ici.TIM_ICFilter = 0;//输入滤波器
	tim_ici.TIM_ICPolarity = TIM_ICPolarity_Falling;
	tim_ici.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	tim_ici.TIM_ICSelection = TIM_ICSelection_IndirectTI;
	
	TIM_ICInit(TIM1,&tim_ici);
	
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
		gpio_i.GPIO_Mode = GPIO_Mode_Out_PP;
		gpio_i.GPIO_Speed = GPIO_Speed_2MHz;
		gpio_i.GPIO_Pin = GPIO_Pin_0;
		GPIO_Init(GPIOB,&gpio_i);
}
