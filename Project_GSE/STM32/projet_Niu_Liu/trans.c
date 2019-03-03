#include "stm32f4xx.h"                  // Device header
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "tm_stm32f4_delay.h"


char str[50];
uint32_t i;
//char* s;
uint16_t distance;

/*la partie uart*/
void USART_Puts(USART_TypeDef* USARTx,volatile char *s) 
{
 while(*s)
 {
  while(!(USARTx ->SR & 0x00000040));//envoyer->0x04
	 USART_SendData(USARTx,*s);
	 *s++;
 }	 
}


int USART2_transmitter_empty(void){ 
	return ( USART_GetFlagStatus(USART2, USART_FLAG_TXE) == SET );
}


void USART2_puts(char *s){ 
	while( *s ){ 
		if(USART2_transmitter_empty()) { 
				USART_SendData(USART2, *s);
				s++; 
		} 
	} 
}


void UART_Initialisation(){
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;	
		/*configuration uart*/
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE); 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);//P2A
	
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF; 
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz;
	
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	//juste connecter PA2 avec rx  parce que c'est la carte qui envoie les donneés , uart recoit les donneés
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);
	
	USART_InitStructure.USART_BaudRate=115200; 
	USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode=USART_Mode_Tx;
	USART_InitStructure.USART_Parity=USART_Parity_No;
	USART_InitStructure.USART_StopBits=USART_StopBits_1;
	USART_InitStructure.USART_WordLength=USART_WordLength_8b;
	USART_Init(USART2,&USART_InitStructure);
	USART_Cmd(USART2,ENABLE);
}


/*la partie I2C*/
void I2C1_Initialize(void)
{
	I2C_InitTypeDef I2C_init1;
	GPIO_InitTypeDef GPIO_InitStructure1;
	GPIO_InitTypeDef GPIO_InitStructure2;
	
	I2C_init1.I2C_ClockSpeed = 100000;
	I2C_init1.I2C_Mode = I2C_Mode_I2C;
	I2C_init1.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_init1.I2C_OwnAddress1=0;
	I2C_init1.I2C_Ack = I2C_Ack_Disable;
	I2C_init1.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	
	//GPIO_PinConfigure(GPIOB, 6, GPIO_MODE_AF, GPIO_OUTPUT_OPEN_DRAIN, GPIO_GPOUTPUT_SPEED_50MHz, GPIO_PULL_UP); 

	
	 GPIO_InitStructure1.GPIO_Pin=GPIO_Pin_6;
	 GPIO_InitStructure1.GPIO_Mode=GPIO_Mode_AF;
	 GPIO_InitStructure1.GPIO_OType=GPIO_OType_OD;
	 GPIO_InitStructure1.GPIO_PuPd=GPIO_PuPd_UP;
	 GPIO_InitStructure1.GPIO_Speed=GPIO_Fast_Speed;	
	 GPIO_Init(GPIOB,&GPIO_InitStructure1);
		//GPIO_PinConfigure(GPIOB, 7, GPIO_MODE_AF, GPIO_OUTPUT_OPEN_DRAIN, GPIO_OUTPUT_SPEED_50MHz, GPIO_PULL_UP); 
  
	 GPIO_InitStructure2.GPIO_Pin=GPIO_Pin_7;
	 GPIO_InitStructure2.GPIO_Mode=GPIO_Mode_AF;
	 GPIO_InitStructure2.GPIO_OType=GPIO_OType_OD;
	 GPIO_InitStructure2.GPIO_PuPd=GPIO_PuPd_UP;
	 GPIO_InitStructure2.GPIO_Speed=GPIO_Fast_Speed;	
	 GPIO_Init(GPIOB,&GPIO_InitStructure2);

	
	//GPIO_PinAF(GPIOB, 6, GPIO_AF_I2C1);   SCL  green
	//GPIO_PinAF(GPIOB, 7, GPIO_AF_I2C1);     SDA  blue
	GPIO_PinAFConfig(GPIOB, 6,GPIO_AF_I2C1);
	GPIO_PinAFConfig(GPIOB, 7,GPIO_AF_I2C1);
	
	I2C_Init(I2C1, &I2C_init1);
	
	I2C_Cmd(I2C1, ENABLE);	
}




void I2C_start(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction)
{
	while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY) == SET);
	I2C_GenerateSTART(I2Cx, ENABLE);
	while(I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT) == ERROR);
	I2C_Send7bitAddress(I2Cx, address, direction);
	if (direction == I2C_Direction_Transmitter){
		while(I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) == ERROR);
	} else if (direction == I2C_Direction_Receiver){
		while(I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) == ERROR);
	}
}

uint16_t get_distance(){
	uint8_t mask;
  uint8_t receive;
	uint8_t distance_faible;
	uint8_t distance_fort;
	uint16_t distance;
	
  I2C_start(I2C1, 0x62<<1, I2C_Direction_Transmitter);
	I2C_SendData(I2C1, 0x00);
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) == ERROR);
	I2C_SendData(I2C1, 0x01);
	I2C_GenerateSTOP(I2C1, ENABLE);
	
	I2C_start(I2C1, 0x62<<1, I2C_Direction_Transmitter);
	I2C_SendData(I2C1, 0x04);
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) == ERROR);
	I2C_SendData(I2C1, 0x08);
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) == ERROR);
	I2C_GenerateSTOP(I2C1, ENABLE);

	I2C_start(I2C1, 0x62<<1, I2C_Direction_Transmitter);
	I2C_SendData(I2C1, 0x01);
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) == ERROR);
	I2C_GenerateSTOP(I2C1, ENABLE);
	
	
	I2C_start(I2C1, 0x62<<1, I2C_Direction_Receiver);
	mask=0x00000001;
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) == ERROR);
	receive=I2C_ReceiveData(I2C1);
	I2C_GenerateSTOP(I2C1, ENABLE);
	
	
	  while((receive&mask)!=0){
		I2C_start(I2C1, 0x62<<1, I2C_Direction_Transmitter);
		I2C_SendData(I2C1, 0x01);
		while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) == ERROR);
		I2C_GenerateSTOP(I2C1, ENABLE);
		
		I2C_start(I2C1, 0x62<<1, I2C_Direction_Receiver);
		while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) == ERROR);	
		receive=I2C_ReceiveData(I2C1);
		I2C_GenerateSTOP(I2C1, ENABLE);	
	}
		
	//while((receive&mask)!=0);
	I2C_start(I2C1, 0x62<<1, I2C_Direction_Transmitter);
	I2C_SendData(I2C1, 0x10);
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) == ERROR);
	I2C_GenerateSTOP(I2C1, ENABLE);
	I2C_start(I2C1, 0x62<<1, I2C_Direction_Receiver);
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) == ERROR);
	distance_faible=I2C_ReceiveData(I2C1);
	I2C_GenerateSTOP(I2C1, ENABLE);
	
	I2C_start(I2C1, 0x62<<1, I2C_Direction_Transmitter);
	I2C_SendData(I2C1, 0x0f);
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) == ERROR);
	I2C_GenerateSTOP(I2C1, ENABLE);
	I2C_start(I2C1, 0x62<<1, I2C_Direction_Receiver);
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) == ERROR);	
	distance_fort=I2C_ReceiveData(I2C1);
	
	distance= (distance_fort<<8)+distance_faible;
	I2C_GenerateSTOP(I2C1, ENABLE);

	
	return distance;
	
}


//Start button
void buttonInitialization(){
	
	GPIO_InitTypeDef GPIO_InitDef;
	GPIO_InitDef.GPIO_Pin=GPIO_Pin_0;
	GPIO_InitDef.GPIO_Mode=GPIO_Mode_IN;
	GPIO_InitDef.GPIO_OType=GPIO_OType_PP;
	GPIO_InitDef.GPIO_PuPd=GPIO_PuPd_DOWN;
	GPIO_InitDef.GPIO_Speed=GPIO_Speed_50MHz;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	GPIO_Init(GPIOA,&GPIO_InitDef);	
}

//partie pwm


//pwm
__IO uint32_t usTick=2500;
void SysTick_Handler(){
usTick--;
	GPIO_SetBits(GPIOB,GPIO_Pin_9);
	if(usTick==0){
	  GPIO_ResetBits(GPIOB,GPIO_Pin_9);
		usTick=2500;
	}
}


void PWMinit(){
	 GPIO_InitTypeDef GPIO_InitStructure1;
	 GPIO_InitTypeDef GPIO_InitStructure2;
	
	 RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	 GPIO_InitStructure1.GPIO_Pin=GPIO_Pin_9;
	 GPIO_InitStructure1.GPIO_Mode=GPIO_Mode_OUT;
	 GPIO_InitStructure1.GPIO_OType=GPIO_OType_OD;
	 GPIO_InitStructure1.GPIO_PuPd=GPIO_PuPd_UP;
	 GPIO_InitStructure1.GPIO_Speed=GPIO_Fast_Speed;	
	 GPIO_Init(GPIOB,&GPIO_InitStructure1);
	
	 GPIO_InitStructure2.GPIO_Pin=GPIO_Pin_8;
	 GPIO_InitStructure2.GPIO_Mode=GPIO_Mode_OUT;
	 GPIO_InitStructure2.GPIO_OType=GPIO_OType_OD;
	 GPIO_InitStructure2.GPIO_PuPd=GPIO_PuPd_UP;
	 GPIO_InitStructure2.GPIO_Speed=GPIO_Fast_Speed;	
	 GPIO_Init(GPIOB,&GPIO_InitStructure2);	
	
}
/*
void PWMconfig(){	
	//direction pb8  step pb9
		GPIO_SetBits(GPIOB,GPIO_Pin_9);
	//	Delay(2500);

		//gnd->vcc      enable->gnd 5v
		GPIO_ResetBits(GPIOB,GPIO_Pin_9);
			
		//}	
}
*/
int main(void)
{

	//configure PWM	
	  //PWMinit();
		buttonInitialization();
	  UART_Initialisation();
		//GPIO_SetBits(GPIOB,GPIO_Pin_8);
   	SystemCoreClockUpdate();
		
		GPIO_SetBits(GPIOB,GPIO_Pin_8);
	
	//configuration de I2C
 //while(1){
	//if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)){
	I2C1_Initialize();

 while(1){
	 //mesure distance
	  SysTick_Config(SystemCoreClock / 1000000);
	  distance=get_distance();
		sprintf(str,"%d\n",distance);
		USART_Puts(USART2,str);
		//PWMconfig();
		
		//	}
	//	}
	}
}
