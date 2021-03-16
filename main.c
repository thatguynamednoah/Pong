/*	Author: Noah De Mers
 *  Partner(s) Name: 
 *	Lab Section: 023
 *	Assignment: Lab #14  Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Demo Link: 
 */
#include <avr/io.h>
#include "timer.h"
#include "scheduler.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

void A2D_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
}

void transmit_data(unsigned char data, unsigned char reg) { //Column to register 1, row to register 2
	int i;
	if (reg == 1) {
		for (i = 0; i < 8; ++i) {
			PORTC = 0x08;
			PORTC |= ((data >> i) & 0x01);
			PORTC |= 0x02;
		}

		PORTC |= 0x04;
	}
	else if (reg == 2) {
		for (i = 0; i < 8; ++i) {
			PORTC = 0x20;
			PORTC |= ((data >> i) & 0x01);
			PORTC |= 0x02;
		}

		PORTC |= 0x10;
	}
	else {
		//should not reach this point
	}
	PORTC |= 0x00;
}

/*
unsigned char display;
unsigned char winner;
unsigned char leftScore;
unsigned char rightScore;
unsigned char singlePlayer;
//-----------------------------
// Menu Controls
//
// Display == 0: Menu (1)
// |      --
// | -  --- 
// |      --
// |- -
// |
// Display == 1: Menu (2)
// |
// | -   
// |      --
// |- - ---
// |      --
//
// Display == 2: Select Length (6-11)
// |---  - -
// |-    - -
// |---  - -
// |- -  - -
// |---  - -
// Display == 3: Select Length (5-9)
// |---  ---
// |-    - -
// |---  ---
// |  -    -
// |---    -
// Display == 4: Select Length (4-7)
// |- -  ---
// |- -    -  
// |---    -
// |  -    -
// |  -    -
// Display == 5: Select Length (3-5)
// |---  ---
// |  -  -  
// |---  ---
// |  -    -
// |---  ---
// Display == 6: Select Length (2-3)
// |---  ---
// |  -    -
// |---  ---
// |-      -
// |---  ---
// Display == 7: Select Length (1)
// |    -
// |    -
// |    - 
// |    -
// |    -
//
// Display == 8: Pong Setup (Going Left) Goes to Loser / Randon at Beginning
// |
// |-      -
// |-   -  -
// |-      -
// |
// Display == 9: Pong Setup (Going Right) Goes to Loser / Randon at Beginning
// |
// |-      -
// |-  -   -
// |-      -
// |
//
// Display == 10: Pong
//
// Display == 11: Display Winner (Left)
// |----
// |----
// |----
// |----
// |----
// Display == 12: Display Winner (Right)
// |    ----
// |    ----
// |    ----
// |    ----
// |    ----
//
// Display == 13: Pause / Show Score
// |---  ---
// |  -  -
// |---  ---
// |-    - -
// |---  --- 
//-----------------------------
enum MC_States { };
int MenuControls(int state) {
	static unsigned short joystickInput;
	switch (state) {
		case menu_wait:
			joystickInput = ADC;
			if (joystickInput > 530) {
				display = 0;
			else if (joystickInput < 500) {
				display = 1;
			}
			else {
				//Do Nothing	
			}

			if ((~PINB & 0x07) = 0x04) {
				if (display == 0) {
					singlePlayer = 0;
				}
				else {
					singlePlayer = 1;
				}
				state = choose_length;
				display = 5;
			}
			else {
				state = menu_wait;
			}
			break;
		case choose_length:
			if ((joystickInput > 530) && (display != 2)) {
				display -= 1;
			}
			else if ((joystickInput < 500) && (display != 7)) {
                                display += 1;
                        }	
			else {
				//Do Nothing
			}

			if ((~PINB & 0x07) = 0x04) {
                                state = wait_start;
                                display = 8; //Or = 9
                        }
                        else {
                                state = choose_length;
                        }
                        break;
		case wait_start:
			if ((~PINB & 0x07) = 0x04) {
                                state = wait_match_end;
                                display = 10;
                        }
                        else {
                                state = wait_start;
                        }
                        break;
		case wait_match_end:
			if ((display == 8) || (display == 9)) {
				state = wait_start;
			}
			else if (display == 11) {
				state = winner;
			}
			else if (display == 12) {
				state = winner;
			}
			else if (display == 0) {
				state = menu_wait;
			}
			else {
				state = wait_match_end;
			}
                        break;
		default:
			state = menu_wait;
			display = 0;
			winner = 0;
			leftScore = 0;
			rightScore = 0;
			singlePlayer = 0;
			break;
	}
}
*/

//-----------------------------
// Bit Shifting Functions
// ----------------------------
const unsigned char upMax = 0xFE;
const unsigned char downMax = 0xEF;
const unsigned char leftMax = 0x80;
const unsigned char rightMax = 0x01;

unsigned char ShiftLeft(unsigned char coordinate) {
	if (coordinate != leftMax) {
		coordinate = coordinate << 1;
	}
	return coordinate;
}

unsigned char ShiftRight(unsigned char coordinate) {
        if (coordinate != rightMax) {
                coordinate = coordinate >> 1;
        }
	return coordinate;
}

unsigned char ShiftUp(unsigned char coordinate) {
	if (coordinate != upMax) {
		coordinate = (coordinate >> 1) | 0x80;
	}
	return coordinate;
}

unsigned char ShiftDown(unsigned char coordinate) {
        if (coordinate != downMax) {
                coordinate = (coordinate << 1) | 0x01;
        }
	return coordinate;
}

//-----------------------------
// Ball Movement
// ----------------------------
unsigned char xBallCoord;
unsigned char yBallCoord;
char xBallDirect;
char yBallDirect;

enum BM_States { ball_check };
int BallMovement(int state) {
	switch(state) {
		case ball_check:
			if (xBallCoord == 0x40) { //Begin Right Transition
				xBallDirect = 1;
				xBallCoord = ShiftRight(xBallCoord);
			}/*
			else if (xBallCoord == 0x02) { //Begin Left Transition
				xBallDirect = -1;
				xBallCoord = ShiftLeft(xBallCoord);
			}*/
			else {
				if (xBallDirect > 0) {
					xBallCoord = ShiftRight(xBallCoord);
				}
				else if (xBallDirect < 0) {
					xBallCoord = ShiftLeft(xBallCoord);
				}
				else {
					//Do Nothing
				}
			}

			if (yBallCoord == 0xFE) { //Begin Down Transition
				yBallDirect = -1;
				yBallCoord = ShiftDown(yBallCoord);
			}
			else if (yBallCoord == 0xEF) { //Begin Up Transition
				yBallDirect = 1;
				yBallCoord = ShiftUp(yBallCoord);
			}
			else {
				if (yBallDirect > 0) {
					yBallCoord = ShiftUp(yBallCoord);
				}
				else if (yBallDirect < 0) {
					yBallCoord = ShiftDown(yBallCoord);
				}
				else {
					//Do Nothing
				}
			}
			state = ball_check;
			break;
		default:
			state = ball_check;
			xBallCoord = 0x08;
			xBallDirect = 1;
			yBallCoord = 0xFB;
			yBallDirect = 0;
			break;
	}
	return state;
}

//-----------------------------
// Right Paddle Movement
// ----------------------------
unsigned char xRightPaddleCoord;
unsigned char yRightPaddleCoord;

enum RPM_States { check_r_paddle };
int RightPaddleMovement(int state) {
	static unsigned short joystickInput;
	switch (state) {
		case check_r_paddle:
			joystickInput = ADC;
			if ((yRightPaddleCoord != 0xF8) && (joystickInput > 530)) {
				yRightPaddleCoord = ShiftUp(yRightPaddleCoord);
			}
			else if ((yRightPaddleCoord != 0xE3) && (joystickInput < 500)) {
				yRightPaddleCoord = ShiftDown(yRightPaddleCoord);
			}
			else {
				//Do Nothing
			}
			state = check_r_paddle;
			break;
		default:
			state = check_r_paddle;
			xRightPaddleCoord = 0x01;
			yRightPaddleCoord = 0xF1;
			break;
	}
	return state;
}

//-----------------------------
// Left Paddle Movement
// ----------------------------
unsigned char xLeftPaddleCoord;
unsigned char yLeftPaddleCoord;

enum LPM_States { check_l_paddle };
int LeftPaddleMovement(int state) {
        switch (state) {
                case check_l_paddle:
                        if ((yLeftPaddleCoord != 0xF8) && ((~PINB & 0x07) == 0x02)) {
                                yLeftPaddleCoord = ShiftUp(yLeftPaddleCoord);
                        }
                        else if ((yLeftPaddleCoord != 0xE3) && ((~PINB & 0x07) == 0x01)) {
                                yLeftPaddleCoord = ShiftDown(yLeftPaddleCoord);
                        }
                        else {
                                //Do Nothing
                        }
                        state = check_l_paddle;
                        break;
                default:
                        state = check_l_paddle;
                        xLeftPaddleCoord = 0x80;
                        yLeftPaddleCoord = 0xF1;
                        break;
        }
        return state;
}

//xBallCoord
//yBallCoord
//
//xBallDirect
//yBallDirect
//
//xRightPaddleCoord
//yRightPaddleCoord
//
//xLeftPaddleCoord
//yLeftPaddleCoord

//----------------------------
// Ball Paddle Interaction
//----------------------------
unsigned char winner;

enum BPI_States { paddle_check };
int BallPaddleInteraction(int state) {
	unsigned char yLeftPaddleCoordPrev;
	unsigned char yRightPaddleCoordPrev;
	switch (state) {
		case paddle_check:
			//----------
			// Right
			// ---------
			if ((yRightPaddleCoordPrev != yRightPaddleCoord) && (xBallCoord == 0x02)) {
				if ((yRightPaddleCoord > yRightPaddleCoordPrev) && ((yBallDirect == 1) || (yBallDirect == 0))) {
					//increase speed
				}
				else if ((yRightPaddleCoord > yRightPaddleCoordPrev) && (yBallDirect == -1)) {
					//decrease speed
				}
				else if ((yRightPaddleCoord < yRightPaddleCoordPrev) && ((yBallDirect == -1) || (yBallDirect == 0))) {
					//increase speed
				}
				else { // ((yRightPaddleCoord < yRightPaddleCoordPrev) && (yBallDirect == 1))
					//decrease speed
				}
			}
			if ((yRightPaddleCoord == 0xF8) && (xBallCoord == 0x02)) {
				if (yBallCoord == 0xFE) {
					xBallDirect = -1;
					yBallDirect = -1;
				}
				else if (yBallCoord == 0xFD) {
					xBallDirect = -1;
					yBallDirect = 0;
				}
				else if (yBallCoord == 0xFB) {
					xBallDirect = -1;
					if (yBallDirect == 0) {
						yBallDirect = -1;
					}
				}
				else if (yBallCoord == 0xF7) {
					xBallDirect = -1;
					yBallDirect = -1;
					//increase speed
				}
				else {
					winner = 2;
				}
			}
			else if ((yRightPaddleCoord == 0xF1) && (xBallCoord == 0x02)) {
				if (yBallCoord == 0xFE) {
					xBallDirect = -1;
					yBallDirect = 1;
					//increase speed
				}
				else if (yBallCoord == 0xFD) {
					xBallDirect = -1;
					if (yBallDirect == 0) {
						yBallDirect = 1;
					}
				}
				else if (yBallCoord == 0xFB) {
					xBallDirect = -1;
					yBallDirect = 0;
				}
				else if (yBallCoord == 0xF7) {
					xBallDirect = -1;
					if (yBallDirect == 0) {
						yBallDirect = -1;
					}
				}
				else if (yBallCoord == 0xEF) {
					xBallDirect = -1;
					yBallDirect = -1;
					//increase speed
				}
				else {
					winner = 2;
				}
			}
			else if ((yRightPaddleCoord == 0xE3) && (xBallCoord == 0x02)) {
				if (yBallCoord == 0xFD) {
					xBallDirect = -1;
					yBallDirect = 1;
					//increase speed
				}
				else if (yBallCoord == 0xFB) {
					xBallDirect = -1;
					if (yBallDirect == 0) {
						yBallDirect = 1;
					}
				}
				else if (yBallCoord == 0xF7) {
					xBallDirect = -1;
					yBallDirect = 0;
				}
				else if (yBallCoord == 0xEF) {
					xBallDirect = -1;
					yBallDirect = 1;
				}
				else {
					winner = 2;
				}
			}
			else {
				//Do Nothing
			}
			
			//-----------
			// Left
			//-----------
			if ((yLeftPaddleCoordPrev != yLeftPaddleCoord) && (xLeftCoord == 0x40)) {
				if ((yLeftPaddleCoord > yLeftPaddleCoordPrev) && ((yBallDirect == 1) || (yBallDirect == 0))) {
					//increase speed
				}
				else if ((yLeftPaddleCoord > yLeftPaddleCoordPrev) && (yBallDirect == -1)) {
					//decrease speed
				}
				else if ((yLeftPaddleCoord < yLeftPaddleCoordPrev) && ((yBallDirect == -1) || (yBallDirect == 0))) {
					//increase speed
				}
				else { // ((yLeftPaddleCoord < yLeftPaddleCoordPrev) && (yBallDirect == 1))
					//decrease speed
				}
			}
			if ((yLeftPaddleCoord == 0xF8) && (xBallCoord == 0x40)) {
				if (yBallCoord == 0xFE) {
					xBallDirect = 1;
					yBallDirect = -1;
				}
				else if (yBallCoord == 0xFD) {
					xBallDirect = 1;
					yBallDirect = 0;
				}
				else if (yBallCoord == 0xFB) {
					xBallDirect = 1;
					if (yBallDirect == 0) {
						yBallDirect = -1;
					}
				}
				else if (yBallCoord == 0xF7) {
					xBallDirect = 1;
					yBallDirect = -1;
					//increase speed
				}
				else {
					winner = 1;
				}
			}
			else if ((yLeftPaddleCoord == 0xF1) && (xBallCoord == 0x40)) {
				if (yBallCoord == 0xFE) {
					xBallDirect = 1;
					yBallDirect = 1;
					//increase speed
				}
				else if (yBallCoord == 0xFD) {
					xBallDirect = 1;
					if (yBallDirect == 0) {
						yBallDirect = 1;
					}
				}
				else if (yBallCoord == 0xFB) {
					xBallDirect = 1;
					yBallDirect = 0;
				}
				else if (yBallCoord == 0xF7) {
					xBallDirect = 1;
					if (yBallDirect == 0) {
						yBallDirect = -1;
					}
				}
				else if (yBallCoord == 0xEF) {
					xBallDirect = 1;
					yBallDirect = -1;
					//increase speed
				}
				else {
					winner = 1;
				}
			}
			else if ((yLeftPaddleCoord == 0xE3) && (xBallCoord == 0x40)) {
				if (yBallCoord == 0xFD) {
					xBallDirect = 1;
					yBallDirect = 1;
					//increase speed
				}
				else if (yBallCoord == 0xFB) {
					xBallDirect = 1;
					if (yBallDirect == 0) {
						yBallDirect = 1;
					}
				}
				else if (yBallCoord == 0xF7) {
					xBallDirect = 1;
					yBallDirect = 0;
				}
				else if (yBallCoord == 0xEF) {
					xBallDirect = 1;
					yBallDirect = 1;
				}
				else {
					winner = 1;
				}
			}
			else {
				//Do Nothing
			}
			state = paddle_check;
			break;
		default:
			state = paddle_check;
			winner = 0;
			yLeftPaddleCoordPrev = yLeftPaddleCoord;
			yRightPaddleCoordPrev = yRightPaddleCoord;
			break;
	}
	return state;
}

//----------------------------
// Output Pong
//----------------------------
enum OP_States { output_pong };
int OutputPong(int state) {
	static unsigned char i;
	static unsigned char x;
	static unsigned char y;
	switch (state) {
		case output_pong:
			state = output_pong;
			break;
		default:
			state = output_pong;
			i = 0x80;
			break;
	}
	switch (state) {
		case output_pong:
			if (winner == 0) {
			if (i == 0x80) {
				x = xLeftPaddleCoord;
				y = yLeftPaddleCoord;
			} 
			else if (i == 0x01) {
				x = xRightPaddleCoord;
				y = yRightPaddleCoord;
			}
			else {
				if ((xBallCoord & i) > 0) {
					x = xBallCoord;
					y = yBallCoord;
				}
				else {
					x = 0x00;
					y = 0xFF;
				}
			}
			i = i >> 1;
			if (i == 0x00) {
				i = 0x80;
			}
			}
			else if (winner == 2) {
				x = 0xF0;
				y = 0xE0;
			}
			else {
				x = 0x0F;
				y = 0xE0;
			}
			transmit_data(x, 1);
			transmit_data(y, 2);
			break;
		default:
			break;
	}
	return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRB = 0x00; PORTB = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
    /* Insert your solution below */
	A2D_init();

	static task task1, task2, task3, task4, task5;
	task* tasks[] = { &task1, &task2, &task3, &task4, &task5 };
	const unsigned short numTasks = sizeof(task)/sizeof(task*);

	const char start = -1;
	task1.state = start;
	task1.period = 250;
	task1.elapsedTime = task1.period;
	task1.TickFct = &BallMovement;

	task2.state = start;
        task2.period = 150;
        task2.elapsedTime = task2.period;
        task2.TickFct = &RightPaddleMovement;

	task3.state = start;
        task3.period = 150;
        task3.elapsedTime = task3.period;
        task3.TickFct = &LeftPaddleMovement;

	task4.state = start;
        task4.period = 50;
        task4.elapsedTime = task4.period;
        task4.TickFct = &BallPaddleInteraction;

	task5.state = start;
        task5.period = 1;
        task5.elapsedTime = task5.period;
        task5.TickFct = &OutputPong;

	unsigned long GCD = tasks[0]->period;
	unsigned short i;
	for (i = 1; i < numTasks; i++) {
		GCD = findGCD(GCD, tasks[i]->period);
	}

	TimerSet(GCD);
	TimerOn();
    while (1) {
	for (i = 0; i < numTasks; i++) {
		if (tasks[i]->elapsedTime == tasks[i]->period ) {
			tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
			tasks[i]->elapsedTime = 0;
		}
		
		tasks[i]->elapsedTime += GCD;
	}

	while(!TimerFlag);
	TimerFlag = 0;
    }
    return 1;
}
