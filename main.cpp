#include <iostream>
#include <stdlib.h>
#include <GLUT/glut.h>
#include <thread>
#include "chip8.h"

using namespace std;

int loadFile(string name, char **mem) {
    ifstream file;
    file.open(name, ios::in|ios::ate);

    if (!file.is_open()) {
      return -1;
    }

    streampos size = file.tellg();
    *mem = new char[size];
    file.seekg(0, ios::beg);
    file.read(*mem, size);

    file.close();
    return size;
}

void draw(void);
void key(unsigned char, int, int);

char *screen;
int width, height;
int scale = 8;

CHIP8 *chip8;
mutex displayBufferMutex;

void chip8executor() {
    while(1) {
        displayBufferMutex.lock();
        bool success = chip8->cycle();
        displayBufferMutex.unlock();

      if (!success) {
        exit(1);
      }

      usleep(1000);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        cout << "please specify file path as first argument" << endl;
        return 1;
    }

    char *mem;
    int size = loadFile(string(argv[1]), &mem);

    if (size < 0) {
        cout << "cannot open specified file" << endl;
        return 1;
    }

    if (size % 2 != 0) {
        cout << "wrong number of bytes in file" << endl;
        return 1;
    }

    // print ROM file
    for (int i = 0; i < size; i+=2) {
        printf("0x%.2X\t0x%.2X\n", mem[i], mem[i+1]);
    }

    // load ROM
    chip8 = new CHIP8();
    chip8->load((uint8_t*)mem, size);
    delete[] mem;

    // prepare screen buffer
    width = 64*scale;
    height = 32*scale;

    screen = new char[width*height*3];

    // prepare GLUT
    glutInitWindowSize(width, height);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutCreateWindow("CHIP-8 emulator");

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glutDisplayFunc(draw);
    glutKeyboardFunc(key);

    thread t1(chip8executor);

    glutMainLoop();

    t1.join();
}

void draw(void) {
    displayBufferMutex.lock();
    for (int c_y = 0; c_y < 32; c_y++) {
        for (int c_x = 0; c_x < 64; c_x++) {
            int c8_pos = c_y*64 + c_x;
            int color = chip8->displayBuffer[c8_pos] ? 255 : 0;

            for (int x = 0;x<scale;x++) {
                for(int y = 0; y < scale; y++) {
                    int pos =( ((31-c_y)*scale+y) * (64*scale) + c_x*scale+x) * 3;
                    screen[pos+0] = color;
                    screen[pos+1] = color;
                    screen[pos+2] = color;
                }
            }
        }
    }
    displayBufferMutex.unlock();

    glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, screen);

    glutSwapBuffers();
    glutPostRedisplay();
}

void key(unsigned char k, int x, int y) {
    cout << "unhandled key " << k << endl;
}
