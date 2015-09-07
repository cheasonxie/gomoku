#include <stdio.h>
#include "gomoku.h"

int main(int argc, char *argv[])
{
	int currentColor;
	int currentPlayerType;
	int i;
	GameEnvironment env;
	StoneLocation to;

	initBoard(env.mainBoard);
	printBoard(env.mainBoard);
	selectGameMode(&env);

	for(env.turnCount = 0; ; env.turnCount++){
		currentColor = (env.turnCount & 1) + 1;
		currentPlayerType = (currentColor == STATE_BLACK ? env.playerBlackType : env.playerWhiteType);
		printf("\n\nTurn%3d, %s, %s\n", env.turnCount, playerTypeString[currentPlayerType], colorString[currentColor]);

		// Think or Input.
		if(currentPlayerType == PLAYER_TYPE_COM_RANDOM){
			
		} else if(currentPlayerType == PLAYER_TYPE_HUMAN){
			printf("%s's turn.\n", colorString[currentColor]);
			inputStoneLocation(&to);	
		} else{
			puts("Invalid plyer type. Abort.");
			return 1;
		}
		printf("%s->(%d, %d)\n", colorString[currentColor], to.x, to.y);
		
		// Check and put stone.
		if(putStone(env.mainBoard, to.x, to.y, currentColor)){
			puts("################################################################");
			if(currentPlayerType == PLAYER_TYPE_HUMAN){
				puts("You can't put stone there. Try other location.");
				env.turnCount--;
			} else{
				puts("COM tried to put stone to illegal location. Abort.");
				return 1;
			}
			puts("################################################################");
			continue;
		}
		printBoard(env.mainBoard);

		// Goal test.
		checkMapRows(env.mainBoard, env.rowList, ROW_LIST_LENGTH);
		for(i = 0; i < ROW_LIST_LENGTH; i++){
			if(env.rowList[i].length == 5){
				break;
			}
		}
		if(i != ROW_LIST_LENGTH){
			puts("Game end.");
			printf("WINNER is %s !\n", colorString[currentColor]);
			break;
		}
	}

	return 0;
}

void inputStoneLocation(StoneLocation *loc)
{
	loc->x = scanIntegerRanged(0, BOARD_SIZE - 1, "X: ");
	loc->y = scanIntegerRanged(0, BOARD_SIZE - 1, "Y: ");
	return;
}

void selectGameMode(GameEnvironment *env)
{	
	puts("================================================================");
	puts("This is a little strong Gomoku program made by hikalium.");
	puts("================================================================");
	env->playerBlackType = scanIntegerRanged(0, 1, "Select black player type (0: COM(rand), 1: human): ");
	env->playerWhiteType = scanIntegerRanged(0, 1, "Select white player type (0: COM(rand), 1: human): ");
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
		printf("%3d", x);
	}
	putchar('\n');
	for(y = 0; y < BOARD_SIZE; y++){
		printf(" %3d ", y);
		for(x = 0; x < BOARD_SIZE; x++){
			printf(" %s ", stateString[board[y * BOARD_SIZE + x]]);
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
	if(board[y * BOARD_SIZE + x]){
		return 2;
	}
	board[y * BOARD_SIZE + x] = state;
	return 0;
}

void checkMapRows(char *board, StoneRow *rowList, int rowListLen)
{
	char checkMap[BOARD_SIZE*BOARD_SIZE];
	int x, y, count, col, d, rowIndex;

	// initialize.
	for(y = 0; y < BOARD_SIZE; y++){
		for(x = 0; x < BOARD_SIZE; x++){
			checkMap[y * BOARD_SIZE + x] = 0;
		}
	}
	for(rowIndex = 0; rowIndex < rowListLen; rowIndex++){
		rowList[rowIndex].length = 0;
	}
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
						printf("%s row (%d, %d) %d %d\n", colorString[col], x, y, d, count);
					}
					rowList[rowIndex].start.x = x;
					rowList[rowIndex].start.y = y; 
					rowList[rowIndex].direction = d;
					rowList[rowIndex].length = count;
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
	if(d == DIRECTION_X){
		nx++;
	} else if(d == DIRECTION_Y){
		ny++;
	} else if(d == DIRECTION_LDOWN){
		nx--;
		ny++;
	} else if(d == DIRECTION_RDOWN){
		nx++;
		ny++;
	}
	checkMapRowsSub(board, checkMap, nx, ny, d, col, count);
} 
