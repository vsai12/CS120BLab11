/*	Author: lab
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include <stdio.h>
#include <stdlib.h>
#endif

volatile unsigned char TimerFlag = 0;

unsigned long timer = 1;
unsigned long timer_count = 0;
void TimerOn() {
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;
	timer_count = timer;
	SREG |= 0x80;
}

void TimerOff() {
	TCCR1B = 0x00; 
}

void TimerISR() {
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect) {
	timer_count--;
	if(timer_count == 0) {
		TimerISR();
		timer_count = timer;
	}
}

void TimerSet(unsigned long M) {
	timer = M;
	timer_count = timer;
}

typedef struct task {
	signed char state;
	unsigned long int period;
	unsigned long int elapsedTime;
	int (*TickFct) (int);
} task;

/*
enum Demo_States {shift};
int Demo_Tick(int state) {
	static unsigned char pattern = 0x80;
	static unsigned char row = 0xFE;

	switch(state) {
		case shift:
			break;
		default: state = shift;
			 break;
	}
	switch(state) {
		case shift:
			if(row == 0xEF && pattern == 0x01) { //Reset demo
				pattern = 0x80;
				row = 0xFE;
			} else if (pattern == 0x01) {
				pattern = 0x80;
				row = (row << 1) | 0x01;
			} else {
				pattern >>= 1;
			}
			break;
		default:
			break;
	}
	PORTC = pattern;
	PORTD = row;
	return state;
}*/

// x: 8-bit value.   k: bit position to set, range is 0-7.   b: set bit to this, either 1 or 0
unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
   return (b ?  (x | (0x01 << k))  :  (x & ~(0x01 << k)) );
              //   Set bit to 1           Set bit to 0
}

unsigned char GetBit(unsigned char x, unsigned char k) {
   return ((x & (0x01 << k)) != 0);
}

unsigned char outputR[5] = {0, 0, 0, 0, 0};

enum Pong_States { P_Reset, P_Play1 };
int Pong_Tick(int state) {
	static unsigned char paddle1B = 1;
	static unsigned char paddle1E = 3;
	unsigned char tmpA = ~PINA & 0xC3;
	switch(state) {
		case P_Reset:
			if(tmpA == 0x02)
				state = P_Play1;
			break;
		case P_Play1:
			if(tmpA == 0x01) {
				state = P_Reset;
			}
			break;
		default:
			state = P_Reset;
			break;
	}

	switch(state) {
		case P_Reset:
			paddle1B = 1;
			paddle1E = 3;
			for(unsigned i = paddle1B; i <= paddle1E; i++) {
				outputR[i] = SetBit(outputR[i], 0, 1);
			}
			outputR[0] = SetBit(outputR[0], 0, 0);
			outputR[4] = SetBit(outputR[4], 0, 0);
			break;
		case P_Play1:
			if(tmpA == 0x40 && paddle1B != 0) {
                                paddle1B--;
				outputR[paddle1B] = SetBit(outputR[paddle1B], 0, 1);
				outputR[paddle1E] = SetBit(outputR[paddle1E], 0, 0);
				paddle1E--;
			}
                        else if(tmpA == 0x80 && paddle1E != 4) {
				paddle1E++;
                                outputR[paddle1E] = SetBit(outputR[paddle1E], 0, 1);
                                outputR[paddle1B] = SetBit(outputR[paddle1B], 0, 0);
                                paddle1B++;
			}
			break;
		default:
			break;
	}
	return state;
}

enum AI_States {A_Reset, A_Play};

unsigned char ApaddleB = 1;
unsigned char ApaddleE = 3;
int AI_Tick(int state) {
        unsigned char tmpA = ~PINA & 0x03;
        switch(state) {
                case A_Reset:
                        if(tmpA == 0x02)
                                state = A_Play;
                        break;
                case A_Play:
                        if(tmpA == 0x01) {
                                state = A_Reset;
                        }
                        break;
                default:
                        state = A_Reset;
                        break;
        }

        switch(state) {
                case A_Reset:
                        ApaddleB = 1;
                        ApaddleE = 3;
                        for(unsigned i = ApaddleB; i <= ApaddleE; i++) {
                                outputR[i] = SetBit(outputR[i], 7, 1);
                        }
                        outputR[0] = SetBit(outputR[0], 7, 0);
                        outputR[4] = SetBit(outputR[4], 7, 0);
                        break;
                case A_Play:
                        break;
                default:
                        break;
        }
        return state;
}


enum Rand_States {R_Reset, R_Play};

unsigned int seed = 0;

int rand_Tick(int state) {
	unsigned char tmpA = ~PINA & 0x03;
	switch(state) {
                case A_Reset:
                        if(tmpA == 0x02)
                                state = A_Play;
                        break;
                case A_Play:
                        if(tmpA == 0x01) {
                                state = A_Reset;
                        }
                        break;
                default:
                        state = A_Reset;
                        break;
        }

        switch(state) {
                case R_Reset:
			seed++;
                        break;
                case R_Play:
			if(seed != 0)
				srand(seed);
			seed = 0;
                        break;
                default:
                        break;
        }
        return state;
}

enum ball_States { B_Reset, B_Start };

int ball_Tick(int state) {
	static unsigned char direction = 0; // 0 means straight; 1 means diagonal left; 2 means diagonal right;
	static unsigned char right = 1;
	static unsigned char bC = 5;
	static unsigned char bR = 2;
	unsigned char tmpA = ~PINA & 0x03;
	static unsigned char reset = 0;
	switch(state) {
		case B_Reset:
			reset = 0;
			if(tmpA == 0x02)
				state = B_Start;
			break;
		case B_Start:
			if(tmpA == 0x01 || reset)
				state = B_Reset;
			break;
		default:
			state = B_Reset;
	}

	switch(state) {
		case B_Reset:
			outputR[bR] = SetBit(outputR[bR], bC, 0);
			bC = 5;
			bR = 2;
			direction = 0;
			right = 1;
			break;
		case B_Start:
			outputR[bR] = SetBit(outputR[bR], bC, 0);
			int r = rand() % 7 + 1;
			if(direction == 1) {
                                if(bR != 4) {
                                        bR++;
					if(r != 7 && ApaddleE != 4) {
						ApaddleE++;
                                		outputR[ApaddleE] = SetBit(outputR[ApaddleE], 7, 1);
                                		outputR[ApaddleB] = SetBit(outputR[ApaddleB], 7, 0);
                                		ApaddleB++;
					}	
				}	
                                else {
                                       	direction = 2;
                                	bR--;
					if(r != 7 && ApaddleB != 0) {
						ApaddleB--;
                               			outputR[ApaddleB] = SetBit(outputR[ApaddleB], 7, 1);
                                		outputR[ApaddleE] = SetBit(outputR[ApaddleE], 7, 0);
                                		ApaddleE--;
					}
                        	}
                        }
			else if(direction == 2) {
                                if(bR != 0)
                                        bR--;
					if(r != 7 && ApaddleB != 0) {
                                                ApaddleB--;
                                                outputR[ApaddleB] = SetBit(outputR[ApaddleB], 7, 1);
                                                outputR[ApaddleE] = SetBit(outputR[ApaddleE], 7, 0);
                                                ApaddleE--;
                                        }
                                else {
                                        direction = 1;
                        		bR++;
					if(r != 7 && ApaddleE != 4) {
                                                ApaddleE++;
                                                outputR[ApaddleE] = SetBit(outputR[ApaddleE], 7, 1);
                                                outputR[ApaddleB] = SetBit(outputR[ApaddleB], 7, 0);
                                                ApaddleB++;
                                        }
                        	}
                        }

			if(bC !=  1 || !right) {
				if(right)
					bC--;
				else {
					if(bC != 6)
						bC++;
					else if(bC == 6) {
						if(GetBit(outputR[bR], 7) == 1) {
	                 	                	right = 1;
        	                	        	if(bR == 0)
                                                		direction = 2;
                                        		else if(bR == 4)
                                                		direction = 1;
                                        		else {
                                                		if(GetBit(outputR[bR + 1], 7) == 1 && GetBit(outputR[bR - 1], 7) == 1)
                                                        		direction = 0;
                                                		else if(GetBit(outputR[bR + 1], 7) == 1)
                                                        		direction = 1;
                                                		else
                                                        		direction = 2;
                                    			}
                                		}
                                		else {
                                        		bC = 7;
                                        		reset = 1;
        		                        }
					}
				}
			}
			else if(bC == 1) {
				if(GetBit(outputR[bR], 0) == 1) {
					right = 0;
					if(bR == 0)
						direction = 1;
					else if(bR == 4)
						direction = 2;
					else {
						if(GetBit(outputR[bR + 1], 0) == 1 && GetBit(outputR[bR - 1], 0) == 1)
							direction = 0;
						else if(GetBit(outputR[bR + 1], 0) == 1)
							direction = 1;
						else
							direction = 2;
					}
				}
				else {
					bC = 0;
					reset = 1;
				}
			}
			break;
		default:
			break;
	}
	outputR[bR] = SetBit(outputR[bR], bC, 1);
	return state;
}

enum display_States { display_display };

int displaySMTick(int state) {
	unsigned char output;
	unsigned char display;
	static unsigned char rows[5] = {0xFE, 0xFD, 0xFB, 0xF7, 0xEF};
	static unsigned char counter = 0;
	switch(state) {
		case display_display: state = display_display; break;
		default: state = display_display; break;
	}

	switch(state) {
		case display_display:
			output = outputR[counter];
			display = rows[counter];
			counter++;
			if(counter > 4)
				counter = 0;
			break;
		default:
			break;
	}
	PORTC = output;
	PORTD = display;
	return state;
}


unsigned long int findGCD(unsigned long int a, unsigned long int b) {
	unsigned long int c;
	while(1) {
		c = a%b;
		if (c == 0) {return b;}
		a = b;
		b = c;
	}
	return 0;
}

int main(void) {
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	static task task1, task2, task3, task4, task5;
	task *tasks[] = { &task1, &task2, &task3, &task4, &task5 };
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	const char start = -1;

	task1.state = start;
	task1.period = 90;
	task1.elapsedTime = task1.period;
	task1.TickFct = &Pong_Tick;

	task2.state = start;
        task2.period = 100;
        task2.elapsedTime = task2.period;
        task2.TickFct = &AI_Tick;

	task3.state = start;
        task3.period = 1;
        task3.elapsedTime = task3.period;
        task3.TickFct = &rand_Tick;

	task4.state = start;
	task4.period = 100;
	task4.elapsedTime = task4.period;
	task4.TickFct = &ball_Tick;

	task5.state = start;
        task5.period = 1;
        task5.elapsedTime = task5.period;
        task5.TickFct = &displaySMTick;

	unsigned short i;
	unsigned long GCD = tasks[0]->period;
	for(i = 0; i < numTasks; i++) {
		GCD = findGCD(GCD, tasks[i]->period);
	}

	TimerSet(GCD);
	TimerOn();

    /* Insert your solution below */
    while (1) {
	for(i = 0; i < numTasks; i++) {
		if(tasks[i]->elapsedTime == tasks[i]->period) {
			tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
			tasks[i]->elapsedTime = 0;
		}
		tasks[i]->elapsedTime += GCD;
	}
	while(!TimerFlag);
	TimerFlag = 0;
    }
    return 0;
}
