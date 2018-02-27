/*
 * net.c
 *
 *  Created on: 12 ����. 2018 �.
 *      Author: folla
 */

#include "net.h"
#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_flash.h"

uint32_t PROGRAM_STRART_ADRESS = 0x8020000;
typedef  void (*pFunction)(void);
pFunction Jump_To_Application;
uint32_t JumpAddress;
/*------------------------------------------------------*/

static char *commands[]={
"clear",
"avail",
"none",
"done",
"connected",
"uploaded",
"ready"
};

int flashFlag = 0;
int connectionFlag = 0;
int offcet = 0;
int connectionTry = 0;
int i = 0;
int filesize;
int sizeCount = 0;
int readyToJump = 0;
uint16_t totLen = 0;

static struct tcp_pcb *server_pcb;

static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
//static void tcp_server_error(void *arg, err_t err);
static err_t tcp_server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
//static err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
//static err_t tcp_server_poll(void *arg, struct tcp_pcb *tpcb);


//static struct tcp_pcb *client_pcb;
//static err_t tcp_client_connected(void *arg, struct tcp_pcb *pcb, err_t err);
//static err_t tcp_client_recieve(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
void jumpToMainFirmware(){
	  /*--------------Jump to application------------------*/
		void (*GoToApp)(void);
		JumpAddress = *((volatile uint32_t*)(USER_FLASH_FIRST_PAGE_ADDRESS + 4));
		GoToApp = (void (*)(void))JumpAddress;
		SCB->VTOR = USER_FLASH_FIRST_PAGE_ADDRESS;
		 __set_MSP(*((volatile uint32_t*) USER_FLASH_FIRST_PAGE_ADDRESS)); //stack pointer (to RAM) for USER app in this address
		GoToApp();
		/*----------------------------------------------------*/
}

static void FLASH_Program_Byte(uint32_t Address, uint8_t Data)
{
  /* Check the parameters */
  assert_param(IS_FLASH_ADDRESS(Address));

  /* If the previous operation is completed, proceed to program the new data */
  FLASH->CR &= CR_PSIZE_MASK;
  FLASH->CR |= FLASH_PSIZE_BYTE;
  FLASH->CR |= FLASH_CR_PG;

  *(__IO uint8_t*)Address = Data;

  /* Data synchronous Barrier (DSB) Just after the write operation
     This will force the CPU to respect the sequence of instruction (no optimization).*/
  __DSB();
}

//uint32_t FLASH_Read(uint32_t address)
//{
//    return (*(__IO uint32_t*)address);
//}

void tcp_server_init(void)
 {

   server_pcb = tcp_new();

   if (server_pcb != NULL)
   {
     err_t err;
     err = tcp_bind(server_pcb, IP_ADDR_ANY, 7);
     if (err == ERR_OK)
     {
       server_pcb = tcp_listen(server_pcb);
       tcp_accept(server_pcb, tcp_server_accept);
     }
     else
     {
       memp_free(MEMP_TCP_PCB, server_pcb);
     }
   }
 }

 static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
 {
	 tcp_setprio(newpcb, TCP_PRIO_MIN);
	 tcp_recv(newpcb, tcp_server_recv);
	 connectionFlag = 1;
	 return err;
 }

 static err_t tcp_server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err){
	 	unsigned char *data = p->payload;
	 	tcp_recved(pcb, p->tot_len);
	 	int i=0;
	 	  if(err == ERR_OK && p != NULL)
	 	  	{
	 		  if (flashFlag == 1 ){
	 					  for (i = 0; i < p->len; i++)
	 					  	  {
	 						  HAL_FLASH_Unlock();
	 						  if (data[i] == 'd' && data[i+1] == 'a' && data[i+2] == 't' && data[i+3] == 'e' && data[i+4] == 'n' && data[i+5] == 'd'){
	 							 tcp_write(pcb, commands[5], strlen(commands[5]), 0);
	 							 tcp_output(pcb);
	 							  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
	 							  HAL_Delay(500);
	 							  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
	 							  readyToJump = 1;
	 						  }else if (readyToJump == 0){
	 							  FLASH_Program_Byte(PROGRAM_STRART_ADRESS+i, data[i]);
	 						  }
	 						  HAL_FLASH_Lock();
	 					  	  }
	 					  PROGRAM_STRART_ADRESS = PROGRAM_STRART_ADRESS + i;
	 				  }
//	 					  if (getGileSizeFlag == 1){//function for file size receive
//	 						  filesize = (int)data;
//	 						  flashFlag = 1;
//	 						 getGileSizeFlag=0;
//	 					  } else
	 		  if (strcmp(data,commands[0]) == 0){ //if "clear" received
	 					  HAL_FLASH_Unlock();
	 					  //erasing of 640kB
	 					  FLASH_Erase_Sector(0x08020000, FLASH_VOLTAGE_RANGE_3);//erase 4 sec
	 					  FLASH_Erase_Sector(0x0803F400, FLASH_VOLTAGE_RANGE_3);//erase 5 sec
	 					  FLASH_Erase_Sector(FLASH_SECTOR_6, FLASH_VOLTAGE_RANGE_3);//erase 6 sec
	 					  HAL_FLASH_Lock();
	 					  flashFlag = 1; // ready to flash
	 					  char *msg = commands[6];
	 					  tcp_write(pcb, msg, strlen(msg), 0);
	 					  tcp_output(pcb);
	 					 pbuf_free(p);
	 				  }else
	 		  if (strcmp(data,commands[2]) == 0){//if "none" received (not used)
	 					  //tcp_client_connection_close(p);
	 					 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
	 					 HAL_Delay(500);
	 					 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
	 					 jumpToMainFirmware();
	 				  }
	 	  	}

	  pbuf_free(p);

	  return ERR_OK;
 }


//static err_t tcp_client_connected(void *arg, struct tcp_pcb *pcb, err_t err) //������� �����������
//{
//	connectionFlag = 1;
//	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
//	HAL_Delay(50);
//	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
//
//	tcp_write(pcb, commands[4], strlen(commands[4]), 0);
//	tcp_output(pcb);
//	return err;
//}
//
//
//static void tcp_client_connection_close(struct tcp_pcb *pcb)
//
//{
//  /* remove callbacks */
//  tcp_recv(pcb, NULL);
//  tcp_sent(pcb, NULL);
//  /* close tcp connection */
//  tcp_close(pcb);
//}
//
//static void tcp_client_error(void *arg, err_t err){
//	connection_to_server();
//}
//
//static err_t tcp_client_recieve(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err){ //���� �������� ��� �� � ����
//	unsigned char *data = p->payload; //� p->payload ����� ���������� ������
//
//	int i=0;
//	  if(err == ERR_OK && p != NULL && connectionFlag == 1)
//	  	{
//		  if (flashFlag == 1){ //���� ���� ����� � ������ � ������
//					  for (i = 0; i < p->len; i++)
//					  	  {
//						  HAL_FLASH_Unlock();
//
//						  if (data[i] == 'e' && data[i+1] == 'n' && data[i+2] == 'd' ){
//							  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
//							  HAL_Delay(500);
//							  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
//							  HAL_FLASH_Unlock();
//							  tcp_client_connection_close(p);
//							  HAL_Delay(100);
//							  jumpToMainFirmware();
//						  }
//						  FLASH_Program_Byte(PROGRAM_STRART_ADRESS+i, data[i]);
//						  HAL_FLASH_Lock();
//					  	  }
//					  PROGRAM_STRART_ADRESS = PROGRAM_STRART_ADRESS + i;
//				  }else
//		  if (strcmp(data,commands[0]) == 0){ //���� ������ ������� �� �������� �����
//					  HAL_FLASH_Unlock();
//					  FLASH_Erase_Sector(0x8020000, FLASH_VOLTAGE_RANGE_3); //������� ������
//					  FLASH_Erase_Sector(0x8030000, FLASH_VOLTAGE_RANGE_3); //������� ������
//					  FLASH_Erase_Sector(0x8040000, FLASH_VOLTAGE_RANGE_3); //������� ������
//					  HAL_FLASH_Lock();
//					  flashFlag = 1;
//					  char *msg = commands[3];
//					  tcp_write(pcb, msg, strlen(msg), 0);
//					  tcp_output(pcb);
//				  }else if (strcmp(data,commands[2]) == 0){//���� ����� �������� ���
//					  tcp_client_connection_close(p);
//					  HAL_Delay(100);
//					  jumpToMainFirmware();
//				  }
//
//	  	}
//
//	  tcp_recved(pcb, p->tot_len);
//	  pbuf_free(p);
//	  return ERR_OK;
//}
//
//void connection_to_server (){ //������������� ����������
//if (connectionFlag==0){
////	HAL_Delay(500);
////	tcp_client_connection_close(client_pcb);
//	//tcp_connect(pcb, NULL);
//		ip_addr_t server_ip;
//		IP4_ADDR(&server_ip, 192,168,0,2);
//		client_pcb = tcp_new();
//		tcp_recv(client_pcb, tcp_client_recieve);
//		tcp_err(client_pcb, tcp_client_error);
//		tcp_connect(client_pcb, &server_ip, 9090, tcp_client_connected);
//		//connectionTry++;
//	}
//
//}

