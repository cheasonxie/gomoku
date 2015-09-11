#include <stdio.h>
#include <stdlib.h>
#include "gomoku.h"

#define EVAL_CANNOT_PUT		(-0xffffff)
#define EVAL_NEUTRAL		(0)
#define EVAL_CHECKMATE		(0xffffff)
#define EVAL_DANGER			(0xfff)
#define EVAL_FACTOR_DIST_FROM_CENTER	5
#define EVAL_FACTOR_CAN_PUT_IN_ROW		5

#define SEARCH_DEPTH	4

#define EVAL_ROWLIST_LEN	1024

int NormalAI_alphaBetaSearch(char *baseBoard, int putColor, int depthCount, int *toX, int *toY);
int NormalAI_evaluateBoard(char *board, int maxColor);


void NormalAI_decideNextLocation(StoneLocation *to, GameEnvironment *env)
{
	NormalAI_alphaBetaSearch(env->mainBoard, env->currentColor, 0, &to->x, &to->y);
}

int NormalAI_alphaBetaSearch(char *baseBoard, int putColor, int depthCount, int *toX, int *toY)
{
	// ボード上全範囲の中で最大の評価値を返す。
	// x, yがNULLでない場合、その最大評価値が存在する座標を代入する。
	int candidateEval, tmpEval, sign;
	int colAfter = (putColor == STATE_BLACK ? STATE_WHITE : STATE_BLACK);
	int x, y;

	candidateEval = 0;

	if(depthCount >= SEARCH_DEPTH){
		return NormalAI_evaluateBoard(baseBoard, putColor);
	}
	for(y = 0; y < BOARD_SIZE; y++){
		for(x = 0; x < BOARD_SIZE; x++){
			if(putStone(baseBoard, x, y, putColor)){
				continue;
			}
			tmpEval = NormalAI_alphaBetaSearch(baseBoard, colAfter, depthCount + 1, NULL, NULL);
			//
			putStone(baseBoard, x, y, STATE_NONE);
			//
			if(tmpEval < candidateEval){
				candidateEval = tmpEval;
				if(toX != NULL && toY != NULL){
					*toX = x;
					*toY = y;
				}
			}
			
		}
	}
	//printf("eval: %d\n", candidateEval);
	return candidateEval;
}

int NormalAI_evaluateBoard(char *board, int maxColor)
{
	int x, y, p, i, sign, eval = 0, c;
	StoneRow rowList[EVAL_ROWLIST_LEN];
	
	for(y = 0; y < BOARD_SIZE; y++){
		for(x = 0; x < BOARD_SIZE; x++){
			c = board[y * BOARD_SIZE + x];
			if(c == STATE_NONE){
				continue;
			}
			if(c == maxColor){
				sign = 1;
			} else{
				sign = -1;
			}
			p = 0;
			p += (x > (BOARD_SIZE >> 1) ? BOARD_SIZE - 1 - x : x);
			p += (y > (BOARD_SIZE >> 1) ? BOARD_SIZE - 1 - y : y);
			p *= EVAL_FACTOR_DIST_FROM_CENTER;
			p *= sign;
			eval += p;
		}
	}
	
	checkMapRows(board, rowList, EVAL_ROWLIST_LEN);
	
	for(i = 0; i < EVAL_ROWLIST_LEN; i++){
		p = 0;
		if(rowList[i].length <= 0){
			break;
		}
		if(rowList[i].col == maxColor){
			sign = 1;
		} else{
			sign = -1;
		}
		if(rowList[i].length >= 5){
			p += 1000000;
		}
		if(rowList[i].length == 4 && rowList[i].endType != 0){
			p += 1000 * (rowList[i].endType == 3 ? 2 : 1);
		}
		if(rowList[i].length == 3 && rowList[i].endType != 0){
			p += 100 * (rowList[i].endType == 3 ? 2 : 1);
		}
		if(rowList[i].length == 2 && rowList[i].endType != 0){
			p += 10;
		}
		eval += sign * p;
	}
	
	return eval;
}