/*
  timerは動作が重くなるため、使用しない
  ドア開フラグでカウントが自動的に更新されてしまうのを防ぐ
*/
#include <Wire.h>
//#include <TimerTC3.h>
#include <LiquidCrystal.h>

// LCD ←→ Arduinoのピンの割り当て
// rs      →   D3
// rw      →   GND
// enable  →   D6
// d4      →   D7
// d5      →   D8
// d6      →   D9
// d7      →   D10
//#define SERIAL_DEBUG
#define CONFIG 0x1A
#define ACCEL_XOUT_H 0x3B
#define GYRO_XOUT_H 0x43
#define PWR_MGMT_1 0x6B
#define WHO_AM_I 0x75
#define ACCEL_CONFIG 0x1C
#define PI 3.14159

float accel_ave[3];   // 0:x_axis, 1:y_axis, 2:z_axis
int ang[2];           // 0:Xangle, 1:Yangle
int addr = 0b1101001;
int cnt = 0, cnt_300 = 0;
bool open_f=0;

// [構文]LiquidCrystal(rs, rw,  enable, d0, d1, d2, d3, d4, d5, d6, d7)
LiquidCrystal lcd(3,6,7,8,9,10);

void get_accel(float *accel)
{
  Wire.beginTransmission(addr);
  Wire.write(ACCEL_XOUT_H);
  Wire.endTransmission();
  Wire.requestFrom(addr, 6);
  accel[0] = (float)(Wire.read() << 8 | Wire.read());
  accel[1] = (float)(Wire.read() << 8 | Wire.read());
  accel[2] = (float)(Wire.read() << 8 | Wire.read());

  if (accel[0] >= 32768)
    accel[0] = accel[0] - 65536.0;
  if (accel[1] >= 32768)
    accel[1] = accel[1] - 65536.0;
  if (accel[2] >= 32768)
    accel[2] = accel[2] - 65536.0;

  accel[0] = accel[0]/16384.0;
  accel[1] = accel[1]/16384.0;
  accel[2] = accel[2]/16384.0;
}

void get_gyro(float *gyro)
{
  Wire.beginTransmission(addr);
  Wire.write(GYRO_XOUT_H);
  Wire.endTransmission();
  Wire.requestFrom(addr, 6);
  gyro[0] = (int)(Wire.read() << 8 | Wire.read()) / 131.0;
  gyro[1] = (int)(Wire.read() << 8 | Wire.read()) / 131.0;
  gyro[2] = (int)(Wire.read() << 8 | Wire.read()) / 131.0;
}

void setup() {
  //LCDの桁数と行数を指定する(16桁2行)
  lcd.begin(16,2);
 
  //画面をクリアしてカーソルの位置を左上(0,0)にする
  lcd.clear();

  // connect sensor
  Wire.begin();

  Wire.beginTransmission(addr);
  Wire.write(CONFIG);
  Wire.write(0x00);
  Wire.endTransmission();

  Wire.beginTransmission(addr);
  Wire.write(PWR_MGMT_1);
  Wire.write(0x01);
  Wire.endTransmission();

  Wire.beginTransmission(addr);
  Wire.write(ACCEL_CONFIG);
  Wire.write(0x00);
  Wire.endTransmission();

  Wire.beginTransmission(addr);
  Wire.write(WHO_AM_I);
  Wire.endTransmission();
  Wire.requestFrom(addr, 1);

  if (Wire.read() == 0x12) {
    lcd.print(" connect:OK");
  } else  {
    lcd.print(" connect:NG");
  }
  // Seial initilize
#ifdef SERIAL_DEBUG
  Serial.begin(9600);
  while(!Serial);
  Serial.println("---------------------------------------");
  if (Wire.read() == 0x12) {\
    Serial.println("OK");
  } else  {\
    Serial.println("NG");
  }
#endif
  delay(1000);

  //TimerTc3.initialize(1000); //1000usタイマー
  //TimerTc3.attachInterrupt(calc_average)  ; //割り込み関数を指定
}

void loop() {
  float accel_raw[3], accel[3];
  
  for(int i=0; i<100; i++)
  {
    get_accel(accel_raw);
    accel[0] = accel_raw[0] + accel[0];
    accel[1] = accel_raw[1] + accel[1];
    accel[2] = accel_raw[2] + accel[2];
  }

  accel_ave[0] = accel[0]/100;
  accel_ave[1] = accel[1]/100;
  accel_ave[2] = accel[2]/100;

  if(abs(accel_ave[2]) > 0.9)           // close処理
  {
    if(open_f)
    {
      cnt++;
      open_f = 0;
      if(cnt>=300)
      {
        cnt = 0;
        cnt_300++;
      }
    }
    //while(abs(accel_ave[0]) < 0.9);
  }
  else if(abs(accel_ave[0]) < 0.15)    // open処理
    open_f = 1;
  
  lcd.clear();
  lcd.setCursor(0,0); //カーソルの位置を指定(0桁0行目の位置)
  lcd.print(accel_ave[0]);
  lcd.print(",");
  lcd.print(accel_ave[1]);
  lcd.print(","); 
  lcd.print(accel_ave[2]);

  
  lcd.setCursor(0,1); //カーソルの位置を指定(0桁1行目の位置)
 /* lcd.print("cnt:");
  lcd.print(cnt);
  lcd.print("  ");
  lcd.print("cnt300:");
  lcd.print(cnt_300);*/

  lcd.print("cnt,cnt300=");
  lcd.print(cnt);
  lcd.print(",");
  lcd.print(cnt_300);

#ifdef SERIAL_DEBUG
  Serial.print("ax:");
  Serial.print(accel_ave[0]);
  Serial.print(",");
  Serial.print("ay:");
  Serial.print(accel_ave[1]);
  Serial.print(",");
  Serial.print("az:");
  Serial.print(accel_ave[2]);
  Serial.print(",");
  Serial.print("open:");
  Serial.print(open_f);
  Serial.print(",");

  /*ang[0] = atan2(accel_ave[0], accel_ave[2])/PI*180.0;
  ang[1] = atan2(accel_ave[1], accel_ave[2])/PI*180.0;
  Serial.print("X_ang:");
  Serial.print(ang[0]);
  Serial.print(",");
  Serial.print("Y_ang:");
  Serial.print(ang[1]);
  */
  Serial.println("");
#endif
  delay(1);
}