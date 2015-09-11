#include <stdio.h>
#include <stdlib.h>
#include "gomoku.h"

void RandAI_decideNextLocation(StoneLocation *to, GameEnvironment *env)
{
	int pos;
	
	pos = rand();
	for(;;){
		pos %= (BOARD_SIZE * BOARD_SIZE);
		if(env->mainBoard[pos] == STATE_NONE){
			break;
		}
		pos++;
	}
	to->y = pos / BOARD_SIZE;
	to->x = pos % BOARD_SIZE;
}
