#include <Servo.h>

#include <Metro.h>
#include <SakuraAlpha.h>

SakuraAlphaI2C sakura;

//タイマー割り込み(擬似)
Metro ledMetro = Metro(1000);
Metro serialReadMetro = Metro(100);
Metro sakuraSendMetro = Metro(300);

//前進
const int CMD_FORWARD = 'f';
//後進
const int CMD_BACKWARD = 'b';
//停止
const int CMD_STOP = 's';

//アーム UP
const int CMD_ARM_UP = 'u';
//アーム DOWN
const int CMD_ARM_DOWN = 'd';

//土壌水分
const int smmPin = A0;

// Motor
int dir1PinA =7;
int dir2PinA = 3;
int speedPinA = 5;

// ポンプ
int dir1PinB = 8;
int dir2PinB = 5;
int speedPinB = 6;

//アーム
const int armPin = 4;
Servo servo;
const int ARM_UP = 0;
const int ARM_DOWN = 60;

//LED
const int LED = 13;
int ledValue = HIGH;

struct CmdPack {
  uint32_t smm; /* 土壌水分 */
  uint32_t st;  /* 土壌温度 */
};
struct CmdPack cmdPack;

void motorInit(){
  pinMode(dir1PinA,OUTPUT);
  pinMode(dir2PinA,OUTPUT);
  pinMode(speedPinA,OUTPUT);
  pinMode(dir1PinB,OUTPUT);
  pinMode(dir2PinB,OUTPUT);
  pinMode(speedPinB,OUTPUT);  
}

/* モーター駆動 */
const int MOTOR_F = 0;
const int MOTOR_B = 0;
const int MOTOR_S = 0;

void motorOn(int flg){
  if(flg == MOTOR_F){
    analogWrite(speedPinA, 255);
    digitalWrite(dir1PinA, LOW);
    digitalWrite(dir2PinA, HIGH);
  }
  else if(flg == MOTOR_B){
    analogWrite(speedPinA, 255);
    digitalWrite(dir1PinA, HIGH);
    digitalWrite(dir2PinA, LOW);
  }
  else if(flg == MOTOR_S){
    digitalWrite(dir1PinA, LOW);
    digitalWrite(dir2PinA, LOW);
  }
}

//ポンプ
void pumpOn(bool isON){
    if(isON){
      analogWrite(speedPinB, 255);
      digitalWrite(dir1PinB, LOW);
      digitalWrite(dir2PinB, HIGH);      
    }
    else{
      //OFF
      digitalWrite(dir1PinB, LOW);
      digitalWrite(dir2PinB, LOW);
    }
}

//アームを初期化
void armInit(){
    servo.attach(armPin);
    servo.write(90);
}

//アームを動かす
void armMove(int v){
  servo.write(v);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);

  //モータードライバ初期化
  motorInit();

  //アーム初期化
  armInit();
  
  for(;;){
    if( sakura.getNetworkStatus() == 1 ) break;
    delay(1000);
  }

}

void loop() {
  /*
  if(ledMetro.check() == 1) {
     digitalWrite(13, ledValue);
     ledValue =! ledValue;
  }
  */
  if(sakuraSendMetro.check() == 1) {
    //さくらIoTへ送信
    cmdPack.smm = analogRead(smmPin);
    sakura.writeChannel(0,cmdPack.smm);
    sakura.writeChannel(1,cmdPack.st);
  }
  
  if(serialReadMetro.check() == 1) {
    if (Serial.available()) {
      char c = Serial.read();
      
      digitalWrite(13, ledValue);
      ledValue =! ledValue;

      if(CMD_FORWARD == c){
        //前進
        motorOn(MOTOR_F);
      }
      else if(CMD_BACKWARD == c){
        //後進
        motorOn(MOTOR_B);
      }
      else if(CMD_STOP == c){
        //停止
        motorOn(MOTOR_S);
      }
      else if(CMD_ARM_UP == c){
        //アーム UP
        armMove(ARM_UP);
      }
      else if(CMD_ARM_DOWN == c){
        //アーム DOWN
        armMove(ARM_DOWN);
        //ポンプを動かす 2秒後にオフ 
        pumpOn(true);
        delay(2000);
        pumpOn(false);
      }
            
      //TODO: 仮でエコーして確認
      Serial.write(c);
    }
  }
}
