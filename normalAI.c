#include <stdio.h>
#include <stdlib.h>
#include "gomoku.h"

#define EVAL_CANNOT_PUT		(-0xffffff)
#define EVAL_NEUTRAL		(0)
#define EVAL_CHECKMATE		(0xffffff)
#define EVAL_DANGER			(0xfff)
#define EVAL_FACTOR_DIST_FROM_CENTER	5
#define EVAL_FACTOR_CAN_PUT_IN_ROW		5

#define EVAL_INFINITE	0xffffff

#define SEARCH_DEPTH	4

#define EVAL_ROWLIST_LEN	1024

int NormalAI_alphaBetaSearch(char *baseBoard, int putColor, int depthCount, int *toX, int *toY, int rootCandidateEval);
int NormalAI_evaluateBoard(char *board);
void NormalAI_createCheckMap(char *board, char *map);
int NormalAI_getRowPotential(StoneRow *row, char *board);


void NormalAI_decideNextLocation(StoneLocation *to, GameEnvironment *env)
{
	int i, x, y, k;
	//
	// もし自分が4つ並べていたらチェックメイト
	//
	for(i = 0; i < ROW_LIST_LENGTH; i++){
		if(env->rowList[i].length <= 0){
			break;
		}
		if(env->rowList[i].col != env->currentColor){
			continue;
		}
		if(env->rowList[i].length != 4){
			continue;
		}
		if(env->rowList[i].endType & ENDTYPE_CAN_PUT){
			// can put before start point.
			x = env->rowList[i].start.x;
			y = env->rowList[i].start.y;
			getLocationOnDirection(&x, &y, env->rowList[i].direction, -1);
			to->x = x;
			to->y = y;
			break;
		}
		if((env->rowList[i].endType >> 1) & ENDTYPE_CAN_PUT){
			// can put after end point.
			x = env->rowList[i].start.x;
			y = env->rowList[i].start.y;
			getLocationOnDirection(&x, &y, env->rowList[i].direction, env->rowList[i].length);
			to->x = x;
			to->y = y;
			break;
		}
	}
	if(i != ROW_LIST_LENGTH && env->rowList[i].length > 0){
		puts("Checkmate!");
		return;
	}
	//
	// オープン3もしくはハーフクローズ4を阻止
	//
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
		if(NormalAI_getRowPotential(&env->rowList[i], env->mainBoard) < 5){
			continue;
		}
		if(env->rowList[i].endType & ENDTYPE_CAN_PUT){
			// can put before start point.
			if(env->rowList[i].length == 4 || (env->rowList[i].length == 3 && env->rowList[i].endType == 3)){
				x = env->rowList[i].start.x;
				y = env->rowList[i].start.y;
				getLocationOnDirection(&x, &y, env->rowList[i].direction, -1);
				to->x = x;
				to->y = y;
				break;
			}
		}
		if((env->rowList[i].endType >> 1) & ENDTYPE_CAN_PUT){
			// can put after end point.
			if(env->rowList[i].length == 4 || (env->rowList[i].length == 3 && env->rowList[i].endType == 3)){
				x = env->rowList[i].start.x;
				y = env->rowList[i].start.y;
				getLocationOnDirection(&x, &y, env->rowList[i].direction, env->rowList[i].length);
				to->x = x;
				to->y = y;
				break;
			}
		}
	}
	if(i != ROW_LIST_LENGTH && env->rowList[i].length > 0){
		printf("Open %d detected.\n", env->rowList[i].length);
		return;
	}
	//
	// 間が一個空いているだけの場所を埋める
	//
	for(k = 3; k > 1; k--){
		for(i = 0; i < ROW_LIST_LENGTH; i++){
			if(env->rowList[i].length <= 0){
				break;
			}
			if(env->rowList[i].col == env->currentColor){
				continue;
			}
			if(NormalAI_getRowPotential(&env->rowList[i], env->mainBoard) < 5){
				continue;
			}
			if(env->rowList[i].length != k){
				continue;
			}
			if(env->rowList[i].endType & ENDTYPE_CAN_PUT){
				// can put before start point.
				x = env->rowList[i].start.x;
				y = env->rowList[i].start.y;
				getLocationOnDirection(&x, &y, env->rowList[i].direction, -2);
				if(x == -1){
					continue;
				}
				if(env->mainBoard[y * BOARD_SIZE + x] != STATE_NONE && env->mainBoard[y * BOARD_SIZE + x] != env->currentColor){
					x = env->rowList[i].start.x;
					y = env->rowList[i].start.y;
					getLocationOnDirection(&x, &y, env->rowList[i].direction, -1);
					to->x = x;
					to->y = y;
					break;
				}
			}
			if((env->rowList[i].endType >> 1) & ENDTYPE_CAN_PUT){
				// can put after end point.
				x = env->rowList[i].start.x;
				y = env->rowList[i].start.y;
				getLocationOnDirection(&x, &y, env->rowList[i].direction, env->rowList[i].length + 1);
				if(x == -1){
					continue;
				}
				if(env->mainBoard[y * BOARD_SIZE + x] != STATE_NONE && env->mainBoard[y * BOARD_SIZE + x] != env->currentColor){
					x = env->rowList[i].start.x;
					y = env->rowList[i].start.y;
					getLocationOnDirection(&x, &y, env->rowList[i].direction, env->rowList[i].length);
					to->x = x;
					to->y = y;
					break;
				}
			}
		}
		if(i != ROW_LIST_LENGTH && env->rowList[i].length > 0){
			if(i != ROW_LIST_LENGTH && env->rowList[i].length > 0){
				puts("Block putting in row.");
				return;
			}
			return;
		}
	}
	NormalAI_alphaBetaSearch(env->mainBoard, env->currentColor, 0, &to->x, &to->y, ((env->currentColor == STATE_BLACK) ? EVAL_INFINITE : -EVAL_INFINITE));

}

int NormalAI_alphaBetaSearch(char *baseBoard, int putColor, int depthCount, int *toX, int *toY, int rootCandidateEval)
{
	// ボード上全範囲の中で最大の評価値を返す。
	// x, yがNULLでない場合、その最大評価値が存在する座標を代入する。
	int candidateEval, tmpEval;
	int colAfter = (putColor == STATE_BLACK ? STATE_WHITE : STATE_BLACK);
	int x, y;
	char checkMap[BOARD_SIZE * BOARD_SIZE];

	if(depthCount >= SEARCH_DEPTH){
		return NormalAI_evaluateBoard(baseBoard);
	}
	
	if(putColor == STATE_BLACK){
		candidateEval = -EVAL_INFINITE;
	} else{
		candidateEval = EVAL_INFINITE;
	}
	
	NormalAI_createCheckMap(baseBoard, checkMap);
	
	for(y = 0; y < BOARD_SIZE; y++){
		for(x = 0; x < BOARD_SIZE; x++){
			if(checkMap[y * BOARD_SIZE + x] != STATE_BLACK){
				continue;
			}
			if(putStone(baseBoard, x, y, putColor)){
				continue;
			}
			tmpEval = NormalAI_alphaBetaSearch(baseBoard, colAfter, depthCount + 1, NULL, NULL, candidateEval);
			//
			putStone(baseBoard, x, y, STATE_NONE);
			//
			if(putColor == STATE_BLACK){
				if(tmpEval > candidateEval){
					candidateEval = tmpEval;
					if(toX != NULL && toY != NULL){
						*toX = x;
						*toY = y;
						printf("(%2d, %2d) eval: %d\n", x, y, candidateEval);
					}
					if(candidateEval > rootCandidateEval){
						break;
					}
				}
			} else{
				if(tmpEval < candidateEval){
					candidateEval = tmpEval;
					if(toX != NULL && toY != NULL){
						*toX = x;
						*toY = y;
						printf("(%2d, %2d) eval: %d\n", x, y, candidateEval);
					}
					if(candidateEval < rootCandidateEval){
						break;
					}
				}
			}
			
		}
		if(x < BOARD_SIZE){
			break;
		}
	}
	return candidateEval;
}

void NormalAI_createCheckMap(char *board, char *map)
{
	int x, y;
	
	for(y = 0; y < BOARD_SIZE; y++){
		for(x = 0; x < BOARD_SIZE; x++){
			map[y * BOARD_SIZE + x] = STATE_NONE;
		}
	}
	
	for(y = 0; y < BOARD_SIZE; y++){
		for(x = 0; x < BOARD_SIZE; x++){
			if(board[y * BOARD_SIZE + x] == STATE_NONE){
				continue;
			}
			putStone(map, x, y, STATE_NONE);
			putStone(map, x, y, STATE_WHITE);
			putStone(map, x + 1, y, STATE_BLACK);
			putStone(map, x, y + 1, STATE_BLACK);
			putStone(map, x + 1, y + 1, STATE_BLACK);
			putStone(map, x - 1, y, STATE_BLACK);
			putStone(map, x, y - 1, STATE_BLACK);
			putStone(map, x - 1, y - 1, STATE_BLACK);
			putStone(map, x + 1, y - 1, STATE_BLACK);
			putStone(map, x - 1, y + 1, STATE_BLACK);
		}
	}
	putStone(map, 7, 7, STATE_BLACK);
}

int NormalAI_evaluateBoard(char *board)
{
	// 黒に有利な場面ほど正に大きな数字を出す。
	int x, y, p, i, sign, eval = 0, c;
	StoneRow rowList[EVAL_ROWLIST_LEN];
	
	for(y = 0; y < BOARD_SIZE; y++){
		for(x = 0; x < BOARD_SIZE; x++){
			c = board[y * BOARD_SIZE + x];
			if(c == STATE_NONE){
				continue;
			}
			if(c == STATE_BLACK){
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
		if(rowList[i].col == STATE_BLACK){
			sign = 1;
		} else{
			sign = -1;
		}
		if(rowList[i].length >= 5){
			p += 1000000;
		}
		if(rowList[i].length == 4 && rowList[i].endType != 0){
			p += 10000 * (rowList[i].endType == 3 ? 2 : 1);
		}
		if(rowList[i].length == 3 && rowList[i].endType != 0){
			p += 100 * (rowList[i].endType == 3 ? 2 : 1);
		}
		if(rowList[i].length == 2 && rowList[i].endType != 0){
			p += 10 * (rowList[i].endType == 3 ? 2 : 1);
		}
		eval += sign * p;
		//
		if(rowList[i].length <= 0){
			break;
		}
		if(rowList[i].col == STATE_BLACK){
			sign = 1;
		} else{
			sign = -1;
		}
		p = NormalAI_getRowPotential(&rowList[i], board);
		p &= 7;
		p *= sign;
		eval += sign * p;
	}
	
	return eval;
}

int NormalAI_getRowPotential(StoneRow *row, char *board)
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
/*
int NormalAI_createRowPotentialMapSub(StoneRow *row, char *board, int *pMap)
{
	// この列がどこまで伸びる可能性があるかを調べる。
	int p = row->length;
	int i, x, y;
	//
	for(y = 0; y < BOARD_SIZE; y++){
		for(x = 0; x < BOARD_SIZE; x++){
			
		}
	}
	// before start
	x = row->start.x;
	y = row->start.y;
	for(i = 0; i < 5-p; i++){
		if(x == -1){
			break;
		}
		getLocationOnDirection(&x, &y, row->direction, -1);
		if(board[y * BOARD_SIZE + x] == STATE_NONE){
			
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
*/
