#include <msp430.h> 
#include <stdbool.h>
#include <time.h>    // for time()
#include <stdlib.h>  // for srand()

#include "lcd.h"


/**
 * main.c
 * Creator: Eric Dutan
 */
//Question: in line 196, I get a Debug call stack

//NOTE: while we wait for the game to start by btn, the timer flag could go true. I tried to make the if(timerFlag==true) be first and
//be what drives the program to evade that little boost from the beggining. But I can't becuase I believe that the propagation time
//is longer than the period by a lot. So it will always be fast.


/* EASY MODE (1.96 sec) -> ID__4, CCRO=61250
 *
 */
volatile bool timerFlag = false;
unsigned int ADC_Value_X;
unsigned int ADC_Value_Y;
unsigned int rand_x;
unsigned int rand_y;
volatile bool valid=true;
volatile bool flip= false;
volatile bool start =false;


typedef enum {NEUTRAL, LEFT, RIGHT, UP, DOWN} JoystickPosition;

//Logic from MP1
    //Global variables
    #define  NUM_ROWS  4
    #define  NUM_COLUMNS  20

    //char screen[NUM_ROWS][NUM_COLUMNS]; //Creates a 4 row by 20 column array for the screen
    // Structures
    typedef struct{
        int x;  //position of the snake in the x plane COLUMNS (Vertical)
        int y;  // position of the snake in the y plane     ROWS (Horizontal)
    } Cell;

    typedef struct{
        Cell body[NUM_COLUMNS*NUM_ROWS];    // the data type Cell of the snakes body is 80 long
        int size;                           // keeps track of the size of the snake
    } Snake;

    Snake snake;            //creates a global structure of Snake called snake

    JoystickPosition joystick;
    JoystickPosition prev_pos;

void onBoot()
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

       //UART A1 setup (MILESTONE 2 SETUP)
      // Choosing UCA0CTLW0 instead of UCA1CTLW0 b/c UCA0CTLW0 has pin 1.7 available
        //Setting baud rate
            UCA0CTLW0 |=UCSWRST;    //Put into SW reset, (Software Reset Enabled) WHY ENABLE??? ANSWER: we turn it off, get it how we want, turn it on
            UCA0CTLW0 |= UCSSEL__SMCLK ;    //Choosing the clock with 1Mhz, why UCSSEL__SMCLK? in spec sheet? ANSWER: UCSSEL__x where x is the type of clock.
                                            //Clocks aren't in spec sheet but is in library
            UCA0BRW = 104;                  //Pre-scalar for 9600 Baud rate with 1MHz SMCLK
            UCA0MCTLW = 0x1100;             //Modulation for 9600 baud rate with 1MHz SMCL, WHY 0X1100 AND NOT 0X11, why the extra zeroes
                                            //ANSWER: The register holds 8 bits, there fore 0x11 is shifter by eight. the first one is the MSB
        //Sets it for UART instead of SPI
            P1SEL1 &= ~BIT7;                //Changes P1.7 function to A1 UART TX
            P1SEL0 |= BIT7;
            UCA0CTLW0 &=~UCSWRST;       //takes UART A1 out of SW Reset



       //TIMER SETUP (Period of 250ms)
            TB0CTL |= TBCLR;    //THIS CLEARS THE TIMER
            TB0CTL |= TBSSEL__SMCLK;    //Using the 1MHz clock
            TB0CTL |= CNTL_0;           //Using the 16 bit
            TB0CTL |= ID_3;             //D1 is 8
            //TB0CTL |= ID__1;            //D2 is 1   ID__1
            TB0CTL |= MC__UP;           //UP mode
            TB0CCR0 = 61250;            // CCR0 = 31250


        //Port Interrupt configuration for blue btn ( 3.1)
            P3DIR &=~ BIT1;         // Configure P3.1 as input
            P3IES |= BIT1;          //Configure IRQ sensitivity H-to_L
            P3IFG &= ~BIT1;         //Clear POrt3.1 IRQ flag
            P3IE |= BIT1;           // Enable port 3.1 IRQ

        //Port Interrupt configuration for yellow btn ( 3.5)
            P3DIR &=~ BIT5;         // Configure P3.5 as input
            P3IES |= BIT5;          //Configure IRQ sensitivity H-to_L
            P3IFG &= ~BIT5;         //Clear POrt3.5 IRQ flag
            P3IE |= BIT5;           // Enable port 3.5 IRQ




            TB0CCTL0 |= CCIE;   // Enable TB0 CCR0 Overflow IRQ         WHERE IN DOCUMENTATION : THIS ENABLES THE INTERRUPT FOR TIMBER B CCR0
            TB0CCTL0 &= ~CCIFG; // Clear CCR0 Flag

            setLCDSize();
            autoscroll_off();


        /*    //Testing for Clock : Test: Correct
            P6DIR |=BIT2;
            P6OUT &=~ BIT2;*/


            //CONFIGURING ADC

            //pins
                P1SEL1 |= BIT3; // Config P1.3 Pin for A2 (XOUT)
                P1SEL0 |= BIT3;

                P1SEL1 |= BIT2; // Config P1.2 Pin for A2 (YOUT)
                P1SEL0 |= BIT2;

                // Configure ADC
                ADCCTL0 &= ~ADCSHT;  // Clear ADCSHY from def. of ADCSHT=01
                ADCCTL0 |= ADCSHT_2; // Conversion Cycles = 16 (ADCSHT=10)
                ADCCTL0 |= ADCON;    // Turn ADC ON

                /*
                 * ADCCTL1 is configuring the time?
                 */
                ADCCTL1 |= ADCSSEL_2; // ADC Clock Source = SMCLK (1MhZ)
                ADCCTL1 |= ADCSHP;    // Sample signal source = sampling timer WHAT? SOL: Timer seelect, using the ADC clock Generator, to choose whetehr to feed external or internal clokc signal


                /*
                 * ADCCTL2 is configuring the Resolution?
                 */
                ADCCTL2 &= ~ADCRES;   // Clear ADCRES from def. of ADCRES = 01
                ADCCTL2 |= ADCRES_2;  // Resolution = 12-bit (ADCRES=10)





            PM5CTL0 &= ~LOCKLPM5; // Turn on GPIO
            __enable_interrupt();

}

//Prototypes of functions
void initializeSnake(void);
void drawSnake(void);
void moveSnake(char dir);
bool validMove(void);
JoystickPosition getJoystickPosition(void);
void genTreat(void);
void reverse_snake(void);
void end_screen(void);

/*
 * KEY: the Mapping of the data structure in 2D array has [0,0], but in the LCD screen its [1,1] => offset by 1
 */
int main(void)
{
    onBoot();
    srand(time(NULL));  // Need to call this once.





    unsigned char prev_char;
    unsigned char first =0;
    unsigned char rev_stop=0;

    clear_screen();
    start_screen();

while(1)
{

    while(start==true)
    {


        initializeSnake();      //starts off the snake at the place
        drawSnake();

        //Create and draw treat
        genTreat();
        draw_character('*', (rand_y+1), ((rand_x)+1));


        while(validMove()==true){

            if(first==0)        //This is just to ensure that at the beginning, the snake moves right
            {
                prev_char='d';
                first++;
            }

            if(timerFlag == true)
            {
                timerFlag=false;                                                         //set flag back to false

                if((flip==true) ||rev_stop==1)     //Problem: when in a size>3, it doesn't go to the right end, ALSO when it swaps and tried to move, it disconnects.
                {
                    if(rev_stop==0)
                        reverse_snake();
                    flip=false;
                    rev_stop++;
                    if(rev_stop==2)
                        rev_stop=0;
                   /* if(prev_char == 's')
                    {
                        moveSnake('w');
                        prev_char = 'w';
                    }
                    else if(prev_char =='w')
                    {
                        moveSnake('s');                                             //This moves the snake in the 2D array
                        prev_char = 's';
                    }
                    else if (prev_char == 'a')
                    {
                        moveSnake('d');
                        prev_char = 'd';
                    }
                    else if (prev_char =='d')
                    {
                        moveSnake('a');
                        prev_char = 'a';
                    }*/


                }
                else{

                                joystick=getJoystickPosition();                                         //getting the position
                                if( (joystick==NEUTRAL)&&(prev_pos!=NEUTRAL) || (prev_pos==joystick)   )// To ensure that it moves even if joystick isn't moved (in Neutral)
                                {
                                    moveSnake(prev_char);   //
                                }
                                else if(joystick == DOWN)
                                {
                                    moveSnake('s');                                             //This moves the snake in the 2D array
                                    prev_char = 's';                                            //This is to ensure that it will continue to move in a certain direction if neutral
                                }
                                else if (joystick == UP)
                                {
                                    moveSnake('w');
                                    prev_char = 'w';
                                }
                                else if(joystick == LEFT)
                                {
                                    moveSnake('a');
                                    prev_char = 'a';
                                }
                                else if(joystick== RIGHT)
                                {
                                    moveSnake('d');
                                    prev_char = 'd';
                                }

                }


                if( !( (snake.body[0].x==rand_x) && (snake.body[0].y==rand_y) ) )                           //IF SNAKE HAS NOT EATEN THE TREAT
                                {
                                    drawSnake();                                                            //THIS DRAWS THE SNAKE WHERE IT IS AFTER MOVING


                                    draw_character('*', (rand_y+1), ((rand_x)+1));                          //This draws the treat


                                }
                else        //EATING
                                {

                                    snake.size++;           //increase size by one

                                    static volatile int i;
                                    for(i =1; i<snake.size; i++)            //starts at the 2nd index as we have just moved the head
                                    {
                                        Cell prev = snake.body[i-1];        //saves the previous position of the snake
                                        Cell current = snake.body[i];       //saves the position of the index i
                                        if(i == snake.size-1)               //ensures that we only update the end of the snake before incrementation
                                                {
                                                snake.body[i]=prev;             //moves the body of the new one to  the previous location
                                                prev=current;                       //Now that prev is used, change prev to be the next one to be of refrence .
                                                }


                                    }

                                    genTreat();             //creates new treat coordinates


                                    drawSnake();            //draws snake with new length
                                    draw_character('*', (rand_y+1), ((rand_x)+1));

                                }














                prev_pos=joystick;
            }
        }

        start=false;
        first=0;
        clear_screen();         //This will be for the LCD SCREEN When GAME OVER
        end_screen();
        static unsigned volatile int i;
        for(i=0;i<65000;i++){}
        for(i=0;i<65000;i++){}
        for(i=0;i<65000;i++){}
        for(i=0;i<65000;i++){}
        clear_screen();
        start_screen();
    }

}






}


//Methods
JoystickPosition getJoystickPosition()
{
    //JoystickPosition location;

               ADCCTL0 &= ~ADCENC;               // Disable ADC to configure channel

               ADCMCTL0 = ADCINCH_3;         // ADC Input channel = A3 (P1.3) XOUT

               ADCCTL0 |= ADCENC | ADCSC;     // Enable and Start conversion

               while(( ADCIFG & ADCIFG0) == 0);  // wait for conv. complete

               ADC_Value_X = ADCMEM0;    // Read ADC result

               ADCCTL0 &= ~ADCENC;


               ADCCTL0 &= ~ADCENC;               // Disable ADC to configure channel

               ADCMCTL0 = ADCINCH_2; // ADC Input Channel = A2 (P1.2) YOUT

               ADCCTL0 |= ADCENC | ADCSC;  // Enable and Start conversion

               while(( ADCIFG & ADCIFG0) == 0);  // wait for conv. complete

               ADC_Value_Y = ADCMEM0;    // Read ADC result

/*
 * To check location we first check if its neutral bc we need both values. After that it checks each value ( LEFT, RIGHT, UP, DOWN)
 * individually. If none of these positions are valid, will return NEUTRAL
 */
               if( (ADC_Value_X ==3067)&& (ADC_Value_Y == 2825))
               {
                   return NEUTRAL;
               }
               else if (    (ADC_Value_X <= 4095) && (ADC_Value_X >= 3800) )    //SHould be 4095
               {
                   return LEFT;
               }
               else if((ADC_Value_X >= 3) && (ADC_Value_X <= 10))       //SHould be 3
               {
                   return RIGHT;
               }
               else if ((ADC_Value_Y <= 4095) && (ADC_Value_Y >= 3800)) //SHould be 4095
               {
                   return UP;
               }
               else if((ADC_Value_Y >= 3) && (ADC_Value_Y <= 10))
               {
                   return DOWN;
               }
               else
                   return  NEUTRAL;

}
        //Methods from MP1
        void initializeSnake()  //Changes the global initialization of it
        {
            snake.size = 3;             //The snake has a size of 3

            snake.body[0].x=2;          //Locates the head (Column 2,row 0)
            snake.body[0].y=0;



            snake.body[1].x=1;          //Locates the middle    (Column 1,row 0)
            snake.body[1].y=0;


            snake.body[2].x=0;          //Locates the end       (Column 0,row 0)
            snake.body[2].y=0;

       //     volatile unsigned int i;
         //   for(i)
        }

        void drawSnake()
        {
            //start off with a clean slate
            clear_screen();         //This will be for the LCD SCREEN


           static volatile int i;

            for( i =0; i<snake.size ; i++)
            {
                if(i==0)    //the head only
                {

                    draw_character('0', ((snake.body[i].y)+1), ((snake.body[i].x)+1));   //will print with an offset of 1 in Y



                    //row is y
                    //column is x
                }
                else
                {
                draw_character('=', (snake.body[i].y)+1, (snake.body[i].x)+1);
                }
            }

        }

        void calibrate(Cell prev)
        {
           static volatile int i;
            for( i =1; i<snake.size; i++)    //starts at the 2nd index as we have just moved the head
                {
                    Cell next = snake.body[i];      //saves the position of the index i
                    snake.body[i]=prev;             //moves the body to the one that was previously saved
                    prev=next;                      //Now that prev is used, change prev to be the next one to be of refrence .
                }
        }


        void moveSnake(char dir){

            Cell prev = snake.body[0];      //saves the first index of the position

            if(dir=='s')
            {
                snake.body[0].y++;              // moves the head downward
            }
            if(dir=='w')    //maket the whole snake move one up
            {
                snake.body[0].y--;
            }
            if(dir=='a')    //make the whole snake move one left
            {
                snake.body[0].x--;      //decrement by one
            }
            if(dir=='d')    //maket the whole snake move one right
            {
                snake.body[0].x++;      //increment by one
            }
            calibrate(prev);
        }

        bool validMove()    //true when its valid, false when invalid
        {
            static volatile int i;
            for(i=1; i<snake.size; i++)
            {
                if ( (snake.body[0].x == snake.body[i].x ) && (snake.body[0].y == snake.body[i].y ) )
                {
                    return false;
                }

            }

            for(i=0; i<snake.size; i++)
            {
                if( !(  ((snake.body[i].x)<  NUM_COLUMNS) && ((snake.body[i].y)<  NUM_ROWS)  && ((snake.body[i].x)>=0) &&((snake.body[i].y)>=0) ) )
                    return false;
            }

            return true;
        }

/*        bool touchSnake(int X, int Y )
        {
           static volatile int i;
            for ( i =0; i<snake.size; i++)       //scan through the whole snake
            {
                if( (X == snake.body[i].x)  && (Y == snake.body[i].y))      //checking to see if they are in the same x coord
                {
                    return true;
                }
            }
            return false;

        }*/


        void genTreat()     //This should work as long as when I print it, I offset x and y by 1, therefore 0|->1 and 19|->20 for x and 0|->1, 3|->4 for y
        {

           /* //Initially creates place for the treat
            rand_x = rand() % NUM_COLUMNS; //the range of random numbers is 0 - 19
            rand_y = rand() % NUM_ROWS; //the range of random numbers is 0 - 3



            //Ensures that the treat isn't placed in the snake, if so, create a new position where it isn't
                while(touchSnake(rand_x,rand_y))
                {
                        rand_x = rand() % NUM_COLUMNS; //the range of random numbers is 0 - 19
                        rand_y = rand() % NUM_ROWS; //the range of random numbers is 0 - 3
                }


           */     bool collision;
                static volatile unsigned int i;
                do{
                    rand_x = rand() % NUM_COLUMNS; //the range of random numbers is 0 - 19
                    rand_y = rand() % NUM_ROWS; //the range of random numbers is 0 - 3

                    collision=false;

                    for(i=0;i<snake.size;i++)
                    {
                        if( (rand_x == snake.body[i].x)  && (rand_y == snake.body[i].y))      //checking to see if they are in the same x coord
                          {
                              collision=true;
                              break;
                          }
                    }


                }while(collision);


        }

        void reverse_snake()
        {
            volatile unsigned int start = 0;        //starts at the beggining of the original snake
            volatile unsigned int end = ((snake.size)-1);  //this keeps track of the end of the original snake
            while (start < end) {
                    Cell temp = snake.body[start];            //This will hold the coordinates and will be used as temp
                    snake.body[start] = snake.body[end];      //This flips the head with the tail
                    snake.body[end] = temp;                   //this saves the point at which it was pointing to the tail. ( at first itteration, will save the head)
                    start++;                                  //Now it will flip the cell next to the head and go down the snake
                    end--;                                    //Now what will be flipped is not the tail but second to last (next to the tail) and keep going
                }


        }

        void start_screen()
        {

                draw_character('S', 1,6);
                draw_character('T', 1,7);
                draw_character('A', 1,8);
                draw_character('R', 1,9);
                draw_character('T', 1,10);

                draw_character('G', 1,12);
                draw_character('A', 1,13);
                draw_character('M', 1,14);
                draw_character('E', 1,15);

                draw_character('(', 2,5);
                draw_character('Y', 2,6);
                draw_character('E', 2,7);
                draw_character('L', 2,8);
                draw_character('L', 2,9);
                draw_character('O', 2,10);
                draw_character('W', 2,11);

                draw_character('B', 2,13);
                draw_character('T', 2,14);
                draw_character('N', 2,15);
                draw_character(')', 2,16);

                draw_character('B', 4,1);
                draw_character('L', 4,2);
                draw_character('U', 4,3);
                draw_character('E', 4,4);

                draw_character('B', 4,6);
                draw_character('T', 4,7);
                draw_character('N', 4,8);
                draw_character('-', 4,9);
                draw_character('>', 4,10);

                draw_character('F', 4,11);
                draw_character('L', 4,12);
                draw_character('I', 4,13);
                draw_character('P', 4,14);

                draw_character('S', 4,16);
                draw_character('N', 4,17);
                draw_character('A', 4,18);
                draw_character('K', 4,19);
                draw_character('E', 4,20);
        }

        void end_screen()
        {
             draw_character('G',2,5);
             draw_character('A',2,6);
             draw_character('M',2,7);
             draw_character('E',2,8);

             draw_character('_',2,9);

             draw_character('O',2,10);
             draw_character('V',2,11);
             draw_character('E',2,12);
             draw_character('R',2,13);
             draw_character('!',2,14);
             draw_character('!',2,15);
             draw_character('!',2,16);

             draw_character('S',3,6);
             draw_character('I',3,7);
             draw_character('Z',3,8);
             draw_character('E',3,9);
             draw_character('-',3,10);
             draw_character('>',3,11);
             if(snake.size <10)
                 {
                 draw_character(snake.size + '0',3,12);
                 }
                 else
                 {
                     draw_character((snake.size / 10) + '0', 3, 12);  //Tens
                     draw_character((snake.size % 10) + '0', 3, 13);  // ones
                 }

        }



//-- Interrupt Service Routines -----
#pragma vector = TIMER0_B0_VECTOR
__interrupt void ISR_TB0_CCR0(void)
{
    timerFlag = true;
    //Testing clock
   //P6OUT ^= BIT2;


    TB0CCTL0 &= ~CCIFG; //Clear CCR0 Flag
}

//Port Interrupt
#pragma vector = PORT3_VECTOR
__interrupt void ISR_PORT3(void)
{
    if((P3IFG & BIT1) >0)
    {
        flip=true;
        P3IFG &= ~BIT1;
    }
    else
    {
        start=true;
        P3IFG &= ~BIT5;
    }

}

