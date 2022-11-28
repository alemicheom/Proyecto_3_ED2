//**************************************************************************************************************
/**
 * main.c
 */
//**************************************************************************************************************
// Librerias
//**************************************************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/adc.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "pinout.h"
#include "driverlib/pin_map.h"
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"


#define XTAL 16000000

void
InitConsole(void)
{
    // Enable GPIO port A which is used for UART0 pins.
    // TODO: change this to whichever GPIO port you are using.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Configure the pin muxing for UART0 functions on port A0 and A1.
    // This step is not necessary if your part does not support pin muxing.
    // TODO: change this to select the port/pin you are using.
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);

    // Enable UART0 so that we can configure the clock.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    // Use the internal 16MHz oscillator as the UART clock source.
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    // Select the alternate (UART) function for these pins.
    // TODO: change this to select the port/pin you are using.
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Initialize the UART for console I/O.
    UARTStdioConfig(0, 115200, 16000000);
}


//**************************************************************************************************************
// Variables Globales
//**************************************************************************************************************
int estado;
int pb_1 = 0;               // pushbutton c6
int oldpb_1 = 0;            // pushbutton c6
int pb_2 = 0;               // pushbutton c7
int oldpb_2 = 0;            // pushbutton c7
int contador1 = 0;

int f_blue = 0;             // banderas para leds con uart
int f_green = 0;
int f_red = 0;
int f_yellow = 0;
int f_display = 0;

char leds;

int bandera = 0;            // para display 7 segmentos
uint32_t ui32Period;
uint32_t i = 0;

int f_hola = 0;             // bandera para animación
int f_leds = 0;             // bandera para animación leds

unsigned char sevenSEG[] = {0b0111111, 0b0000110, 0b1011011, 0b1001111, 0b01100110, 0b1101101, 0b1111101, 0b0000111, 0b1111111, 0b1101111, 0b1110111, 0b1111100, 0b0111001, 0b01011110, 0b1111001, 0b1110001};
unsigned char hola[] = {0b01110110, 0b00111111, 0b00111000, 0b01110111, 0b00000000};

uint32_t pui32ADC0Value[1];
uint32_t pot;


//**************************************************************************************************************
// Prototipos de Función
//**************************************************************************************************************

void Timer0IntHandler(void);

unsigned short map(uint32_t val, uint32_t in_min, uint32_t in_max,
            unsigned short out_min, unsigned short out_max);



//**************************************************************************************************************
// Código Principal
//**************************************************************************************************************
int main(void)
{
PinoutSet();
InitConsole();
    UARTprintf(" \n");
    UARTprintf("  Proyecto\n");
    UARTprintf("  Alejandro \n");
    UARTprintf(" \n");

//TIMER

    // 16MHZ
    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);

    // Se habilita el reloj para el temporizador
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    // Configuración del Timer 0 como temporizador per�odico
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

    // Se calcula el período para el temporizador (1 seg)
    ui32Period = (SysCtlClockGet()) / 2;
    // Establecer el periodo del temporizador
    TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period - 1);

    // Se habilita la interrupción por el TIMER0A
    IntEnable(INT_TIMER0A);
    // Se establece que exista la interrupción por Timeout
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    // Se habilitan las interrupciones Globales
    IntMasterEnable();
    // Se habilita el Timer
    TimerEnable(TIMER0_BASE, TIMER_A);

    //pines para 7 seg
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3| GPIO_PIN_4| GPIO_PIN_5| GPIO_PIN_6| GPIO_PIN_7);

    //ADC

    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
                ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE |
                                             ADC_CTL_END);
                ADCSequenceEnable(ADC0_BASE, 3);
                ADCIntClear(ADC0_BASE, 3);

    //**********************************************************************************************************
    // Loop Principal
    //**********************************************************************************************************
    while (1)
    {

    estado = GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2);         // variable para cambiar de estado

    pb_1 = GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_4);                        // variable para leer pb 1
    pb_2 = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4);                        // variable para leer pb 2


    switch(estado){
        case 0:                                                             // switch 1 = 0 switch 2 = 0

            leds = UARTCharGetNonBlocking(UART0_BASE);    //UARTgetc();//

                   if (leds == 'b'){           // on blue
                       if (f_blue == 0){
                           GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_3, GPIO_PIN_3);
                           f_blue++;
                       }
                       else if (f_blue == 1){ // off blue
                           GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_3, 0);
                           f_blue = 0;
                       }
                   }

                   if (leds == 'g'){           // on green
                       if (f_green == 0){
                           GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_2, GPIO_PIN_2);
                           f_green++;
                       }
                       else if (f_green == 1){ // off green
                           GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_2, 0);
                           f_green = 0;
                       }
                   }
                   if (leds == 'r'){           // on red
                      if (f_red == 0){
                           GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1, GPIO_PIN_1);
                           f_red++;
                      }
                      else if (f_red == 1){    // off red
                           GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1, 0);
                           f_red = 0;
                      }
                   }

                   if (leds == 'y'){           // on yellow
                       if (f_yellow == 0){
                           GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_0, GPIO_PIN_0);
                           f_yellow++;
                       }
                       else if (f_yellow == 1){ // off yellow
                            GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_0, 0);
                            f_yellow = 0;
                       }
                   }

                   if(pb_1 && oldpb_1== 0){             // print con UART
                       UARTprintf("Blue: %d\n", f_blue);
                       UARTprintf("Green: %d\n", f_green);
                       UARTprintf("Red: %d\n", f_red);
                       UARTprintf("Yellow: %d\n", f_yellow);
                       UARTprintf("Display: %d\n", f_display);
                   }
                   oldpb_1 = pb_1;

                   if(pb_2 && oldpb_2== 0){             // print con UART
                       UARTprintf("Blue: %d\n", f_blue);
                       UARTprintf("Green: %d\n", f_green);
                       UARTprintf("Red: %d\n", f_red);
                       UARTprintf("Yellow: %d\n", f_yellow);
                       UARTprintf("Display: %d\n", f_display);
                  }
                  oldpb_2 = pb_2;

                  if (leds == '0'){
                      GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3| GPIO_PIN_4| GPIO_PIN_5| GPIO_PIN_6 | GPIO_PIN_7, 0b0111111);
                      f_display = 0;
                       }
                  if (leds == '1'){
                                        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3| GPIO_PIN_4| GPIO_PIN_5| GPIO_PIN_6 | GPIO_PIN_7,0b0000110);
                                        f_display = 1;
                                         }
                  if (leds == '2'){
                                        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3| GPIO_PIN_4| GPIO_PIN_5| GPIO_PIN_6 | GPIO_PIN_7, 0b1011011);
                                        f_display = 2;
                                         }
                  if (leds == '3'){
                                        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3| GPIO_PIN_4| GPIO_PIN_5| GPIO_PIN_6 | GPIO_PIN_7, 0b1001111);
                                        f_display = 3;
                                         }
                  if (leds == '4'){
                                        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3| GPIO_PIN_4| GPIO_PIN_5| GPIO_PIN_6 | GPIO_PIN_7, 0b01100110);
                                        f_display = 4;
                                         }
                  if (leds == '5'){
                                       GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3| GPIO_PIN_4| GPIO_PIN_5| GPIO_PIN_6 | GPIO_PIN_7, 0b1101101);
                                       f_display = 5;
                                        }
                  if (leds == '6'){
                                        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3| GPIO_PIN_4| GPIO_PIN_5| GPIO_PIN_6 | GPIO_PIN_7, 0b1111101);
                                        f_display = 6;
                                         }
                  if (leds == '7'){
                                       GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3| GPIO_PIN_4| GPIO_PIN_5| GPIO_PIN_6 | GPIO_PIN_7, 0b0000111);
                                       f_display = 7;
                                        }
                  if (leds == '8'){
                                        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3| GPIO_PIN_4| GPIO_PIN_5| GPIO_PIN_6 | GPIO_PIN_7, 0b1111111);
                                        f_display = 8;
                                         }
          break;


        case 0x2:                                                          // switch 1 = 1 switch 2 = 0
            if(pb_1 && oldpb_1== 0){             //incrementrar leds
                contador1++;
                   if(contador1 > 15) {
                       contador1 = 0;
                   }
                   GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, contador1);
               }
             oldpb_1 = pb_1;

             if(pb_2 && oldpb_2==0){          //decrementar leds
                 contador1--;
                    if(contador1 < 0) {
                        contador1 = 15;
                    }
                    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, contador1);
              }
             oldpb_2 = pb_2;

            break;

        case 0x4:                                                          // switch 1 = 0 switch 2 = 1
            //interrupción de timer
            break;

        case 0x6:                                                          // switch 1 = 1 switch 2 = 1
            ADCProcessorTrigger(ADC0_BASE, 3);
            while(!ADCIntStatus(ADC0_BASE, 3, false)){
            }

            ADCIntClear(ADC0_BASE, 3);
            ADCSequenceDataGet(ADC0_BASE, 3, pui32ADC0Value);
            pot = map(pui32ADC0Value[0], 0, 4095, 0, 99);
            UARTprintf("POT = %4d\r", pot);

            break;
        default:
            GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_3, 0);
            break;

    }

    }
}

void Timer0IntHandler(void)
{

    // Clear the timer interrupt
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    if(estado == 0x2){                  //contador en 7 seg

        if(bandera > 15)               // hasta 15
            bandera = 0;

        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3| GPIO_PIN_4| GPIO_PIN_5| GPIO_PIN_6 | GPIO_PIN_7, sevenSEG[bandera]);
        bandera++;

        UARTprintf("Leds: %d\n", contador1);
        UARTprintf("7_Seg: %d\n", bandera);
    }

     if (estado == 0x4){

         if(f_hola > 6){
                    f_hola = 0;
                }

                GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3| GPIO_PIN_4| GPIO_PIN_5| GPIO_PIN_6 | GPIO_PIN_7 , hola[f_hola]);
                f_hola++;

         if(f_leds > 8){
                    f_leds = 0;
          }
                SysCtlDelay((uint32_t)(SysCtlClockGet() / 12));
                f_leds++;

         if(f_leds == 4){
                GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_2, GPIO_PIN_2);
                GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1, 0);
                GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_3, 0);

         }
         else if(f_leds == 8){
             GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1, GPIO_PIN_1);
             GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_3, GPIO_PIN_3);
             GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_2, 0);
         }


     }


}

unsigned short map(uint32_t x, uint32_t x0, uint32_t x1,
                   unsigned short y0, unsigned short y1){
    return (unsigned short)(y0+((float)(y1-y0)/(x1-x0))*(x-x0));
}

