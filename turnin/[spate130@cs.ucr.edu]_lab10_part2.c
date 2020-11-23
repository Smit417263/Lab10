/*	Author: lab
 *  Partner(s) Name: 
 *	Lab Section: 023
 *	Assignment: Lab #10  Exercise #2
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "bit.h"
#include "timer.h"
#include "keypad.h"
#include "scheduler.h"


#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif


task tasks[3];
const unsigned short tasksNum=3;
const unsigned long tasksPeriod=100;

unsigned char tempB = 1;

enum Key{start,wait,on,checker,done};

unsigned char j;
unsigned char reset;
const unsigned char check[5]={'1','2','3','4','5'};

int tick_1(int state){

unsigned char x;
unsigned char tmp;
x=GetKeypadKey();

switch(state){
    case start:
        j=0;
        state = wait;
        break;
    case wait:
        if(x != '#'){
            state = wait;
        }
        else if(x == '#'){
            j = 0;
            reset = 0;
            state = on;
        }
        break;

    case on:
        if(x == '#'){
            state = on;
        }
        else if(x == '\0'){
            state = checker;
        }
        break;

    case checker:
        if(x == '#'){
            state = on;
            j = 0;
            reset = 0;
        }
        else if((x == '\0') && (j < 5)){
            state = checker;
        }
        else if((x != '\0') && (j < 5)){
            tmp = x;
            state = done;
            if(tmp == check[j]){
                reset = reset + 1;
            }
            j=j+1;
        }
        else if((x == '\0') && (j >= 5)){
            state=wait;
        }
        break;

    case done:
        if(x!='\0'){
            state = done;
        }
        else if(x=='\0'){
            state = checker;
        }
        break;
    default:
        break;
}
switch(state){
    case start:
        break;

    case wait:
        if (reset == 5){
            tempB=0;
            reset=0;
        }
        break;

    case on:
        reset=0;
        j=0;
        break;

    case checker:
        break;

    case done:
        break;

    default:
        break;

}

return state;
}



enum DoorLock{init,press,lock};

int tick_2(int state){
	switch(state){
		case init:
			state = press;
			break;

		case press:
			if((~PINB & 0x80) == 0x80){
				tempB = 1;
				state = lock;
			}
			else{
				state = press;
			}
			break;

		case lock:
			if((~PINB & 0x80) == 0x80){
				state = lock;
			}
			else{
				state = press;
			}
			break;

		default:
			break;
	}
	switch(state){
        case init:
            break;

		case press:
			break;

        case lock:
            break;

		default:
			break;
	}
	return state;
}

enum LD_SM{LD_start,output};

int tick_3(int state){
	switch(state){
		case LD_start:
			state = output;
			break;

		case output:
			state = output;
			break;

		default:
			break;
	}
	switch(state){
		case LD_start:
			break;

		case output:
			PORTB = tempB;
			break;

		default:
			break;
	}
	return state;
}

int main(void) {

	DDRB=0x7F;PORTB=0x80;
	DDRC=0xF0;PORTC=0x0F;

	unsigned char i = 0;
	tasks[i].state = start;
	tasks[i].period=100;
	tasks[i].elapsedTime=0;
	tasks[i].TickFct=&tick_1;
    	i++;
    	tasks[i].state = init;
    	tasks[i].period = 100;
    	tasks[i].elapsedTime = 0;
    	tasks[i].TickFct = &tick_2;
	i++;
	tasks[i].state = LD_start;
 	tasks[i].period = 100;
    	tasks[i].elapsedTime = 0;
    	tasks[i].TickFct = &tick_3;

	TimerSet(100);
	TimerOn();

	while(1){}

	return 1;
}
