/**************************************************************************************************
 
  Phyplus Microelectronics Limited confidential and proprietary. 
  All rights reserved.

  IMPORTANT: All rights of this software belong to Phyplus Microelectronics 
  Limited ("Phyplus"). Your use of this Software is limited to those 
  specific rights granted under  the terms of the business contract, the 
  confidential agreement, the non-disclosure agreement and any other forms 
  of agreements as a customer or a partner of Phyplus. You may not use this 
  Software unless you agree to abide by the terms of these agreements. 
  You acknowledge that the Software may not be modified, copied, 
  distributed or disclosed unless embedded on a Phyplus Bluetooth Low Energy 
  (BLE) integrated circuit, either as a product or is integrated into your 
  products.  Other than for the aforementioned purposes, you may not use, 
  reproduce, copy, prepare derivative works of, modify, distribute, perform, 
  display or sell this Software and/or its documentation for any purposes.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  PHYPLUS OR ITS SUBSIDIARIES BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
  
**************************************************************************************************/



#include "rf_phy_driver.h"
#include "global_config.h"
#include "jump_function.h"

#include "ll_debug.h"
#include "ll_hw_drv.h"
#include "ll_sleep.h"

#include "config.h"
#include "uart.h"
#include "gpio.h"
#include "common.h"
#include "log.h"
#include "flash.h"
#include "clock.h"
#include "Sensor_Broadcast.h"
#include "pwrmgr.h"
#include "error.h"
#include "pwrmgr.h"

#define   SYSTEM_ON_TIMER       3000


extern void  ble_main(void);

extern int app_main(void);
extern int hal_rom_code_ini(void);
extern void init_config(void);
extern uint32 *pGlobal_config;      
extern  uint32_t pclk;
extern uint8_t io_wakeup_flag;
extern uint8 Sensor_Broadcast_TaskID;
extern uint32_t osal_sys_tick;
extern uint8 advertData[];

//uint8_t rx_length=0;

static void gpio_pos_wakeuphandler(GPIO_Pin_e pin,IO_Wakeup_Pol_e type){
	if(type==POSEDGE){
  }
	else if(type==NEGEDGE){
		}
	
	}

static void rf_wakeup_handler(void){
  NVIC_SetPriority((IRQn_Type)BB_IRQ, IRQ_PRIO_REALTIME);
  NVIC_SetPriority((IRQn_Type)CP_TIMER_IRQ, IRQ_PRIO_HIGH);
}

static void hal_rfphy_init(void)
{
    //============config the txPower
    g_rfPhyTxPower  = RF_PHY_TX_POWER_EXTRA_MAX ;
    //============config BLE_PHY TYPE
    g_rfPhyPktFmt   = PKT_FMT_BLE1M;
    //============config RF Frequency Offset
    g_rfPhyFreqOffSet   =RF_PHY_FREQ_FOFF_00KHZ;

    hal_rom_code_ini();
    
    //Quick Boot setting and 
     *(volatile uint32_t *) 0x4000f01c = 0x0000004;       //  3'b1xx: 62.5us.  control bits for main digital part reset wait time after power up charge pump. 

    //========= low currernt setting IO init
    //========= pull all io to gnd by default
    *(volatile uint32_t *) 0x4000f008 = 0x36db6db6;//P00 - P09 pull down
    *(volatile uint32_t *) 0x4000f00c = 0x36db6db6;//P10 - P19 pull down
    *(volatile uint32_t *) 0x4000f010 = 0x36db6db6;//P20 - P29 pull down
    *(volatile uint32_t *) 0x4000f014 = 0xb0c3edb6;//P30 - P34 pull donw

    //========= UART RX Pull up
    hal_gpio_pull_set(P10,WEAK_PULL_UP);    

    DCDC_CONFIG_SETTING(0x0d);


    NVIC_SetPriority((IRQn_Type)BB_IRQ, IRQ_PRIO_REALTIME);
    NVIC_SetPriority((IRQn_Type)CP_TIMER_IRQ, IRQ_PRIO_HIGH);

    hal_pwrmgr_register(MOD_USR0, NULL, rf_wakeup_handler);

}


static void hal_init(void)
{
    hal_system_init(g_system_clk);//(SYS_CLK_DLL_32M);//SYS_CLK_XTAL_16M);   //system init

    hal_rtc_clock_config(CLK_32K_XTAL);
      
    hal_pwrmgr_RAM_retention(RET_SRAM0|RET_SRAM1);
    
    uart_Cfg_t cfg = {    
        .tx_pin = P9,
        .rx_pin = P10,
        .rts_pin = GPIO_DUMMY,
        .cts_pin = GPIO_DUMMY,
        .baudrate = 115200,
        .use_fifo = TRUE,
        .hw_fwctrl = FALSE,
        .use_tx_buf = FALSE,
        .parity = FALSE,
        .evt_handler = NULL,
    };
    hal_uart_init(cfg);
    hal_gpio_init();
    hal_pwrmgr_register(MOD_USR0, NULL, rf_wakeup_handler);
   
    hal_gpioin_register(P14,gpio_pos_wakeuphandler,NULL);

    hal_gpio_pull_set(P9,FLOATING);
//    hal_gpio_pull_set(P18,STRONG_PULL_UP);
}
    
int  main(void)  
{   
    g_system_clk = SYS_CLK_XTAL_16M;
//    user_start_handler();
    system_on_handler(P14,SYSTEM_ON_TIMER);   //3s system on,you can programe it

    
  
    init_config();

    hal_pwrmgr_init();
    
    hal_rfphy_init();
  
    hal_init();

//    LOG("Bridge iBeacon begin!\n");
   
    app_main();
}
///////////////////////////////////  end  ///////////////////////////////////////
