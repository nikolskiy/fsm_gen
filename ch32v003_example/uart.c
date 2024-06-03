#include "uart.h"

#include <ch32v003fun.h>
#include <stdint.h>
#include <stdio.h>

#define BUF_SIZE 16

static char input_buffer[BUF_SIZE];
uint32_t input_idx = 0;
static char output_buffer[BUF_SIZE];

static void DMA_Deinit()
{
    DMA1_Channel4->CFGR &= (uint16_t)(~DMA_CFG4_EN);
    DMA1_Channel4->CFGR = 0;
    DMA1_Channel4->CNTR = 0;
    DMA1_Channel4->PADDR = 0;
    DMA1_Channel4->MADDR = 0;

    DMA1->INTFCR |= ((uint32_t)(DMA_GIF4 | DMA_TCIF4 | DMA_HTIF4 | DMA_TEIF4));
}

static void DMA_Init()
{
    register uint32_t cfg = 0;

    RCC->AHBPCENR |= RCC_AHBPeriph_DMA1;

    cfg = DMA1_Channel4->CFGR;

    cfg &= CFGR_CLEAR_Mask;

    cfg |= DMA_DIR_PeripheralDST | DMA_PeripheralInc_Disable | DMA_PeripheralDataSize_Byte |
           DMA_MemoryInc_Enable | DMA_MemoryDataSize_Byte | DMA_M2M_Disable |
           DMA_Mode_Normal | DMA_Priority_VeryHigh;

    DMA1_Channel4->CFGR = cfg;

    DMA1_Channel4->PADDR = (uint32_t)(&USART1->DATAR);
    DMA1_Channel4->MADDR = (uint32_t)(output_buffer);
    DMA1_Channel4->CNTR = 0;

    NVIC_EnableIRQ(DMA1_Channel4_IRQn);
    DMA1_Channel4->CFGR |= DMA_IT_TC | DMA_CFG4_EN;
}

void DMA1_Channel4_IRQHandler(void) __attribute__((interrupt));
void DMA1_Channel4_IRQHandler(void) {
    if(DMA1->INTFR & DMA1_FLAG_TC4) {
        DMA1->INTFCR = DMA_CTCIF4;
        uart_send_complete();
    }
}

void uart_send(const char* buf, uint32_t size)
{
  char *op = output_buffer;
  register uint32_t sz = size;
  for(; sz ; --sz) {
      *op++ = *buf++;
  }
  DMA1_Channel4->CNTR = (uint32_t)size;
}

void uart_init()
{
    DMA_Deinit();
    DMA_Init();

    RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1;

    funPinMode(PD5, GPIO_CFGLR_OUT_10Mhz_AF_PP);
    funPinMode(PD6, GPIO_CFGLR_IN_PUPD);
    DYN_GPIO_MOD(GPIOD, OUTDR, ODR6, 1);

    // 8N1
    USART1->CTLR1 = USART_WordLength_8b | USART_Parity_No | USART_Mode_Tx | USART_Mode_Rx;
    USART1->CTLR2 = USART_StopBits_1;
    USART1->CTLR3 = USART_HardwareFlowControl_None;

    USART1->BRR = UART_BRR; // 115200, from ch32v003fun.h

    NVIC_EnableIRQ(USART1_IRQn);

    USART1->CTLR1 |= USART_CTLR1_RXNEIE;
    USART1->CTLR1 |= CTLR1_UE_Set;
    USART1->CTLR3 |= USART_DMAReq_Tx;
}

void USART1_IRQHandler(void) __attribute__((interrupt));
void USART1_IRQHandler(void)
{
    if(USART1->STATR & USART_STATR_RXNE) {;
        char c = USART1->DATAR & 0xFF;
        c = c < ' ' ? 0 : c;
        input_buffer[input_idx++] = c;
        if(c == 0) {
            uart_received_line(input_buffer);
            input_idx=0;
        }
    }
}
