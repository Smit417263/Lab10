/*	Author: lab
 *  Partner(s) Name: 
 *	Lab Section: 023
 *	Assignment: Lab #10  Exercise #2
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Demo Link:
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

void set_PWM(double frequency) {
	static double current_frequency;
	if (frequency != current_frequency) {

		if (!frequency) TCCR3B &= 0x08;
		else TCCR3B |= 0x03;
		if (frequency < 0.954) OCR3A = 0xFFFF;
		else if (frequency > 31250) OCR3A = 0x0000;

		else OCR3A = (short)(8000000 / (128 * frequency)) - 1;

		TCNT3 = 0;
		current_frequency = frequency;
	}
}
void PWM_on() {
	TCCR3A = (1 << COM3A0);
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	set_PWM(0);
}
void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}


task tasks[4];

unsigned char tempB = 1;
const unsigned short num=4;
const unsigned long t_periodd=100;

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



const unsigned short sound[16]={280.00,270.00,280.00,290.00,280.00,300.00,310.00,270.00,280.00,270.00,280.00,290.00,280.00,300.00,310.00,270.00};

unsigned char s;

enum ring {R_start,wait1,on1,press1};

int tick_3(int state){
switch(state) {
    case R_start:
        state = wait1;
        break;

    case wait1:
        if((~PINA & 0x80) == 0x00){
            state = wait1;
        }
        else if((~PINA & 0x80) == 0x80){
            state = on1;
            s = 0;
        }
        break;
    case on1:
        if(s < 16){
            state = on1;
        }
        else if(s >= 16){
            state = press1;
        }
        break;
    case press1:
        if((~PINA & 0x80) != 0x00){
            state = press1;
        }
        else if((~PINA & 0x80) == 0x00){
            state = wait1;
        }
        break;

    default:
        break;
}
switch(state){
    case R_start:
        set_PWM(0);
        break;

    case wait1:
        s = 0;
        set_PWM(0);
        break;

    case on1:
        set_PWM(sound[s]);
        s++;
        break;

    case press1:
        set_PWM(0);
        break;

    default:
        break;

}

return state;
}


enum LD_SM{LD_start,output};

int tick_4(int state){
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

	DDRA=0x00;PORTA=0xFF;
	DDRB=0x7F;PORTB=0x80;
	DDRC=0xF0;PORTC=0x0F;

    	PWM_on();

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
    	tasks[i].state = R_start;
	tasks[i].period = 200;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &tick_3;
	i++;
	tasks[i].state = LD_start;
    	tasks[i].period = 100;
    	tasks[i].elapsedTime = 0;
    	tasks[i].TickFct = &tick_4;

	TimerSet(100);
	TimerOn();

	while(1){}

	return 1;
}

