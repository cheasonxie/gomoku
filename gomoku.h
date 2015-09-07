#define BOARD_SIZE	15
#define ROW_LIST_LENGTH (BOARD_SIZE * BOARD_SIZE)

#define STATE_NONE	0
#define STATE_BLACK	1
#define STATE_WHITE	2

#define PLAYER_TYPE_COM_RANDOM	0
#define PLAYER_TYPE_HUMAN		1

#define DIRECTION_X		1
#define DIRECTION_Y		2
#define DIRECTION_LDOWN	4
#define DIRECTION_RDOWN 8

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

char *playerTypeString[] = {
	"COM(rand)",
	"Human"
};

typedef struct STONE_LOCATION StoneLocation;
struct STONE_LOCATION {
	int x, y;
};

typedef struct STONE_ROW StoneRow;
struct STONE_ROW {
	StoneLocation start;
	int direction;
	int length; // > 1.
};

typedef struct GAME_ENVIRONMENT GameEnvironment;
struct GAME_ENVIRONMENT {
	int playerBlackType;
	int playerWhiteType;
	char mainBoard[BOARD_SIZE*BOARD_SIZE];
	StoneRow rowList[ROW_LIST_LENGTH];
	int turnCount;	// even: black, odd: white
};

void inputStoneLocation(StoneLocation *loc);
void selectGameMode(GameEnvironment *env);
int scanIntegerRanged(int from, int to, char *message);
void initBoard(char *board);
void printBoard(char *board);
int putStone(char *board, int x, int y, int state);
void checkMapRows(char *board, StoneRow *rowList, int rowListLen);
void checkMapRowsSub(char *board, char *checkMap, int nx, int ny, int d, int col, int *count);
