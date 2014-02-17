#include <Arduino.h>
#include <ModbusSlave.h>

enum {
  
  MB_UP,          // 0 R/W
  MB_DOWN,        // 1 R/W
  MB_LEFT,        // 2 R/W
  MB_RIGHT,       // 3 R/W
  MB_SELECT,      // 4 R/W
  MB_PHOTO_CELL,  // 5 R
  MB_RAIN_SENS,   // 4 R

  MB_REGS
};

#define RAIN_SENS 4
#define PHOTO_CELL 5

//#define BUTTON_UP      8
//#define BUTTON_DOWN    9
//#define BUTTON_LEFT   10
//#define BUTTON_RIGHT  11
//#define BUTTON_SELECT 12
#define BUTTON_UP     14 // A0
#define BUTTON_DOWN   15 // A1
#define BUTTON_LEFT   16 // A2
#define BUTTON_RIGHT  17 // A3
#define BUTTON_SELECT 18 // A4

//#define CMD_UP     2
//#define CMD_DOWN   3
//#define CMD_LEFT   4
//#define CMD_RIGHT  5
//#define CMD_SELECT 6
#define CMD_UP      8
#define CMD_DOWN    9
#define CMD_LEFT   10
#define CMD_RIGHT  11
#define CMD_SELECT 12

ModbusSlave mbs;
boolean change=false;
int regs[MB_REGS];
int r0 = 9;
int r1 = 0;
int r2 = 0;

void setup() {

  pinMode(CMD_UP, OUTPUT);
  pinMode(CMD_DOWN, OUTPUT);
  pinMode(CMD_LEFT, OUTPUT);
  pinMode(CMD_RIGHT, OUTPUT);
  pinMode(CMD_SELECT, OUTPUT);
  
  pinMode(PHOTO_CELL, INPUT); // external pull-up
  pinMode(RAIN_SENS, INPUT); // external pull-up

  pinMode(BUTTON_UP, INPUT);
  pinMode(BUTTON_DOWN, INPUT);
  pinMode(BUTTON_LEFT, INPUT);
  pinMode(BUTTON_RIGHT, INPUT);
  pinMode(BUTTON_SELECT, INPUT);
  
  // activate pullup resistor
  digitalWrite(BUTTON_UP, HIGH);
  digitalWrite(BUTTON_DOWN, HIGH);
  digitalWrite(BUTTON_LEFT, HIGH);
  digitalWrite(BUTTON_RIGHT, HIGH);
  digitalWrite(BUTTON_SELECT, HIGH);
  
  mbs.configure(1, 115200, 'n', 0);
  for(byte i=0; i<MB_REGS; i++)
    regs[i]=0;
  delay(100);
}

void loop() {

  // GET BUTTON STATUS
  
  regs[MB_UP]=!digitalRead(BUTTON_UP);
  regs[MB_DOWN]=!digitalRead(BUTTON_DOWN);
  regs[MB_LEFT]=!digitalRead(BUTTON_LEFT);
  regs[MB_RIGHT]=!digitalRead(BUTTON_RIGHT);
  regs[MB_SELECT]=!digitalRead(BUTTON_SELECT);
  
  if(regs[MB_UP]==1) { change=true; digitalWrite(CMD_UP, HIGH); }
  if(regs[MB_DOWN]==1) { change=true; digitalWrite(CMD_DOWN, HIGH); }
  if(regs[MB_LEFT]==1) { change=true; digitalWrite(CMD_LEFT, HIGH); }
  if(regs[MB_RIGHT]==1) { change=true; digitalWrite(CMD_RIGHT, HIGH); }
  if(regs[MB_SELECT]==1) { change=true; digitalWrite(CMD_SELECT, HIGH); }

  if(change) {
    delay(50);
    digitalWrite(CMD_UP, LOW);
    digitalWrite(CMD_DOWN, LOW);
    digitalWrite(CMD_LEFT, LOW);
    digitalWrite(CMD_RIGHT, LOW);
    digitalWrite(CMD_SELECT, LOW);
    change=false;
    for(byte i=0; i<MB_REGS; i++)
      regs[i]=0;
  }

  // GET PHOTOCELL STATUS

  r1 = digitalRead(PHOTO_CELL);
  if (r1 != r0) {
    delay(50);
    r2 = digitalRead(PHOTO_CELL); // 2nd check
    if (r2 == r1) {
      if (r1 == HIGH) {
        regs[MB_PHOTO_CELL]=1;
      }
      else {
        regs[MB_PHOTO_CELL]=0;
      }
      r0 = r1;
    }
  }

  // GET RAIN SENSOR STATUS

  regs[MB_RAIN_SENS]=digitalRead(RAIN_SENS);

  // SET/GET MODBUS STATUS

  if(mbs.update(regs, MB_REGS)>4) {
    if(regs[MB_UP]==1) { change=true; digitalWrite(CMD_UP, HIGH); }
    if(regs[MB_DOWN]==1) { change=true; digitalWrite(CMD_DOWN, HIGH); }
    if(regs[MB_LEFT]==1) { change=true; digitalWrite(CMD_LEFT, HIGH); }
    if(regs[MB_RIGHT]==1) { change=true; digitalWrite(CMD_RIGHT, HIGH); }
    if(regs[MB_SELECT]==1) { change=true; digitalWrite(CMD_SELECT, HIGH); }
    
    if(change) {
      delay(50);
      digitalWrite(CMD_UP, LOW);
      digitalWrite(CMD_DOWN, LOW);
      digitalWrite(CMD_LEFT, LOW);
      digitalWrite(CMD_RIGHT, LOW);
      digitalWrite(CMD_SELECT, LOW);
      change=false;
      for(byte i=0; i<MB_REGS; i++)
        regs[i]=0;
    }
  
  }

}


