
// I HAVE INCLUDED THE LIBRARIES THAT ARE GIVEN IN THE MAIN FILE OF THE LORA MODULE

#include "msp430.h"
#include "LoRaWan.h"
#include <stdint.h>
//#include <iostream>
#include "tpl_os.h"
//#include <msp430.h>

//#include <stdio.h>


LoRaWan LoraRadio;   // I AM CREATING A GLOBAL OBJECT FOR THE LORAWAN CLASS




// OK HERE I CREATED A LORAWAN OBJECT "LoraRadio"






//--- LORA ABP KEYS parameters----------------------------------------------------------------------------------------------//
// Network Session Key (MSB)
uint8_t NwkSkey[16] = { 0x01, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
// Application Session Key (MSB)
uint8_t AppSkey[16] = { 0x01, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
// Device Address (MSB)
uint8_t DevAddr[4] = { 0x11, 0x22, 0x33, 0x44 };
//--------------------------------------------------------------------------------------------------------------------------//





#define APP_ISR_buttonS2_START_SEC_CODE
#include "tpl_memmap.h"








FUNC(void, OS_APPL_CODE) ioInit(){




	// Disable the GPIO power-on default high-impedance mode
	// to activate previously configured port settings
	PM5CTL0 &= ~LOCKLPM5;



    WDTCTL = WDTPW | WDTHOLD;                   // Stop WatchDog timer

    // Configure GPIO
    P1OUT = 0; P1DIR = 0xFF;
    P2OUT = 0; P2DIR = 0xFF;
    P3OUT = 0; P3DIR = 0xFF;
    P4OUT = 0; P4DIR = 0xFF;
   // P5OUT = 0; P5DIR = 0xFF;
    //P6OUT = 0; P6DIR = 0xFF;
    //P7OUT = 0; P7DIR = 0xFF;
    //P8OUT = 0; P8DIR = 0xFF;
    //P9OUT = 0; P9DIR = 0xFF;
    //P10OUT = 0;P10DIR = 0xFF;

    PJOUT = 0;
    PJSEL0 = BIT4 | BIT5;                     // For XT1 - LFXT 32Khz Osc
    PJDIR = 0xFFFF;

    PMMCTL0_H = PMMPW_H;                      // Unlock PMM registers
    PM5CTL0 &= ~LOCKLPM5;                     // Unlock I/O pins
    PMMCTL0_H = 0x00;                         // Lock PMM registers

    // Configure GPIO for pushbutton interrupt
    P1OUT |= BIT1;                            // Pull-up resistor on P1.1
    P1REN |= BIT1;                            // Select Pullup or pulldown resistor enabled
    P1DIR &= ~BIT1;                           // Set all but P1.1 to output direction
    P1IES = BIT1;                             // P1.1 high-to-low transition
    P1IFG = 0;                                // Clear all P1 interrupt flags
    P1IE = BIT1;                              // P1.1 interrupt enabled

    // LED configuration
    P1DIR |= BIT0;
    //P9DIR |= BIT7;


    //set GPIO P4.6 (red LED1) as an output
    P4DIR |= 1 << 6;




 //set GPIO P1.0 (LED2) as an output
  //set GPIO P1.2 as an output
	P1DIR = 0x05;



}










FUNC(int, OS_APPL_CODE) main(void)
{

   


	ioInit();
	
	// Start LoRa Radio
	LoraRadio.begin( _EU863, CH0, SF12BW125, 15, false);

	StartOS(OSDEFAULTAPPMODE);

	
	return 0;


}


















/*-----------------------------------------------------------------------------
 * ISR button
 */

ISR(buttonS2)
{
	

    P1IFG &= ~BIT1;   // Clear P1.1 IFG
   // P1OUT |= BIT0;    // P1.0 LED = ON


    //P4OUT ^= 1 << 6; //toggle red led

  //LoraRadio.sendData( 80, "0123456789", 11 );          // Problems faced => it is not exiting this function 

     P4OUT ^= 1 << 6;
    
    //P1OUT &= ~BIT0;    // P1.0 LED = OFF
//    __bic_SR_register_on_exit(LPM0_bits);     // Exit LPM0


}

#define APP_ISR_buttonS2_STOP_SEC_CODE
#include "tpl_memmap.h"


























