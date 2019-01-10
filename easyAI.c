#include <stdio.h>
#include <stdlib.h>
#include "gomoku.h"

#define EVAL_CANNOT_PUT		(-0xffffff)
#define EVAL_NEUTRAL		(0)
#define EVAL_CHECKMATE		(0xffffff)
#define EVAL_DANGER			(0xfff)
#define EVAL_FACTOR_DIST_FROM_CENTER	5
#define EVAL_FACTOR_CAN_PUT_IN_ROW		7

int EasyAI_getRowPotential(StoneRow *row, char *board);

void EasyAI_decideNextLocation(StoneLocation *to, GameEnvironment *env)
{
	int evalMap[BOARD_SIZE*BOARD_SIZE];
	int x, y, p, i;
	int bestEval;
	
	for(y = 0; y < BOARD_SIZE; y++){
		for(x = 0; x < BOARD_SIZE; x++){
			if(env->mainBoard[y * BOARD_SIZE + x] != STATE_NONE){
				evalMap[y * BOARD_SIZE + x] = EVAL_CANNOT_PUT;
			} else{
				evalMap[y * BOARD_SIZE + x] = EVAL_NEUTRAL;
				p = 0;
				p += (x > (BOARD_SIZE >> 1) ? BOARD_SIZE - 1 - x : x);
				p += (y > (BOARD_SIZE >> 1) ? BOARD_SIZE - 1 - y : y);
				p *= EVAL_FACTOR_DIST_FROM_CENTER;
				evalMap[y * BOARD_SIZE + x] += p;
			}
		}
	}
	// 自分の列を伸ばす戦略
	for(i = 0; i < ROW_LIST_LENGTH; i++){
		if(env->rowList[i].length <= 0){
			break;
		}
		if(env->rowList[i].col != env->currentColor){
			continue;
		}
		if(EasyAI_getRowPotential(&env->rowList[i], env->mainBoard) < 5){
			continue;
		}
		if(env->rowList[i].endType & ENDTYPE_CAN_PUT){
			// can put before start point.
			x = env->rowList[i].start.x;
			y = env->rowList[i].start.y;
			getLocationOnDirection(&x, &y, env->rowList[i].direction, -1);
			p = EVAL_FACTOR_CAN_PUT_IN_ROW * env->rowList[i].length;
			p += ((env->rowList[i].length == 4) ? EVAL_CHECKMATE : 0);
			evalMap[y * BOARD_SIZE + x] += p;
		}
		if((env->rowList[i].endType >> 1) & ENDTYPE_CAN_PUT){
			// can put after end point.
			x = env->rowList[i].start.x;
			y = env->rowList[i].start.y;
			getLocationOnDirection(&x, &y, env->rowList[i].direction, env->rowList[i].length);
			p = EVAL_FACTOR_CAN_PUT_IN_ROW * env->rowList[i].length;
			p += ((env->rowList[i].length == 4) ? EVAL_CHECKMATE : 0);
			evalMap[y * BOARD_SIZE + x] += p;
		}
	}
	// 相手の列を抑える戦略
	for(i = 0; i < ROW_LIST_LENGTH; i++){
		if(env->rowList[i].length <= 0){
			break;
		}
		if(env->rowList[i].col == env->currentColor){
			continue;
		}
		if(env->rowList[i].length < 3){
			continue;
		}
		if(env->rowList[i].length == 3 && env->rowList[i].endType != 3){
			continue;
		}
		if(EasyAI_getRowPotential(&env->rowList[i], env->mainBoard) < 5){
			continue;
		}
		if(env->rowList[i].endType & ENDTYPE_CAN_PUT){
			// can put before start point.
			x = env->rowList[i].start.x;
			y = env->rowList[i].start.y;
			getLocationOnDirection(&x, &y, env->rowList[i].direction, -1);
			p = EVAL_FACTOR_CAN_PUT_IN_ROW * env->rowList[i].length * 2;
			p += ((env->rowList[i].length == 3 && env->rowList[i].endType == 3) ? (EVAL_DANGER >> 1) : 0);
			p += ((env->rowList[i].length == 4) ? EVAL_DANGER : 0);
			evalMap[y * BOARD_SIZE + x] += p;
		}
		if((env->rowList[i].endType >> 1) & ENDTYPE_CAN_PUT){
			// can put after end point.
			x = env->rowList[i].start.x;
			y = env->rowList[i].start.y;
			getLocationOnDirection(&x, &y, env->rowList[i].direction, env->rowList[i].length);
			p = EVAL_FACTOR_CAN_PUT_IN_ROW * env->rowList[i].length * 2;
			p += ((env->rowList[i].length == 3 && env->rowList[i].endType == 3) ? (EVAL_DANGER >> 1) : 0);
			p += ((env->rowList[i].length == 4) ? EVAL_DANGER : 0);
			evalMap[y * BOARD_SIZE + x] += p;
		}
	}
	// 相手の列で間隔が一つのものを埋める戦略
	for(i = 0; i < ROW_LIST_LENGTH; i++){
		if(env->rowList[i].length <= 0){
			break;
		}
		if(env->rowList[i].col == env->currentColor){
			continue;
		}
		if(env->rowList[i].endType & ENDTYPE_CAN_PUT){
			// can put before start point.
			x = env->rowList[i].start.x;
			y = env->rowList[i].start.y;
			getLocationOnDirection(&x, &y, env->rowList[i].direction, -2);
			p = EVAL_FACTOR_CAN_PUT_IN_ROW * env->rowList[i].length * 3;
			if(env->mainBoard[y * BOARD_SIZE + x] != STATE_NONE && env->mainBoard[y * BOARD_SIZE + x] != env->currentColor){
				x = env->rowList[i].start.x;
				y = env->rowList[i].start.y;
				getLocationOnDirection(&x, &y, env->rowList[i].direction, -1);
				p += ((env->rowList[i].length >= 2) ? EVAL_DANGER : 0);
				evalMap[y * BOARD_SIZE + x] += p;
			}
		}
		if((env->rowList[i].endType >> 1) & ENDTYPE_CAN_PUT){
			// can put after end point.
			x = env->rowList[i].start.x;
			y = env->rowList[i].start.y;
			getLocationOnDirection(&x, &y, env->rowList[i].direction, env->rowList[i].length + 1);
			p = EVAL_FACTOR_CAN_PUT_IN_ROW * env->rowList[i].length * 3;
			if(env->mainBoard[y * BOARD_SIZE + x] != STATE_NONE && env->mainBoard[y * BOARD_SIZE + x] != env->currentColor){
				x = env->rowList[i].start.x;
				y = env->rowList[i].start.y;
				getLocationOnDirection(&x, &y, env->rowList[i].direction, env->rowList[i].length);
				p += ((env->rowList[i].length >= 2) ? EVAL_DANGER : 0);
				evalMap[y * BOARD_SIZE + x] += p;
			}
		}
	}
	// Debug out.
	for(i = 0; i < ROW_LIST_LENGTH; i++){
		if(env->rowList[i].length <= 0){
			break;
		}
		printf("%s row ", colorString[env->rowList[i].col]);
		//
		x = env->rowList[i].start.x;
		y = env->rowList[i].start.y;
		getLocationOnDirection(&x, &y, env->rowList[i].direction, -1);
		p = evalMap[y * BOARD_SIZE + x];
		printf("[%d]", p);
		//
		printf("%c%d, %d%c", ((env->rowList[i].endType & 1) ? '(' : '['), x, y, (((env->rowList[i].endType >> 1) & 1) ? ')' : ']'));
		x = env->rowList[i].start.x;
		y = env->rowList[i].start.y;
		getLocationOnDirection(&x, &y, env->rowList[i].direction, env->rowList[i].length);
		p = evalMap[y * BOARD_SIZE + x];
		printf("[%d]", p);
		//
		printf("%d\n", env->rowList[i].length);
	}
	// Choose best location to put.
	bestEval = EVAL_CANNOT_PUT;
	for(y = 0; y < BOARD_SIZE; y++){
		for(x = 0; x < BOARD_SIZE; x++){
			if(evalMap[y * BOARD_SIZE + x] > bestEval){
				bestEval = evalMap[y * BOARD_SIZE + x];
				to->x = x;
				to->y = y;
			}
		}
	}
}

int EasyAI_getRowPotential(StoneRow *row, char *board)
{
	// この列がどこまで伸びる可能性があるかを調べる。
	int p = row->length;
	int i, x, y;
	// before start
	x = row->start.x;
	y = row->start.y;
	for(i = 0; i < 4; i++){
		if(x == -1){
			break;
		}
		getLocationOnDirection(&x, &y, row->direction, -1);
		if(board[y * BOARD_SIZE + x] == STATE_NONE){
			p++;
		} else{
			break;
		}
	}
	// after end
	x = row->start.x;
	y = row->start.y;
	getLocationOnDirection(&x, &y, row->direction, row->length - 1);
	for(i = 0; i < 4; i++){
		getLocationOnDirection(&x, &y, row->direction, -1);
		if(x == -1){
			break;
		}
		if(board[y * BOARD_SIZE + x] == STATE_NONE || board[y * BOARD_SIZE + x] == row->col){
			p++;
		} else{
			break;
		}
	}
	return p;
}
