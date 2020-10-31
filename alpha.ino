/**************************************************************
 * Arduino TINY BASIC with a 8080 Emulation
 *
 * Arduino 8080 Virtual Machine with up to 4kB ROM (0x0000-0x0fff) and 1 kB RAM
 * (0x1000-0x13FF)
 *
 * OUT DFh sends data to serial port
 * IN DFh reads data from serial port
 * OUT 0xFE controls LED on pin13 (bit 0)
 * IN DEh returns serial status: 0x02 for no data on input buffer, 0x22 means
 * data are available
 *
 * 8080 emulator part is Copyright (C) 2012 Alexander Demin <alexander@demin.ws>
 * under GPL2 Tiny Basic and Tiny Basic 2 are copylefted by LI-CHEN WANG Source
 * code for BASIC has been compiled with www.asm80.com
 *
 */

#include <Arduino.h>

#include "i8080.h"
#include "i8080_hal.h"

void setup() {
  Serial.begin(115200);
  pinMode(13, OUTPUT);
  i8080_init();
  i8080_jump(0);
}

void loop() {
  // delay(500);
  // examine();
  i8080_instruction();
}

//// MEMORY DEFINITIONS

// test for 8080 emu
const byte PROGMEM ROM[] = {0xF3, 0x31, 0x01, 0x84, 0xFB, 0x3E, 0x01, 0xD3,
                            0xFE, 0xCD, 0x16, 0x00, 0x3E, 0x00, 0xD3, 0xFE,
                            0xCD, 0x16, 0x00, 0xC3, 0x05, 0x00, 0x11, 0x00,
                            0x96, 0x1B, 0x7A, 0xB3, 0xC2, 0x19, 0x00, 0xC9};
// Uncomment for TINY BASIC v1
//#include "basic.h"

// Uncomment for TINY BASIC v2
//#include "basic2.h"

#define CRAMBEG 0x8000
#define CRAMEND 0x8400

// some initial RAM constants for Tiny BASIC 1
byte RAM[CRAMEND - CRAMBEG];

// 1111 11xx xyyy yyyy
byte readByte(int addr) {
  if (addr >= 0 && addr < 0x2000) return pgm_read_byte_near(ROM + addr);
  if (addr >= CRAMBEG && addr < CRAMEND) return RAM[addr - CRAMBEG];
  return 0xFF;  // void memory
}

void writeByte(int addr, byte value) {
  if (addr >= CRAMBEG && addr < CRAMEND) {
    RAM[addr - CRAMBEG] = value;
    return;
  }
}

//// HAL - Hardware Abstraction Layer for Emulator

int i8080_hal_memory_read_byte(int addr) { return (int)readByte(addr); }

void i8080_hal_memory_write_byte(int addr, int value) {
  writeByte(addr, value);
}

int i8080_hal_memory_read_word(int addr) {
  return readByte(addr) | (readByte(addr + 1) << 8);
}

void i8080_hal_memory_write_word(int addr, int value) {
  writeByte(addr, value & 0xff);
  writeByte(addr + 1, (value >> 8) & 0xff);
}

int i8080_hal_io_input(int port) {
  switch (port) {
    case 0xde:  // serial status
      return Serial.available() ? 0x03 : 0x02;
      break;
    case 0xdf:  // serial input
      return Serial.available() ? Serial.read() : 0;
      break;
    default:
      return 0xff;
  }
}

void i8080_hal_io_output(int port, int value) {
  switch (port) {
    case 0xdf:  // serial out
      Serial.print((char)(value & 0x7f));
      break;
    case 0xfe:  // led control
      digitalWrite(13, value & 0x01);
      break;

    default:
      break;
  }
}

void i8080_hal_iff(int on) {
  // no interrupts implemented
}
