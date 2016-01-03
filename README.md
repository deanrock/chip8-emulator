# CHIP8 emulator

Partially working CHIP-8 emulator implemented in C++ with OpenGL and GLUT libraries.

### What is not implemented

* beep sound
* no offscreen rendering (for display buffer)
* 0xFx0A command (wait for key press)


### Usage

Compile via (on OS X):

	g++ -framework GLUT -framework OpenGL -Wdeprecated-declarations main.cpp chip8.cpp

Run by calling:

	./a.out path/to/rom.ch8


### Resources
* https://en.wikipedia.org/wiki/CHIP-8#Opcode_table
* http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
* http://mattmik.com/chip8.html
