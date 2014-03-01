#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ModbusSlave.h>

enum {
  
  MB_CAMERA,      // 0 R/W
  MB_UP,          // 1 R/W
  MB_DOWN,        // 2 R/W
  MB_LEFT,        // 3 R/W
  MB_RIGHT,       // 4 R/W
  MB_SELECT,      // 5 R/W
  MB_PHOTO_CELL,  // 6 R
  MB_RAIN_SENS,   // 7 R

  MB_REGS
};

#define TX_ENABLE 6

#define SCB2K 0
#define SHC735 1

#define RAIN_SENS 4
#define PHOTO_CELL 5

#define BUTTON_UP     14 // A0
#define BUTTON_DOWN   15 // A1
#define BUTTON_LEFT   16 // A2
#define BUTTON_RIGHT  17 // A3
#define BUTTON_SELECT 18 // A4

#define CMD_UP      8
#define CMD_DOWN    9
#define CMD_LEFT   10
#define CMD_RIGHT  11
#define CMD_SELECT 12

// 1000 = 20s
// 30000 = 10m
#define RAIN_HYSTER_START 15000
#define BUTTON_WAIT_START 10

SoftwareSerial rs485(2,3);
ModbusSlave mbs;
boolean change=false;
int camera=SCB2K;
int buttonWait=0;
int rainHyster=0;
int regs[MB_REGS];
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
    
  rs485.begin(9600);
  pinMode(TX_ENABLE, OUTPUT);
  delay(100);
}

// 'FF', '01', '00', '08', '00', '20', '29' - up
// 'FF', '01', '00', '10', '00', '20', '31' - down
// 'FF', '01', '00', '04', '20', '00', '25' - left
// 'FF', '01', '00', '02', '20', '00', '23' - right
// 'FF', '01', '00', '03', '00', '5F', '63' - select
// 'FF', '00', '00', '40', '00', '00', '41' - zoom0
// 'FF', '00', '00', '20', '00', '00', '21' - zoom1
// 'FF', '00', '04', '00', '00', '00', '05' - iris0
// 'FF', '00', '02', '00', '00', '00', '03' - iris1
// 'FF', '00', '00', '80', '00', '00', '81' - focus0
// 'FF', '00', '01', '00', '00', '00', '02' - focus1

void sendUp() {
  sendCmd(0x00, 0x08, 0x00, 0x20, 0x29);
}

void sendDown() {
  sendCmd(0x00, 0x10, 0x00, 0x20, 0x31 );
}

void sendLeft() {
  sendCmd(0x00, 0x04, 0x20, 0x00, 0x25);
}

void sendRight() {
  sendCmd(0x00, 0x02, 0x20, 0x00, 0x23);
}

void sendSelect() {
  sendCmd(0x00, 0x03, 0x00, 0x5F, 0x63);
}

void sendCmd(int c1, int c2, int d1, int d2, int cksum) {
  digitalWrite(TX_ENABLE, HIGH);
  delay(1);
  rs485.write(0xFF);
  rs485.write(0x01);
  rs485.write(c1);
  rs485.write(c2);
  rs485.write(d1);
  rs485.write(d2);
  rs485.write(cksum);
  delay(50);
  digitalWrite(TX_ENABLE, LOW);
  rs485.flush();
}

void loop() {

  // GET BUTTON STATUS
  
  if(buttonWait<1) {
    regs[MB_UP]=!digitalRead(BUTTON_UP);
    regs[MB_DOWN]=!digitalRead(BUTTON_DOWN);
    regs[MB_LEFT]=!digitalRead(BUTTON_LEFT);
    regs[MB_RIGHT]=!digitalRead(BUTTON_RIGHT);
    regs[MB_SELECT]=!digitalRead(BUTTON_SELECT);
    if(camera==SCB2K) {
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
        buttonWait=BUTTON_WAIT_START;
      }
    }
    else { // SCH735  
      if(regs[MB_UP]==1) { change=true; sendUp(); }
      if(regs[MB_DOWN]==1) { change=true; sendDown(); }
      if(regs[MB_LEFT]==1) { change=true; sendLeft(); }
      if(regs[MB_RIGHT]==1) { change=true; sendRight(); }
      if(regs[MB_SELECT]==1) { change=true; sendSelect(); }
      if(change) {
        change=false;
        buttonWait=BUTTON_WAIT_START;
      }
    }
  }
  else {
    buttonWait--;
  }
  
  for(byte i=0; i<MB_REGS; i++)
    regs[i]=0;
  
  // GET PHOTOCELL STATUS

  r1 = digitalRead(PHOTO_CELL);
  delay(10);
  r2 = digitalRead(PHOTO_CELL); // 2nd check
  if (r2 == r1) {
    regs[MB_PHOTO_CELL]=r1;
  }

  // GET RAIN SENSOR STATUS

  if(rainHyster<1) {
    regs[MB_RAIN_SENS]=!digitalRead(RAIN_SENS);
    if(regs[MB_RAIN_SENS]==1)
      rainHyster=RAIN_HYSTER_START;
  }
  else {
    regs[MB_RAIN_SENS]=1;
    rainHyster--;
  }

  // SET CAMERA
  
  regs[MB_CAMERA]=camera;

  // SET/GET MODBUS STATUS
  if(mbs.update(regs, MB_REGS)>4) {
    camera=regs[MB_CAMERA];

    if(camera==SCB2K) {
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
      }
    }
    else { // SHC735
      if(regs[MB_UP]==1) { change=true; sendUp(); }
      if(regs[MB_DOWN]==1) { change=true; sendDown(); }
      if(regs[MB_LEFT]==1) { change=true; sendLeft(); }
      if(regs[MB_RIGHT]==1) { change=true; sendRight(); }
      if(regs[MB_SELECT]==1) { change=true; sendSelect(); }
      if(change) {
        change=false;
      }
    }
    for(byte i=0; i<MB_REGS; i++)
      regs[i]=0;
  }
  delay(10);
}


