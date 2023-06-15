#include "wiiIRcam.h"
#include "HID-Project.h"
#include "MsTimer2.h"

// IR座標からマウス座標に変換するときのオフセット
#define WIIMOTE_MOUSE_OFFSET 120

#define LEFT_MOUSE_PIN    18
#define RIGHT_MOUSE_PIN   9

#define LEFT_KEY_PIN      19
#define RIGHT_KEY_PIN     5
#define UP_KEY_PIN        6
#define DOWN_KEY_PIN      4

#define ADJUST_PIN        15

#define POLLING_RATE 20

wiiIRcam cam;

// マウス座標 (IR座標ではない)
volatile int32_t formerX  = 0, formerY = 0, currentX, currentY, outX, outY;

volatile uint8_t sensitivity = 2;

// LPF用定数
const float T = POLLING_RATE;
const float Ti = 100;

void setup() {

  pinMode(LEFT_MOUSE_PIN, INPUT_PULLUP);
  pinMode(RIGHT_MOUSE_PIN, INPUT_PULLUP);

  pinMode(LEFT_KEY_PIN, INPUT_PULLUP);
  pinMode(RIGHT_KEY_PIN, INPUT_PULLUP);
  pinMode(UP_KEY_PIN, INPUT_PULLUP);
  pinMode(DOWN_KEY_PIN, INPUT_PULLUP);

  pinMode(ADJUST_PIN, INPUT_PULLUP);

  Serial.begin(115200);
  cam.begin();
  AbsoluteMouse.begin();

  MsTimer2::set(POLLING_RATE, interrupt);
  MsTimer2::start();
}


void interrupt() {

  sei();  //割り込み内でWire利用のため

  // カーソル
  if( cam.get() ){

    if(cam.x1 != 1023 && cam.y1 != 1023 ){
      currentX = map(cam.x1, WIIMOTE_X_MIN + WIIMOTE_MOUSE_OFFSET, WIIMOTE_X_MAX - WIIMOTE_MOUSE_OFFSET, INT16_MIN, INT16_MAX);
      currentY = map(cam.y1, WIIMOTE_Y_MIN + WIIMOTE_MOUSE_OFFSET, WIIMOTE_Y_MAX - WIIMOTE_MOUSE_OFFSET, INT16_MIN, INT16_MAX);

      outX = Ti/(T+Ti)*formerX + T/(T+Ti)*currentX;
      outY = Ti/(T+Ti)*formerY + T/(T+Ti)*currentY;

      if(INT16_MAX < outX){
        outX = INT16_MAX;
      } else if(outX < INT16_MIN){
        outX = INT16_MIN;
      }

      if(INT16_MAX < outY){
        outY = INT16_MAX;
      } else if(outY <= INT16_MIN){   //Y方向だけなぜかINT16_MINで画面上にとび抜ける…？
        outY = INT16_MIN + 1;
      }

      AbsoluteMouse.moveTo( outX, -outY );
    }

    formerX = outX;
    formerY = outY;
  }

  // キーボード
  if( !digitalRead(LEFT_MOUSE_PIN) ){
    AbsoluteMouse.press(MOUSE_LEFT);
  } else if( digitalRead(LEFT_MOUSE_PIN) ){
    AbsoluteMouse.release(MOUSE_LEFT);
  }

  if( !digitalRead(RIGHT_MOUSE_PIN) ){
    AbsoluteMouse.press(MOUSE_RIGHT);
  } else if( digitalRead(RIGHT_MOUSE_PIN) ){
    AbsoluteMouse.release(MOUSE_RIGHT);
  }

  if( !digitalRead(UP_KEY_PIN) ){
    Keyboard.press(KEY_UP_ARROW);
  } else if( digitalRead(UP_KEY_PIN) ){
    Keyboard.release(KEY_UP_ARROW);
  }

  if( !digitalRead(DOWN_KEY_PIN) ){
    Keyboard.press(KEY_DOWN_ARROW);
  } else if( digitalRead(DOWN_KEY_PIN) ){
    Keyboard.release(KEY_DOWN_ARROW);
  }

  if( !digitalRead(LEFT_KEY_PIN) ){
    Keyboard.press(KEY_LEFT_ARROW);
  } else if( digitalRead(LEFT_KEY_PIN) ){
    Keyboard.release(KEY_LEFT_ARROW);
  }

  if( !digitalRead(RIGHT_KEY_PIN) ){
    Keyboard.press(KEY_RIGHT_ARROW);
  } else if( digitalRead(RIGHT_KEY_PIN) ){
    Keyboard.release(KEY_RIGHT_ARROW);
  }

  if( !digitalRead(ADJUST_PIN) ){
    while(!digitalRead(ADJUST_PIN)){}
    
    (4 <= sensitivity) ? (sensitivity = 0) : (sensitivity++);
    
    cam.begin(sensitivity);
  }
}

void loop(){

}
