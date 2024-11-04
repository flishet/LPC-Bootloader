#include "LPC177x_8x.h"
#include "lpc177x_8x_uart.h"
#include "lpc177x_8x_timer.h"
#include <string.h>

#define IAP_LOCATION 0x1FFF1FF1
#define APP_START_ADDRESS 0x2000


char erase=0,write=0,reset=0;

uint32_t start=2;
uint32_t end=20;


typedef void (*IAP)(unsigned int[], unsigned int[]);
IAP iap_entry = (IAP)IAP_LOCATION;


void erase_sector(uint32_t start_sector, uint32_t end_sector) 
{
    unsigned int command[5], result[4];
    command[0] = 50;
    command[1] = start_sector;
    command[2] = end_sector;
    __disable_irq(); 
    iap_entry(command, result);
    __enable_irq();
	
    if (result[0] != 0) 
			{

			}
			
		command[0] = 52;
    command[1] = start_sector;
    command[2] = end_sector;
    command[3] = SystemCoreClock / 1000;
    __disable_irq(); 
    iap_entry(command, result);
    __enable_irq();
}


void CopyRAMToFlash(uint32_t flash_address, uint8_t *data, uint32_t size) 
{
    unsigned int command[5];
    unsigned int result[4];
    
    command[0] = 50;
    command[1] = flash_address / 4096;
    command[2] = flash_address / 4096;
    iap_entry(command, result);    
    if (result[0] == 0) 
			{ 

			}
		command[0] = 51;
		command[1] = flash_address;
		command[2] = (uint32_t)data;
		command[3] = size;
		command[4] = SystemCoreClock / 1000;
		iap_entry(command, result);        
		if (result[0] != 0) 
			{

			}
}

void WriteToFlash(uint32_t FlashAddress,uint8_t *data_ptr,uint32_t Lenght)
{
uint8_t buffer[256];
while (Lenght > 0) 
	{
		uint32_t write_size = (Lenght >= 256) ? 256 : Lenght;						
		
		if(write_size<256)
		{
			memcpy(buffer, data_ptr, write_size);
			memset(buffer + write_size, 255, 256 - write_size);
			CopyRAMToFlash(FlashAddress, buffer, 256);
		}
		else
		{
		CopyRAMToFlash(FlashAddress, data_ptr, 256);
		}
		
		FlashAddress += write_size;
		data_ptr += write_size;
		Lenght -= write_size;
	}
}


void jump_to_application(uint32_t app_address) 
{
typedef void (*FunctionPointer)(void);
    uint32_t app_stack = *(volatile uint32_t*) app_address;
    uint32_t app_reset_handler = *(volatile uint32_t*) (app_address+4);
    __set_MSP(app_stack);
		FunctionPointer Aplication = (FunctionPointer) app_reset_handler;
		Aplication();
}


void TIMER1_IRQHandler(void)
{
	TIM_ClearIntPending(LPC_TIM1, TIM_MR0_INT);
}


//uint32_t data[64];
//uint32_t flash_address = 0x2000;
//uint32_t remaining_data = 3968; 


int main() 
	{	
		uint32_t stack = *(volatile uint32_t*) APP_START_ADDRESS;
		if(stack != 0xffffffff)
		{
			jump_to_application(APP_START_ADDRESS);
		}
	
    TIM_TIMERCFG_Type TIM_ConfigStruct;
    TIM_MATCHCFG_Type TIM_MatchConfigStruct ;

    TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_TICKVAL;
    TIM_ConfigStruct.PrescaleValue	= 1;	
    TIM_MatchConfigStruct.IntOnMatch   = TRUE;
		TIM_MatchConfigStruct.MatchChannel = 0;
    TIM_MatchConfigStruct.ResetOnMatch = TRUE;
    TIM_MatchConfigStruct.StopOnMatch  = FALSE;
    TIM_MatchConfigStruct.MatchValue = 120000;
    TIM_Init(LPC_TIM1, TIM_TIMER_MODE, &TIM_ConfigStruct);
    TIM_ConfigMatch(LPC_TIM1, &TIM_MatchConfigStruct);
    NVIC_SetPriority(TIMER1_IRQn,0);
    NVIC_EnableIRQ(TIMER1_IRQn);
    TIM_Cmd(LPC_TIM1,ENABLE);
		
		
    while(1)
		{
			if(erase==1)
			{
				erase=0;
				erase_sector(start,end);				
			}
//			if(reset==1)
//			{
//				reset=0;
//				jump_to_application(APP_START_ADDRESS);
//			}
//			
//			if(write==1)
//			{
//				write=0;
//			}
		}
}