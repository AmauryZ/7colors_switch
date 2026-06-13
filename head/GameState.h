#ifndef GAME_STATE_H
#define GAME_STATE_H

typedef enum Color{
	ERROR = -1,
	EMPTY,
	PLAYER_1, //X
	PLAYER_2, //O
	RED, //A
	GREEN, //B
	BLUE, //C
	YELLOW, //D
	MAGENTA, //E
	CYAN, //F
	WHITE //G
}Color;  

typedef struct GameState{
	Color* map;
	int size;
} GameState;


void create_empty_game_state (GameState* state, int size);
void set_map_value (GameState* state, int x, int y, Color value);
Color get_map_value (GameState* state, int x, int y);
void fill_map(GameState* state);
int main(int argc, char** argv);

#endif