#include <iostream>
#include <fstream>
#include <ctime>
#include <chrono>
#include <stack>

using namespace std;

class CHIP8 {
	uint8_t memory[4096];
	uint8_t Vx[16];
	uint16_t pc = 0;
	uint16_t I = 0;

	// stack
	stack<uint16_t> stack;

	// delay timer
	uint8_t delay_timer = 0;

	// sound timer
	uint8_t sound_timer = 0;

	clock_t prev_time;
	unsigned long time_increment = 0;

	void addCharacter(uint8_t c, char buf[]);
	void clearDisplayBuffer();

	// keyboard input
	bool keys[16];

public:
	uint8_t displayBuffer[64*32];

	CHIP8();
	void load(uint8_t *mem, int size);
	bool cycle();
	void keyPressed(char key);
	void keyUp(char key);
};
