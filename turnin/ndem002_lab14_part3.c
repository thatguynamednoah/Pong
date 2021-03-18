/*	Author: Noah De Mers
 *  Partner(s) Name: 
 *	Lab Section: 023
 *	Assignment: Lab #14  Exercise #3
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Demo Link: https://www.youtube.com/watch?v=Sv1MU4nwq4o
 */
#include <avr/io.h>
#include "timer.h"
#include "scheduler.h"
#include <stdlib.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

void A2D_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
}

void transmit_data(unsigned char data, unsigned char reg) {
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
		// Should Not Reach This Point
	}
	PORTC |= 0x00;
}

// Global Variables
unsigned char gameReset;
unsigned char gamePaused;
unsigned char winner;
unsigned char winMax;
unsigned char leftScore;
unsigned char rightScore;
unsigned char display;

const unsigned char upMax = 0xFE;
const unsigned char downMax = 0xEF;
const unsigned char leftMax = 0x80;
const unsigned char rightMax = 0x01;

unsigned char xBallCoord;
unsigned char yBallCoord;
char xBallDirect;
char yBallDirect;
unsigned char speed;

unsigned char xRightPaddleCoord;
unsigned char yRightPaddleCoord;

unsigned char xLeftPaddleCoord;
unsigned char yLeftPaddleCoord;
unsigned char aiEnable;

//-----------------------------
// Menu
//-----------------------------
enum MS_States { menu_wait, menu_release, game_length, game_length_release, game_top, game_top_release, play_game, play_game_release, pause_display, pause_release, score_display, final_score, reset_release };
int MenuSystem(int state) {
	static unsigned short joystickInput;
	static unsigned char winCounted;
	switch (state) {
		case menu_wait:
			joystickInput = ADC;
			if (joystickInput > 530) { //Two Players
				display = 0;
			}
			else if (joystickInput < 500) { //One Player
				display = 1;
			}
			else {
				// Maintain Current Value
			}

			if ((~PINB & 0x04) == 0x04) {
				if (display == 0) {
					aiEnable = 0;
				}
				else {
					aiEnable = 1;
				}
				state = menu_release;
				display = 2;
			}
			else {
				state = menu_wait;
			}
			break;
		case menu_release:
			if ((~PINB & 0x0C) == 0x00) {
				state = game_length;
			}
			else {
				state = menu_release;
			}
			break;
		case game_length: // Select how many rounds will be played
			joystickInput = ADC;
                        if (joystickInput > 530) {
                                if (display < 4) {
					display += 1;
				}
                        }
                        else if (joystickInput < 500) {
                                if (display > 2) {
					display -= 1;
				}
                        }
                        else {
                                // Maintain Current Value
                        }

			if ((~PINB & 0x0C) == 0x04) {
				if (display == 2) { // 3-5
					winMax = 3;
				}
				else if (display == 3) { // 2-3
					winMax = 2;
				}
				else { // 1
					winMax = 1;
				}
				state = game_length_release;
				display = 5;
			}
			else if ((~PINB & 0x0C) == 0x08) {
				state = reset_release;
				winner = 0;
				gameReset = 1;
				display = 0;
			}
			else {
				state = game_length;
			}
			break;
		case game_length_release:
			if ((~PINB & 0x0C) == 0x00) {
				state = game_top;
				leftScore = 0;
				rightScore = 0;
			}
			else {
				state = game_length_release;
			}
			break;
		case game_top: // Waits for button press to start the game.
			if ((~PINB & 0x0C) == 0x04) {
				state = game_top_release;
			}
			else if ((~PINB & 0x0C) == 0x08) {
                                state = reset_release;
                                winner = 0;
                                gameReset = 1;
                                display = 0;
				leftScore = 0;
				rightScore = 0;
                        }
			else {
				state = game_top;
			}
			break;
		case game_top_release:
			if ((~PINB & 0x0C) == 0x00) {
                                state = play_game;
				display = 6;
				gamePaused = 0;
				gameReset = 0;
				winner = 0;
                        }
                        else {
                                state = game_top_release;
                        }
                        break;
		case play_game:
			if ((~PINB & 0x0C) == 0x04) {
				state = play_game_release;
				gamePaused = 1;
				display = 7;
			}
			else if ((~PINB & 0x0C) == 0x08) {
                                state = reset_release;
                                winner = 0;
                                gameReset = 1;
				leftScore = 0;
				rightScore = 0;
				gamePaused = 1;
                                display = 0;
                        }
			else if ((winner > 0) && (winCounted == 0)) {
				if (winner == 1) {
					rightScore += 1;
				}
				else {
					leftScore += 1;
				}
				winCounted = 1;
				winner = 0;
				gameReset = 1;
				display = 7;
				if ((leftScore == winMax) || (rightScore == winMax)) {
					state = final_score;
				}
				else {
					state = score_display;
				}

			}
			else {
				state = play_game;
				winCounted = 0;
			}
			break;
		case play_game_release:
			if ((~PINB & 0x0C) == 0x00) {
                                state = pause_display;
                        }
                        else {
                                state = play_game_release;
                        }
                        break;
		case pause_display:
			if ((~PINB & 0x0C) == 0x04) {
                                state = pause_release;
                        }
                        else if ((~PINB & 0x0C) == 0x08) {
                                state = reset_release;
                                winner = 0;
                                gameReset = 1;
                                display = 0;
                        }
                        else {
                                state = pause_display;
                        }
                        break;
		case pause_release:
			if ((~PINB & 0x0C) == 0x00) {
                                state = play_game;
				display = 6;
				gamePaused = 0;
                        }
                        else {
                                state = pause_release;
                        }
                        break;
		case score_display:
			if ((~PINB & 0x0C) == 0x04) {
                                state = game_top;
                                display = 5;
				winner = 0;
                        }
                        else if ((~PINB & 0x0C) == 0x08) {
                                state = reset_release;
                                winner = 0;
                                gameReset = 1;
                                display = 0;
                        }
                        else {
                                state = score_display;
				winner = 0;
                        }
			break;
		case final_score:
			if ((~PINB & 0x0C) > 0x00) {
                                state = reset_release;
                                display = 0;
				gameReset = 1;
				gamePaused = 1;
				winner = 0;
				leftScore = 0;
				rightScore = 0;
				winMax = 0;
				winCounted = 0;
                        }
                        else {
                                state = final_score;
                        }
			break;
		case reset_release:
			if ((~PINB & 0x0C) == 0x00) {
				state = menu_wait;
			}
			else {
				state = reset_release;
			}
			break;
		default:
			state = menu_wait;
			display = 0;
			gameReset = 1;
			gamePaused = 1;
			winner = 0;
			leftScore = 0;
			rightScore = 0;
			winMax = 0;
			winCounted = 0;
			break;
	}
	return state;
}

//-----------------------------
// Bit Shifting Functions
// ----------------------------
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
enum BM_States { ball_check };
int BallMovement(int state) {
	static unsigned char i;
	switch(state) {
		case ball_check:
			if (gameReset == 0) {
				if (gamePaused == 0) {
					if (i >= speed) {
						if (xBallDirect > 0) { //Shift Right
							xBallCoord = ShiftRight(xBallCoord);
						}
						else if (xBallDirect < 0) { // Shift Left
							xBallCoord = ShiftLeft(xBallCoord);
						}
						else {
							// Should Not Reach This Point
						}
		
						if (yBallCoord == 0xFE) { // Flip Y-Direction to Down
							yBallDirect = -1;
							yBallCoord = ShiftDown(yBallCoord);
						}
						else if (yBallCoord == 0xEF) { // Flip Y-Direction to Up
							yBallDirect = 1;
							yBallCoord = ShiftUp(yBallCoord);
						}
						else {
							if (yBallDirect > 0) { //Shift Up
								yBallCoord = ShiftUp(yBallCoord);
							}
							else if (yBallDirect < 0) { //Shift Up
								yBallCoord = ShiftDown(yBallCoord);
							}
							else { // yBallDirect == 0
								// Maintain Current Value
							}
						}
					}

					i += 1;
					if (i > 3) {
                	                	i = 0;
                        		}
				}
				else { // gamePaused == 1
					// Maintain Current Value
				}
			}
			else { // gameReset == 1
				xBallCoord = 0x08;
                	        xBallDirect = 1;
        	                yBallCoord = 0xFB;
	                        yBallDirect = 0;
				i = 3; 
				speed = 3;
			}
			state = ball_check;
			break;
		default:
			state = ball_check;
			xBallCoord = 0x08;
			xBallDirect = 1;
			yBallCoord = 0xFB;
			yBallDirect = 0;
			i = 3;
			speed = 3;
			break;
	}
	return state;
}

//-----------------------------
// Right Paddle Movement
// ----------------------------
enum RPM_States { check_r_paddle };
int RightPaddleMovement(int state) {
	static unsigned short joystickInput;
	switch (state) {
		case check_r_paddle:
			if (gameReset == 0) {
				if (gamePaused == 0) {
					joystickInput = ADC;
					if ((yRightPaddleCoord != 0xF8) && (joystickInput > 530)) {
						yRightPaddleCoord = ShiftUp(yRightPaddleCoord);
					}
					else if ((yRightPaddleCoord != 0xE3) && (joystickInput < 500)) {
						yRightPaddleCoord = ShiftDown(yRightPaddleCoord);
					}
					else {
						// Maintain Current Value
					}
				}
				else { // gamePaused == 1
					// Maintain Current Value
				}
			}
			else { // gameReset == 1
				yRightPaddleCoord = 0xF1;
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
enum LPM_States { check_l_paddle };
int LeftPaddleMovement(int state) {
	static unsigned char hasLeft;
	int determineMovement;
        switch (state) {
                case check_l_paddle:
			if (gameReset == 0) {
				if (gamePaused == 0) {
					if (aiEnable == 1) {
						if (xBallCoord == 0x10) {
							hasLeft = 0;
						}
	
						if ((xBallCoord == 0x20) && (hasLeft == 0)) {
							determineMovement = rand() % 10;

							if (yBallDirect == -1) {
								if ((determineMovement < 5) || (determineMovement == 7)) {
									if (yLeftPaddleCoord != 0xE3) {
										yLeftPaddleCoord = ShiftDown(yLeftPaddleCoord);
									}
								}
								else if ((determineMovement == 6) || (determineMovement == 9)) {
        	        	                                        if (yLeftPaddleCoord != 0xF8) {
                	        	                                        yLeftPaddleCoord = ShiftUp(yLeftPaddleCoord);
                        	        	                        }
                                	        	        }
                                        	        	else {
	                                        	                // Maintain Current Value
								}
							}
							else if (yBallDirect == 1) {
								if ((determineMovement < 5) || (determineMovement == 7)) {
                                	        	                if (yLeftPaddleCoord != 0xF8) {
                                        	        	                yLeftPaddleCoord = ShiftUp(yLeftPaddleCoord);
                                                	        	}
	                                                	}
								else if ((determineMovement == 6) || (determineMovement == 9)) {
									if (yLeftPaddleCoord != 0xE3) {
										yLeftPaddleCoord = ShiftDown(yLeftPaddleCoord);
									}
								}
								else {
									// Maintain Current Value
								}
							}
							else { // yBallDirect == 0
								if ((determineMovement < 5) || (determineMovement == 7)) {
                        		                                if (yLeftPaddleCoord == 0xF8) {
                                		                                yLeftPaddleCoord = ShiftDown(yLeftPaddleCoord);
                                        		                }
									else if (yLeftPaddleCoord == 0xE3) {
										yLeftPaddleCoord = ShiftUp(yLeftPaddleCoord);
									}
									else {
										// Maintain Current Value
									}
                        	        	                }
								else {
									// Maintain Current Value
								}
							}
							hasLeft = 1;
						}
					}
					else { // aiEnable == 0
			                        if ((yLeftPaddleCoord != 0xF8) && ((~PINB & 0x07) == 0x02)) {
        			                        yLeftPaddleCoord = ShiftUp(yLeftPaddleCoord);
                			        }
                        			else if ((yLeftPaddleCoord != 0xE3) && ((~PINB & 0x07) == 0x01)) {
	                                		yLeftPaddleCoord = ShiftDown(yLeftPaddleCoord);
	        	                	}
        	        	        	else {
                	        	        	// Maintain Current Value
	                	        	}
					}
				}
				else { // gamePaused == 1
					// Maintain Current Value
				}
			}
			else { // gameReset == 1;
				xLeftPaddleCoord = 0x80;
				yLeftPaddleCoord = 0xF1;
				hasLeft = 0;
			}
                        state = check_l_paddle;
                        break;
                default:
                        state = check_l_paddle;
                        xLeftPaddleCoord = 0x80;
                        yLeftPaddleCoord = 0xF1;
			hasLeft = 0;
                        break;
        }
        return state;
}

//----------------------------
// Ball Paddle Interaction
//----------------------------
enum BPI_States { paddle_check };
int BallPaddleInteraction(int state) {
	static unsigned char yLeftPaddleCoordPrev;
	static unsigned char yRightPaddleCoordPrev;
	static unsigned char hasLeft;
	switch (state) {
		case paddle_check:
			// Right Paddle Checks
			if ((xBallCoord != 0x02) && (xBallCoord != 0x40)) {
				hasLeft = 0;
			}

			if ((yRightPaddleCoordPrev != yRightPaddleCoord) && (xBallCoord == 0x02) && (hasLeft == 0)) { // Checks for instances by which a Speed Increase or Decrease should occur
				if ((yRightPaddleCoord > yRightPaddleCoordPrev) && ((yBallDirect == 1) || (yBallDirect == 0))) {
					if (speed > 1) {
						speed -= 1;
					}
				}
				else if ((yRightPaddleCoord > yRightPaddleCoordPrev) && (yBallDirect == -1)) {
					if (speed < 3) {
						speed += 1;
					}
				}
				else if ((yRightPaddleCoord < yRightPaddleCoordPrev) && ((yBallDirect == -1) || (yBallDirect == 0))) {
					if (speed > 1) {
						speed -= 1;
					}
				}
				else { // ((yRightPaddleCoord < yRightPaddleCoordPrev) && (yBallDirect == 1))
					if (speed < 3) {
						speed += 1;
					}
				}
				hasLeft = 1;
			}
			if ((yRightPaddleCoord == 0xF8) && (xBallCoord == 0x02)) { // Determine Ball Direction when Paddle is High
				if (yBallCoord == 0xFE) {
					xBallDirect = -1;
					yBallDirect = -1;
				}
				else if (yBallCoord == 0xFD) {
					xBallDirect = -1;
					yBallDirect = 0;
					if ((speed < 3) && (hasLeft == 0)) {
						speed += 1;
						hasLeft = 1;
					}
				}
				else if (yBallCoord == 0xFB) {
					xBallDirect = -1;
					if (yBallDirect == 0) {
						yBallDirect = -1;
					}
				}
				else if ((yBallCoord == 0xF7) && (yBallDirect != 0)) {
					xBallDirect = -1;
					yBallDirect = -1;
					if ((speed > 1) && (hasLeft == 0)) {
						speed -= 1;
						hasLeft = 1;
					}
				}
				else {
					winner = 2;
				}
			}
			else if ((yRightPaddleCoord == 0xF1) && (xBallCoord == 0x02)) { // Determine Ball Direction when Paddle is Mid
				if ((yBallCoord == 0xFE) && (yBallDirect != 0)) {
					xBallDirect = -1;
					yBallDirect = 1;
					if ((speed > 1) && (hasLeft == 0)) {
						speed -= 1;
						hasLeft = 1;
					}
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
					if ((speed < 3) && (hasLeft == 0)) {
						speed += 1;
						hasLeft = 1;
					}
				}
				else if (yBallCoord == 0xF7) {
					xBallDirect = -1;
					if (yBallDirect == 0) {
						yBallDirect = -1;
					}
				}
				else if ((yBallCoord == 0xEF) && (yBallDirect != 0)) {
					xBallDirect = -1;
					yBallDirect = -1;
					if ((speed > 1) && (hasLeft == 0)) {
						speed -= 1;
						hasLeft = 1;
					}
				}
				else {
					winner = 2;
				}
			}
			else if ((yRightPaddleCoord == 0xE3) && (xBallCoord == 0x02)) { // Determine Ball Direction when Paddle is Low
				if ((yBallCoord == 0xFD) && (yBallDirect != 0)) {
					xBallDirect = -1;
					yBallDirect = 1;
					if ((speed > 1) && (hasLeft == 0)) {
						speed -= 1;
						hasLeft = 1;
					}
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
					if ((speed < 3) && (hasLeft == 0)) {
						speed += 1;
						hasLeft = 1;
					}
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
				// Should Not Reach This Point
			}
			
			// Left Paddle Checks
			if ((yLeftPaddleCoordPrev != yLeftPaddleCoord) && (xBallCoord == 0x40) && (hasLeft == 0)) { // Checks for instances by which a Speed Increase or Decrease should occur
				if ((yLeftPaddleCoord > yLeftPaddleCoordPrev) && ((yBallDirect == 1) || (yBallDirect == 0))) {
					if (speed > 1) {
						speed -= 1;
					}
				}
				else if ((yLeftPaddleCoord > yLeftPaddleCoordPrev) && (yBallDirect == -1)) {
					if (speed < 3) {
						speed += 1;
					}
				}
				else if ((yLeftPaddleCoord < yLeftPaddleCoordPrev) && ((yBallDirect == -1) || (yBallDirect == 0))) {
					if (speed > 1) {
						speed -= 1;
					}
				}
				else { // ((yLeftPaddleCoord < yLeftPaddleCoordPrev) && (yBallDirect == 1))
					if (speed < 3) {
						speed += 1;
					}
				}
				hasLeft = 1;
			}
			if ((yLeftPaddleCoord == 0xF8) && (xBallCoord == 0x40)) { // Determine Ball Direction when Paddle is High
				if (yBallCoord == 0xFE) {
					xBallDirect = 1;
					yBallDirect = -1;
				}
				else if (yBallCoord == 0xFD) {
					xBallDirect = 1;
					yBallDirect = 0;
					if ((speed < 3) && (hasLeft == 0)) {
						speed += 1;
						hasLeft = 1;
					}
				}
				else if (yBallCoord == 0xFB) {
					xBallDirect = 1;
					if (yBallDirect == 0) {
						yBallDirect = -1;
					}
				}
				else if ((yBallCoord == 0xF7) && (yBallDirect != 0)) {
					xBallDirect = 1;
					yBallDirect = -1;
					if ((speed > 1) && (hasLeft == 0)) {
						speed -= 1;
						hasLeft = 1;
					}
				}
				else {
					winner = 1;
				}
			}
			else if ((yLeftPaddleCoord == 0xF1) && (xBallCoord == 0x40)) { // Determine Ball Direction when Paddle is Mid
				if ((yBallCoord == 0xFE) && (yBallDirect != 0)) {
					xBallDirect = 1;
					yBallDirect = 1;
					if ((speed > 1) && (hasLeft == 0)) {
						speed -= 1;
						hasLeft = 1;
					}
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
					if ((speed < 3) && (hasLeft == 0)) {
						speed += 1;
						hasLeft = 1;
					}
				}
				else if (yBallCoord == 0xF7) {
					xBallDirect = 1;
					if (yBallDirect == 0) {
						yBallDirect = -1;
					}
				}
				else if ((yBallCoord == 0xEF) && (yBallDirect != 0)) {
					xBallDirect = 1;
					yBallDirect = -1;
					if ((speed > 1) && (hasLeft == 0)) {
						speed -= 1;
						hasLeft = 1;
					}
				}
				else {
					winner = 1;
				}
			}
			else if ((yLeftPaddleCoord == 0xE3) && (xBallCoord == 0x40)) { // Determine Ball Direction when Paddle is Low
				if ((yBallCoord == 0xFD) && (yBallDirect != 0)) {
					xBallDirect = 1;
					yBallDirect = 1;
					if ((speed > 1) && (hasLeft == 0)) {
						speed -= 1;
						hasLeft = 1;
					}
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
					if ((speed < 3) && (hasLeft == 0)) {
						speed += 1;
						hasLeft = 1;
					}
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
				// Should Not Reach This Point
			}

			yLeftPaddleCoordPrev = yLeftPaddleCoord;
			yRightPaddleCoordPrev = yRightPaddleCoord;
			state = paddle_check;
			break;
		default:
			state = paddle_check;
			winner = 0;
			hasLeft = 0;
			yLeftPaddleCoordPrev = 0xF1;
			yRightPaddleCoordPrev = 0xF1;
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
	static unsigned char clear;
	switch (state) {
		case output_pong:
			state = output_pong;
			break;
		default:
			state = output_pong;
			i = 0x80;
			clear = 0;
			x = 0x00;
			y = 0xFF;
			break;
	}
	switch (state) {
		case output_pong:
			if (display == 0) { //Outputs Menu for Two Players
				x = 0x14;
				y = 0xFB;
			}
			else if (display == 1) { //Outputs Menu for One Player
				x = 0x10;
				y = 0xFB;
			}
			else if (display == 2) { //Outputs Menu for 3 of 5 Game Wins
				if (i == 0x80) {
					x = 0x80;
					y = 0xE0;
				}
				else if (i == 0x01) {
					x = 0x01;
					y = 0xF8;
				}
				else {
					x = 0x00;
					y = 0xFF;
				}

				i = i >> 1;
				if (i == 0x00) {
					i = 0x80;
				}
			}
			else if (display == 3) { //Outputs Menu for 2 of 3 Game Wins
                                if (i == 0x80) {
                                        x = 0x80;
                                        y = 0xF8;
                                }
                                else if (i == 0x01) {
                                        x = 0x01;
                                        y = 0xFC;
                                }
                                else { 
                                        x = 0x00;
                                        y = 0xFF;
                                }

                                i = i >> 1;
                                if (i == 0x00) {
                                        i = 0x80;
                                }
                        }
			else if (display == 4) { //Outputs Menu for 1 Game
                                if (i == 0x80) {
                                        x = 0x80;
                                        y = 0xFE;
                                }
                                else if (i == 0x01) {
                                        x = 0x01;
                                        y = 0xFE;
                                }
                                else { 
                                        x = 0x00;
                                        y = 0xFF;
                                }

                                i = i >> 1;
                                if (i == 0x00) {
                                        i = 0x80;
                                }
                        }
			else if ((display == 5) || (display == 6)) { //Outputs Game Setup (MenuSystem state = game_top) OR Current Gameplay (MenuSystem state = play_game)
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
				if (clear == 0) {
					i = i >> 1;
					if (i == 0x00) {
						i = 0x80;
					}
					clear = 1;
				}
				else {
					x = 0x00;
					y = 0xFF;
					clear = 0;
				}
			}
			else if (display == 7) { //Outputs Current Score
				if (i == 0x80) {
					if (leftScore == 0) {
						x = 0x00;
						y = 0xFF;
					}
					else if (leftScore == 1) {
						x = 0x80;
						y = 0xFE;
					}
					else if (leftScore == 2) {
						x = 0x80;
						y = 0xFC;
					}
					else if (leftScore == 3) {
						x = 0x80;
						y = 0xF8;
					}
					else { //Error State
						x = 0xFF;
						y = 0xE0;
					}
				}
				else if (i == 0x01) {
					if (rightScore == 0) {
                                                x = 0x00;
                                                y = 0xFF;
                                        }
                                        else if (rightScore == 1) {
                                                x = 0x01;
                                                y = 0xFE;
                                        }
                                        else if (rightScore == 2) {
                                                x = 0x01;
                                                y = 0xFC;
                                        }
                                        else if (rightScore == 3) {
                                                x = 0x01;
                                                y = 0xF8;
                                        }
                                        else { //Error State
                                                x = 0xFF;
                                                y = 0xE0;
                                        }
				}
				else {
					x = 0x18;
					y = 0xFB;
				}
				if (clear == 0) {
					i = i >> 1;
                        	        if (i == 0x00) {
                	                        i = 0x80;
					}
					clear = 1;
				}
				else {
					x = 0x00;
					y = 0xFF;
					clear = 0;
				}
			}
			else { // Error State
				x = 0xFF;
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

	static task task1, task2, task3, task4, task5, task6;
	task* tasks[] = { &task1, &task2, &task3, &task4, &task5, &task6 };
	const unsigned short numTasks = 6; //sizeof(task)/sizeof(task*); For some reason 5 was the max so numTasks is manually assigned 6.

	const char start = -1;
	task1.state = start;
        task1.period = 100;
        task1.elapsedTime = task1.period;
        task1.TickFct = &MenuSystem;

	task2.state = start;
	task2.period = 75;
	task2.elapsedTime = task2.period;
	task2.TickFct = &BallMovement;

	task3.state = start;
        task3.period = 150;
        task3.elapsedTime = task3.period;
        task3.TickFct = &RightPaddleMovement;

	task4.state = start;
        task4.period = 150;
        task4.elapsedTime = task4.period;
        task4.TickFct = &LeftPaddleMovement;

	task5.state = start;
        task5.period = 50;
        task5.elapsedTime = task5.period;
        task5.TickFct = &BallPaddleInteraction;

	task6.state = start;
        task6.period = 1;
        task6.elapsedTime = task6.period;
        task6.TickFct = &OutputPong;

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
