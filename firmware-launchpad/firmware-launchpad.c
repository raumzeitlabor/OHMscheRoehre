//*****************************************************************************
//
// blinky.c - Simple example to blink the on-board LED.
//
// Copyright (c) 2012 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 9453 of the EK-LM4F120XL Firmware Package.
//
//*****************************************************************************


#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/adc.h"

#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/rom_map.h"
#include "utils/ustdlib.h"

#include "inc/lm4f120h5qr.h"

//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>Blinky (blinky)</h1>
//!
//! A very simple example that blinks the on-board LED using direct register
//! access.
//
//*****************************************************************************

static unsigned char selector1;
static unsigned char selector2;
static unsigned char button1;
static unsigned char button2;
static unsigned long value;
static unsigned long brightness;

void
UARTSend(const unsigned char *pucBuffer, unsigned long ulCount)
{
    //
    // Loop while there are more characters to send.
    //
    while(ulCount--)
    {
        //
        // Write the next character to the UART.
        //
        UARTCharPutNonBlocking(UART5_BASE, *pucBuffer++);
    }
}

main(void)
{
	unsigned long ulADC1_Value[1];
    volatile unsigned long ulLoop;
    char buf[32];

    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                          SYSCTL_XTAL_16MHZ);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART5);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

    GPIOPinConfigure(GPIO_PE5_U5TX);
    GPIOPinTypeUART(GPIO_PORTE_BASE, GPIO_PIN_5);

    UARTConfigSetExpClk(UART5_BASE, ROM_SysCtlClockGet(), 38400,  (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_EVEN));
SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_2);
ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH1 | ADC_CTL_IE |
                             ADC_CTL_END);
ADCSequenceEnable(ADC0_BASE, 3);
ADCIntClear(ADC0_BASE, 3);



    ROM_GPIODirModeSet(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_DIR_MODE_IN);
    ROM_GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_DIR_MODE_IN);
    ROM_GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_DIR_MODE_IN);
    ROM_GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_DIR_MODE_IN);
    ROM_GPIODirModeSet(GPIO_PORTD_BASE, GPIO_PIN_0, GPIO_DIR_MODE_IN);
    ROM_GPIODirModeSet(GPIO_PORTD_BASE, GPIO_PIN_1, GPIO_DIR_MODE_IN);
    ROM_GPIODirModeSet(GPIO_PORTD_BASE, GPIO_PIN_2, GPIO_DIR_MODE_IN);
    ROM_GPIODirModeSet(GPIO_PORTD_BASE, GPIO_PIN_3, GPIO_DIR_MODE_IN);
    ROM_GPIODirModeSet(GPIO_PORTE_BASE, GPIO_PIN_1, GPIO_DIR_MODE_IN);

	ROM_GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_2,
						 GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	ROM_GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4,
							 GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	ROM_GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_3,
							 GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	ROM_GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_2,
							 GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	ROM_GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_0,
							 GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	ROM_GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_1,
							 GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	ROM_GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_2,
							 GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	ROM_GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_3,
							 GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	ROM_GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_1,
							 GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
ADCProcessorTrigger(ADC0_BASE, 3);

    //
    // Loop forever.
    //
    while(1)
    {


        //
        // Turn on the LED.
        //
        GPIO_PORTF_DATA_R |= 0x08;

        //
        // Delay for a bit.
        //
        for(ulLoop = 0; ulLoop < 200000; ulLoop++)
        {
        }

        //
        // Turn off the LED.
        //
        GPIO_PORTF_DATA_R &= ~(0x08);

        //
        // Delay for a bit.
        //
        for(ulLoop = 0; ulLoop < 20000; ulLoop++)
        {
        }

        selector2 = 0;
        selector1 = 0;
        button1 = 0;
        button2 = 0;

        if (!GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4)) {
        	button1 = 1; // BURST
        }

        if (!GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_3)) {
        	selector1 = 2; // VIDEO
		}

        if (!GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_2)) {
        	selector1 = 1; // 4.333 MHz
        }

        if (!GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_0)) {
        	selector2 = 1; // GRID
        }

        if (!GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_1)) {
        	//ROM_UARTCharPut(UART5_BASE, 'X');
        	selector2 = 4; // FARBE
        }

        if (!GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_2)) {
        	selector2 = 2; // DOT GRID
        }

        if (!GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_3)) {
           	selector2 = 3; // WEISS
        }

        if (!GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_1)) {
			button2 = 1; // 5.5 MHz
		}

        if (!GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_2)) {
        	selector1 = 3; // HF
        }


	if (ADCIntStatus(ADC0_BASE, 3, false)) {
		ADCIntClear(ADC0_BASE, 3);
		ADCSequenceDataGet(ADC0_BASE, 3, ulADC1_Value);
		ADCProcessorTrigger(ADC0_BASE, 3);

		brightness = (ulADC1_Value[0] / 32);
	}

	value = (selector1 | (selector2 << 2) | (button1 << 5) | (button2 << 6) | (brightness << 12))+1;

	usprintf(buf, "%dX\n\r", value);
	UARTSend(buf, 32);
    }
}
