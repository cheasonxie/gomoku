#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "gomoku.h"

char *stateString[] = {
	".",
	"●",
	"○"
};

char *colorString[] = {
	"None",
	"Black",
	"White"
};

void (*AIList[AI_LIST_LENGTH])(StoneLocation *to, GameEnvironment *env) = {
	RandAI_decideNextLocation,
	EasyAI_decideNextLocation,
	NormalAI_decideNextLocation,
	ReijerAI_decideNextLocation
};
char *playerTypeString[AI_LIST_LENGTH + 1] = {
	"Human",
	"RandAI",
	"EasyAI",
	"NormalAI",
	"ReijerAI"
};

int main()
{
	GameEnvironment env;
	StoneLocation to;
	// 内部的には座標を常にゼロオリジンで扱う。

	initBoard(env.mainBoard);
	printBoard(env.mainBoard);
	selectGameMode(&env);

	srand(time(NULL));

	for(env.gameCount = 0; env.gameCount < env.gameCountLimit; env.gameCount++){
		for(env.turnCount = 0; ; env.turnCount++){
			env.currentColor = (env.turnCount & 1) + 1;
			env.currentPlayerType = (env.currentColor == STATE_BLACK ? env.playerBlackType : env.playerWhiteType);
			printf("\n\n%3d-%3d, %s, %s\n", env.gameCount, env.turnCount, playerTypeString[env.currentPlayerType], colorString[env.currentColor]);
	
			// Think or Input.
			if(env.currentPlayerType == PLAYER_TYPE_HUMAN){
				printf("%s's turn.\n", colorString[env.currentColor]);
				if(inputStoneLocation(&to)){
					// revert to last turn.
					if((env.currentColor == STATE_BLACK ? env.playerWhiteType : env.playerBlackType) != PLAYER_TYPE_HUMAN){
						// back twice.
						revertTurn(&env, 2);
					} else{
						// back once.
						revertTurn(&env, 1);
					}
					printBoard(env.mainBoard);
					continue;
				}
			} else if(env.currentPlayerType <= AI_LIST_LENGTH){
				AIList[env.currentPlayerType - 1](&to, &env);
			} else{
				puts("Invalid player type. Abort.");
				return 1;
			}
			printf("%s->(%d, %d)\n", colorString[env.currentColor], to.x + DISPLAY_NUM_BASE, to.y + DISPLAY_NUM_BASE);
			
			// Check and put stone.
			if(putStone(env.mainBoard, to.x, to.y, env.currentColor)){
				puts("################################################################");
				if(env.currentPlayerType == PLAYER_TYPE_HUMAN){
					puts("You can't put stone there. Try other location.");
					env.turnCount--;
				} else{
					puts("COM tried to put stone to illegal location. Abort.");
					return 1;
				}
				puts("################################################################");
				continue;
			}
			env.history[env.turnCount] = to;
			printBoard(env.mainBoard);
			if(isGameEnd(&env)){
				break;
			}
		}
		printf("BlackWins: %4d/%4d    (%s)\n", env.playerBlackWins, env.gameCount + 1, playerTypeString[env.playerBlackType]);
		printf("WhiteWins: %4d/%4d    (%s)\n", env.playerWhiteWins, env.gameCount + 1, playerTypeString[env.playerWhiteType]);
		printf("     Draw: %4d/%4d\n", env.gameCount - env.playerBlackWins - env.playerWhiteWins + 1, env.gameCount + 1);
		initBoard(env.mainBoard);
		env.rowList[0].length = 0;
	}

	return 0;
}

int inputStoneLocation(StoneLocation *loc)
{
	puts("Type next location (-1 means revert to last turn).");
	loc->x = scanIntegerRanged(-1, BOARD_SIZE - 1 + DISPLAY_NUM_BASE, "X: ");
	if(loc->x <= -1 + DISPLAY_NUM_BASE){
		return 1;
	}
	loc->y = scanIntegerRanged(-1, BOARD_SIZE - 1 + DISPLAY_NUM_BASE, "Y: ");
	if(loc->y <= -1 + DISPLAY_NUM_BASE){
		return 1;
	}
	loc->x -= DISPLAY_NUM_BASE;
	loc->y -= DISPLAY_NUM_BASE;
	return 0;
}

void selectGameMode(GameEnvironment *env)
{	
	int i;
	
	env->playerBlackWins = 0;
	env->playerWhiteWins = 0;
	puts("================================================================");
	puts("This is a little strong Gomoku program made by hikalium.");
	puts("================================================================");
	puts("Player type list.");
	for(i = 0; i < AI_LIST_LENGTH + 1; i++){
		printf("%d: %s\n", i, playerTypeString[i]);
	}
	env->playerBlackType = scanIntegerRanged(0, AI_LIST_LENGTH, "Select black player type: ");
	env->playerWhiteType = scanIntegerRanged(0, AI_LIST_LENGTH, "Select white player type: ");
	if(env->playerBlackType != PLAYER_TYPE_HUMAN && env->playerWhiteType != PLAYER_TYPE_HUMAN){
		env->gameCountLimit = scanIntegerRanged(1, 65535, "Input number of games (1-65535): ");
	} else{
		env->gameCountLimit = 1;
	}
	return;
}

int scanIntegerRanged(int from, int to, char *message)
{	
	int tmp;
	for(;;){
		printf("%s", message);
		if(scanf("%d", &tmp) == 1){
			if(from <= tmp && tmp <= to){
				break;
			}
			puts("Out of range. Input again.");
		} else{
			scanf("%*s");
		}
	}
	return tmp;
}

void revertTurn(GameEnvironment *env, int count)
{
	int i;
	
	if(env->turnCount < count){
		puts("################################################################");
		puts("You can't back before starting this game.");
		env->turnCount--;
		return;
	}
	for(i = 0; i < count; i++){
		putStone(env->mainBoard, env->history[env->turnCount - 1 - i].x, env->history[env->turnCount - 1 - i].y, STATE_NONE);
	}
	env->turnCount -= (count + 1);
	printf("Back to turn %d.", env->turnCount + 1);
}

void initBoard(char *board)
{
	int x, y;

	for(y = 0; y < BOARD_SIZE; y++){
		for(x = 0; x < BOARD_SIZE; x++){
			board[y * BOARD_SIZE + x] = STATE_NONE;
		}
	}
	return;
}

void printBoard(char *board)
{
	int x, y;
	printf("    ");
	for(x = 0; x < BOARD_SIZE; x++){
		printf("%3d", x + DISPLAY_NUM_BASE);
	}
	putchar('\n');
	for(y = 0; y < BOARD_SIZE; y++){
		printf(" %3d ", y + DISPLAY_NUM_BASE);
		for(x = 0; x < BOARD_SIZE; x++){
			printf(" %s ", stateString[(int)board[y * BOARD_SIZE + x]]);
		}
		putchar('\n');
	}
	return;
}

int putStone(char *board, int x, int y, int state)
{
	// 0: success
	// 1: out of range
	// 2: already exist.
	if(x < 0 || BOARD_SIZE <= x || y < 0 || BOARD_SIZE <= y){
		return 1;
	}
	if(board[y * BOARD_SIZE + x] && state != STATE_NONE){
		return 2;
	}
	board[y * BOARD_SIZE + x] = state;
	return 0;
}

void checkMapRows(char *board, StoneRow *rowList, int rowListLen)
{
	char checkMap[BOARD_SIZE*BOARD_SIZE];
	int x, y, count, col, d, rowIndex, endType;

	// initialize.
	for(y = 0; y < BOARD_SIZE; y++){
		for(x = 0; x < BOARD_SIZE; x++){
			checkMap[y * BOARD_SIZE + x] = 0;
		}
	}
	rowList[0].length = 0;
	rowIndex = 0;
	for(col = 1; col <= 2; col++){
		for(d = 1; d <= 8; d <<= 1){
			for(y = 0; y < BOARD_SIZE; y++){
				for(x = 0; x < BOARD_SIZE; x++){
					if(board[y * BOARD_SIZE + x] == STATE_NONE){
						continue;
					}
					if(checkMap[y * BOARD_SIZE + x] & d){
						continue;
					}
					count = 0;
					checkMapRowsSub(board, checkMap, x, y, d, col, &count);
					if(count > 1){
						endType = checkMapRows_getEndType(board, x, y, d, count);
						//printf("%s row %c%d, %d%c %d %d %d\n", colorString[col], ((endType & 1) ? '(' : '['), x, y, (((endType >> 1) & 1) ? ')' : ']'), d, count, endType);
						rowList[rowIndex].col = col;
						rowList[rowIndex].start.x = x;
						rowList[rowIndex].start.y = y; 
						rowList[rowIndex].direction = d;
						rowList[rowIndex].length = count;
						rowList[rowIndex].endType = endType;
						rowIndex++;
						if(rowIndex >= rowListLen){
							puts("RowList overflow.");
							return;
						}
					}
				}
			}
		}
	}
	rowList[rowIndex].length = 0;
}

void checkMapRowsSub(char *board, char *checkMap, int nx, int ny, int d, int col, int *count)
{
	if(nx < 0 || BOARD_SIZE <= nx || ny < 0 || BOARD_SIZE <= ny){
		return;
	}
	if(board[ny * BOARD_SIZE + nx] != col){
		return;
	}
	(*count)++;
	checkMap[ny * BOARD_SIZE + nx] |= d;
	getLocationOnDirection(&nx, &ny, d, 1);
	checkMapRowsSub(board, checkMap, nx, ny, d, col, count);
}

int checkMapRows_getEndType(char *board, int x, int y, int d, int count)
{
	int endType = 0;
	int px, py;
	
	// start
	px = x;
	py = y;
	getLocationOnDirection(&px, &py, d, -1);
	if(px != -1 && board[py * BOARD_SIZE + px] == STATE_NONE){
		endType |= ENDTYPE_CAN_PUT;
	}
	
	// end
	px = x;
	py = y;
	getLocationOnDirection(&px, &py, d, count);
	if(px != -1 && board[py * BOARD_SIZE + px] == STATE_NONE){
		endType |= (ENDTYPE_CAN_PUT << 1);
	}
	return endType;
}

void getLocationOnDirection(int *x, int *y, int d, int count)
{
	// (*x, *y) is set (-1, -1) when location after moving is out of bound.
	int dx = 0, dy = 0;
	if(d == DIRECTION_X){
		dx++;
	} else if(d == DIRECTION_Y){
		dy++;
	} else if(d == DIRECTION_LDOWN){
		dx--;
		dy++;
	} else if(d == DIRECTION_RDOWN){
		dx++;
		dy++;
	}
	*x += dx * count;
	*y += dy * count;
	if(*x < 0 || BOARD_SIZE <= *x || *y < 0 || BOARD_SIZE <= *y){
		*x = -1;
		*y = -1;
	}
}

int isGameEnd(GameEnvironment *env)
{
	int i, x, y;

	checkMapRows(env->mainBoard, env->rowList, ROW_LIST_LENGTH);
	for(i = 0; i < ROW_LIST_LENGTH; i++){
		if(env->rowList[i].length == 0){
			break;
		}
		if(env->rowList[i].length >= 5){
			break;
		}
	}
	if(i != ROW_LIST_LENGTH && env->rowList[i].length >= 5){
		puts("Game end.");
		printf("WINNER is %s !\n", colorString[env->currentColor]);
		if(env->currentColor == STATE_BLACK){
			env->playerBlackWins++;
		} else{
			env->playerWhiteWins++;
		}
		return 1;
	}
	for(y = 0; y < BOARD_SIZE; y++){
		for(x = 0; x < BOARD_SIZE; x++){
			if(env->mainBoard[y * BOARD_SIZE + x] == STATE_NONE){
				break;
			}
		}
		if(x != BOARD_SIZE){
			break;
		}
	}
	if(y == BOARD_SIZE){
		puts("Game end.");
		printf("Game ended in draw... \n");
		return 1;
	}

	return 0;
}
