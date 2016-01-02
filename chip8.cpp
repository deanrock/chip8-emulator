#include "chip8.h"

using namespace std;

CHIP8::CHIP8() {
	this->prev_time = chrono::system_clock::now().time_since_epoch().count();
}

void CHIP8::load(uint8_t *mem, int size) {
	// seed random
	srand(time(NULL));

	this->pc = 0x200;

	for(int i = 0; i < size; i++) {
		this->memory[0x200 + i] = mem[i];
	}

	// clear display buffer
	for (int i = 0; i < 64*32; i++) {
		this->displayBuffer[i] = 0;
	}
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

	// increment PC
	this->pc += 2;

	if (prefix == 1) {
		this->pc = nnn;
	}else if (prefix == 3) {
		if (this->Vx[x] == kk) { // skip next instruction if match
			this->pc += 2;
			return true;
		}
	}else if (prefix == 4) {
		if (this->Vx[x] != kk) { // skip next instruction if dont match
			this->pc += 2;
			return true;
		}
	}else if (prefix == 5) {
		if (this->Vx[x] == this->Vx[y]) {
			this->pc += 2;
			return true;
		}
	}else if (prefix == 6) {
		this->Vx[x] = kk;
	}else if (prefix == 7) {
		this->Vx[x] = this->Vx[x] + kk;
	}else if(prefix == 8) {
		if (n == 0) {
			this->Vx[x] = this->Vx[y];
		}else if (n == 4) {
			int r = this->Vx[x] + this->Vx[y];
			if (r > 0xFF) {
				this->Vx[0xF] = 1;
			}else{
				this->Vx[0xF] = 0;
			}

			this->Vx[x] = (r & 0xFF);
		}else if (n == 5) {
			this->Vx[0xF] = (Vx[x] > Vx[y]);
			this->Vx[x] = this->Vx[x] - this->Vx[y];
			// TOOD: what happens if result is negative?
		}else{
			goto unknown;
		}
	}else if (prefix == 9) {
		if (this->Vx[x] != this->Vx[y]) {
			this->pc += 2;
			return true;
		}
	}else if (prefix == 0xA) {
		this->I = nnn;
	}else if (prefix == 0xC) {
		uint8_t rnd = rand() % 256;
		this->Vx[x] = kk & rnd;
	}else if (prefix == 0xD) {
		int dX = this->Vx[x];
		int dY = this->Vx[y];

		for(int i = 0; i < 6; i++) {
			for (int j = 0;j < 8;j++) {
				this->displayBuffer[(dY + i) * 64 + dX + j] ^= (this->memory[I+i] >> (7-j)) & 0x1;
			}
		}
		// TODO:
		// modify VF register
		// take care of offscreen rendering
	}else if (prefix == 0xF) {
		switch (kk) {
		case 0x15:
			this->delay_timer = this->Vx[x];
			break;
		case 0x1e:
			this->I = this->I + this->Vx[x];
			break;
		case 0x55:
			for(int i = 0; i <= x; i++) {
				this->memory[this->I+i] = this->Vx[i];
			}
			break;
		case 0x65:
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
