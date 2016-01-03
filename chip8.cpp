#include "chip8.h"

CHIP8::CHIP8() {
	this->prev_time = chrono::system_clock::now().time_since_epoch().count();
}

void CHIP8::addCharacter(uint8_t c, char buf[]) {
	for(int i = 0; i < 6; i++) {
		this->memory[c*6 + i] = buf[i];
	}
}

void CHIP8::clearDisplayBuffer() {
	// clear display buffer
	for (int i = 0; i < 64*32; i++) {
		this->displayBuffer[i] = 0;
	}
}

void CHIP8::keyPressed(char key) {
	uint8_t k = 0x00;
	if (key >= 65 && key <= 70) {
		k = key - 55;
	}else if (key >= 97 && key <= 102) {
		k = key - 87;
	}else if (key >= 48 && key <= 57) {
		k = key - 48;
	}else{
		cout << "unknown key pressed" << endl;
		return;
	}
	
	this->keys[k] = true;
}

void CHIP8::keyUp(char key) {
	uint8_t k = 0x00;
	if (key >= 65 && key <= 70) {
		k = key - 55;
	}else if (key >= 97 && key <= 102) {
		k = key - 87;
	}else if (key >= 48 && key <= 57) {
		k = key - 48;
	}else{
		cout << "unknown key up-ed" << endl;
		return;
	}

	this->keys[k] = false;
}

void CHIP8::load(uint8_t *mem, int size) {
	// seed random
	srand(time(NULL));

	// CHIP character sprites
	this->addCharacter(0x0, (char [6]){0xF0, 0x90, 0x90, 0x90, 0xF0});
	this->addCharacter(0x1, (char [6]){0x20, 0x60, 0x20, 0x20, 0x70});
	this->addCharacter(0x2, (char [6]){0xF0, 0x10, 0xF0, 0x80, 0xF0});
	this->addCharacter(0x3, (char [6]){0xF0, 0x10, 0xF0, 0x10, 0xF0});
	this->addCharacter(0x4, (char [6]){0x90, 0x90, 0xF0, 0x10, 0x10});
	this->addCharacter(0x5, (char [6]){0xF0, 0x80, 0xF0, 0x10, 0xF0});
	this->addCharacter(0x6, (char [6]){0xF0, 0x80, 0x90, 0x90, 0xF0});
	this->addCharacter(0x7, (char [6]){0xF0, 0x10, 0x20, 0x40, 0x40});
	this->addCharacter(0x8, (char [6]){0xF0, 0x90, 0xF0, 0x90, 0xF0});
	this->addCharacter(0x9, (char [6]){0xF0, 0x90, 0xF0, 0x10, 0xF0});
	this->addCharacter(0xA, (char [6]){0xF0, 0x90, 0xF0, 0x90, 0x90});
	this->addCharacter(0xB, (char [6]){0xE0, 0x90, 0xE0, 0x90, 0xE0});
	this->addCharacter(0xC, (char [6]){0xF0, 0x80, 0x80, 0x80, 0xF0});
	this->addCharacter(0xD, (char [6]){0xE0, 0x90, 0x90, 0x90, 0xE0});
	this->addCharacter(0xE, (char [6]){0xF0, 0x80, 0xF0, 0x80, 0xF0});
	this->addCharacter(0xF, (char [6]){0xF0, 0x80, 0xF0, 0x80, 0x80});

	// load ROM
	this->pc = 0x200;

	for(int i = 0; i < size; i++) {
		this->memory[0x200 + i] = mem[i];
	}

	this->clearDisplayBuffer();
}

bool CHIP8::cycle() {
	// handle timers
	int diff = (chrono::system_clock::now().time_since_epoch().count() - this->prev_time) / 1000;
	this->time_increment += diff;

	while(this->time_increment >= 16) { //roughly 60Hz
		this->time_increment -= 16;

		if (this->delay_timer > 0) {
			this->delay_timer -= 1;
		}

		if (this->sound_timer > 0) {
			//TODO emit sound
			cout << "should emit sound!" << endl;

			this->sound_timer -= 1;
		}
	}

	this->prev_time = chrono::system_clock::now().time_since_epoch().count();

	// handle execution
	uint16_t opcode = (this->memory[this->pc] << 8) | this->memory[this->pc+1];
	uint16_t nnn = (opcode & 0xFFF);
	uint8_t n = (opcode & 0xF); // lower 4-bit only
	uint8_t x = ((opcode >> 8) & 0xF); // lower 4-bit only
	uint8_t y = ((opcode >> 4) & 0xF); // lower 4-bit only
	uint8_t kk = (opcode & 0xFF);

	uint8_t prefix = (opcode >> 12) & 0xF;

	// local variables
	uint8_t a, b, c;

	// increment PC
	this->pc += 2;

	if (prefix == 0) {
		if (opcode == 0xE0) { // 0x00E0
			this->clearDisplayBuffer();
		}else if (opcode == 0xEE) { // 0x00EE
			if (!this->stack.empty()) {
				this->pc = this->stack.top();
				this->stack.pop();
			}else{
				cout << "cannot pop from stack; stack is empty" << endl;
				return false;
			}
		}else{// 0x0nnn
			this->pc = nnn;
		}
	}else if (prefix == 1) { // 0x1nnn
		this->pc = nnn;
	}else if (prefix == 2) { // 0x2nnn
		this->stack.push(this->pc);
		this->pc = nnn;
	}else if (prefix == 3) { // 0x3xkk
		if (this->Vx[x] == kk) { // skip next instruction if match
			this->pc += 2;
			return true;
		}
	}else if (prefix == 4) { // 0x4xkk
		if (this->Vx[x] != kk) { // skip next instruction if dont match
			this->pc += 2;
			return true;
		}
	}else if (prefix == 5) { // 0x5xy0
		if (this->Vx[x] == this->Vx[y]) {
			this->pc += 2;
			return true;
		}
	}else if (prefix == 6) { // 0x6xkk
		this->Vx[x] = kk;
	}else if (prefix == 7) { // 0x7xkk
		this->Vx[x] = this->Vx[x] + kk;
	}else if(prefix == 8) {
		if (n == 0) { // 0x8xy0
			this->Vx[x] = this->Vx[y];
		}else if (n == 1) { // 0x8xy1
			this->Vx[x] = this->Vx[x] | this->Vx[y];
		}else if (n == 2) { // 0x8xy2
			this->Vx[x] = this->Vx[x] & this->Vx[y];
		}else if (n == 3) { // 0x8xy3
			this->Vx[x] = this->Vx[x] ^ this->Vx[y];
		}else if (n == 4) { // 0x8xy4
			int r = this->Vx[x] + this->Vx[y];
			if (r > 0xFF) {
				this->Vx[0xF] = 1;
			}else{
				this->Vx[0xF] = 0;
			}

			this->Vx[x] = (r & 0xFF);
		}else if (n == 5) { // 0x8xy5
			this->Vx[0xF] = (Vx[x] > Vx[y]);
			this->Vx[x] = this->Vx[x] - this->Vx[y];
			// TOOD: what happens if result is negative?
		}else if (n == 6) { // 0x8xy6
			this->Vx[0xF] = (this->Vx[x] & 0x1);
			this->Vx[x] /= 2;
		}else if (n == 7) { // 0x8xy7
			this->Vx[0xF] = (Vx[y] > Vx[x]);
			this->Vx[x] = this->Vx[y] - this->Vx[x];
			// TOOD: what happens if result is negative?
		}else if (n == 0xE) { // 0x8xyE
			this->Vx[0xF] = ((this->Vx[x] >> 7) & 0x1);
			this->Vx[x] *= 2;
		}else{
			goto unknown;
		}
	}else if (prefix == 9) { // 0x9xy0
		if (this->Vx[x] != this->Vx[y]) {
			this->pc += 2;
			return true;
		}
	}else if (prefix == 0xA) { // 0xAnnn
		this->I = nnn;
	}else if (prefix == 0xB) { //0xBnnn
		this->pc = nnn + this->Vx[0];
	}else if (prefix == 0xC) { // 0xCxkk
		uint8_t rnd = rand() % 256;
		this->Vx[x] = kk & rnd;
	}else if (prefix == 0xD) { // 0xDxyn
		int dX = this->Vx[x];
		int dY = this->Vx[y];

		for(int i = 0; i < n; i++) {
			for (int j = 0;j < 8;j++) {
				this->displayBuffer[(dY + i) * 64 + dX + j] ^= (this->memory[I+i] >> (7-j)) & 0x1;
			}
		}
		// TODO:
		// modify VF register
		// take care of offscreen rendering
	}else if (prefix == 0xE) {
		if (kk == 0x9E) { // 0xEx9E
			// check if key is pressed
			if (this->keys[this->Vx[x]]) {
				this->pc += 2;
			}
		}else if (kk == 0xA1) { // 0xExA1
			// check if key is NOT pressed
			if (!this->keys[this->Vx[x]]) {
				this->pc += 2;
			}
		}else{
			goto unknown;
		}
	}else if (prefix == 0xF) {
		switch (kk) {
		case 0x7: // 0xFx07
			this->Vx[x] = this->delay_timer;
			break;
		//case 0xA: // 0xFx0A
		//TODO: implement
		case 0x15: // 0xFx15
			this->delay_timer = this->Vx[x];
			break;
		case 0x18: // 0xFx18
			this->sound_timer = this->Vx[x];
			break;
		case 0x1e: // 0xFx1E
			this->I = this->I + this->Vx[x];
			break;
		case 0x29: // 0xFx29
			this->I = this->memory[this->Vx[x]*5];
			break;
		case 0x33: // 0xFx33
			a = this->Vx[x] / 100;
			this->memory[this->I] = this->memory[a*5];
			b = (this->Vx[x] / 10) - (this->Vx[x]/100*10);
			this->memory[this->I + 1] = this->memory[b*5];
			c = this->Vx[x] - (a*100) - (b*10);
			this->memory[this->I + 2] = this->memory[c*5];
		case 0x55: // 0xFx55
			for(int i = 0; i <= x; i++) {
				this->memory[this->I+i] = this->Vx[i];
			}
			break;
		case 0x65: // 0xFx65
			for(int i = 0; i <= x; i++) {
				this->Vx[i] = this->memory[this->I+i];
			}
			break;
		default:
			goto unknown;
		}
	}else{
		goto unknown;
	}

	return true;

unknown:
	// unknown opcode
	printf("opcode: %x\n", opcode);
	printf("unknown opcode\n");
	return false;
}
