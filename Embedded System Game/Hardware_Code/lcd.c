/*
 * lcd.c
 *
 *  Created on: Mar 20, 2025
 *      Author: ericdutan
 */

#include <msp430.h>
#include <stdbool.h>
#include "lcd.h"   //Including the header file

int DELAY_CYCLES = 1000;

//volatile bool timerFlag = false;

void clear_screen()
{
    unsigned volatile int i;

    UCA0TXBUF = 0xFE;    //START COMMAND hold the data waiting to be moved into the transmit shift register

    for(i=0; i<DELAY_CYCLES; i++){}     //busy waiting
                          //reseting to false

    UCA0TXBUF = 0x58;   //CLEAR SCREEN this is X

    for(i=0; i<DELAY_CYCLES; i++){}     //busy waiting
                       //reseting to false
}

/*
 * COLUMN GETS SENT FIRST, THEN ROW
 */
void set_cursor_position(unsigned char row, unsigned char column)
{


        //Attempt to correct. Question: 0x47 or 0x58
     unsigned volatile int i;


    UCA0TXBUF = 0xFE;    //START COMMAND hold the data waiting to be moved into the transmit shift register

    for(i=0; i<DELAY_CYCLES; i++){}     //busy waiting

                        //reseting to false

    UCA0TXBUF = 0x47;   //  set cursor position

    for(i=0; i<DELAY_CYCLES; i++){}     //busy waiting



    UCA0TXBUF = column;    // This is to transmit whatever character is stored in column

    for(i=0; i<DELAY_CYCLES; i++){}     //busy waiting



    UCA0TXBUF = row;    // This is to transmit whatever character is stored in row

    for(i=0; i<DELAY_CYCLES; i++){}     //busy waiting



}

void draw_character(char ch, unsigned char row, unsigned char column){
     unsigned volatile int i;


    set_cursor_position(row, column);

    UCA0TXBUF = ch;

    for(i=0; i<DELAY_CYCLES; i++){}     //busy waiting

    //while(timerFlag == false){};            //using the timer. This will get off when its true
    //timerFlag= false;                       //reseting to false

}
void setLCDSize(){
     unsigned volatile int i;

    UCA0TXBUF = 0xFE;    //START COMMAND hold the data waiting to be moved into the transmit shift register

    for(i=0; i<DELAY_CYCLES; i++){}     //busy waiting

    //while(timerFlag == false){};            //using the timer. This will get off when its true
   // timerFlag= false;                       //reseting to false

    UCA0TXBUF = 0xD1;    //SET LCD Size Command

    for(i=0; i<DELAY_CYCLES; i++){}     //busy waiting

   // while(timerFlag == false){};            //using the timer. This will get off when its true
   // timerFlag= false;                       //reseting to false

    UCA0TXBUF = 0x14;                   //Number of columns

    for(i=0; i<DELAY_CYCLES; i++){}     //busy waiting

    //while(timerFlag == false){};            //using the timer. This will get off when its true
    //timerFlag= false;                       //reseting to false

    UCA0TXBUF = 0x04;                   //Number of rows

    for(i=0; i<DELAY_CYCLES; i++){}     //busy waiting

    //while(timerFlag == false){};            //using the timer. This will get off when its true
   // timerFlag= false;                       //reseting to false


}

void autoscroll_off(){
     unsigned volatile int i;

    UCA0TXBUF = 0xFE;    //START COMMAND hold the data waiting to be moved into the transmit shift register

    for(i=0; i<DELAY_CYCLES; i++){}     //busy waiting

    //while(timerFlag == false){};            //using the timer. This will get off when its true
    //timerFlag= false;                       //reseting to false

    UCA0TXBUF = 0x52;                   //turn off auto scrolling

    for(i=0; i<DELAY_CYCLES; i++){}     //busy waiting

    //while(timerFlag == false){};            //using the timer. This will get off when its true
    //timerFlag= false;                       //reseting to false

}

void erase_cell(unsigned char row, unsigned char column){
    draw_character(' ', row, column);
}

