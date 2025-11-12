/*
* File: Embedded_final_project.ino
* Author: Bradley Duguay W0516067 // Alexander Marmura W0516675
* Date: 2025/11/04
* Description: Meow :3
*/
#include "Arduino.h"
#include "uRTCLib.h"
#include <LiquidCrystal.h>
//libraries

#define up_time 13
#define down_time 8
#define scroll_btn 9  //scroll from hours to minutes to mod to time-type on lcd
#define confirm_btn 10



#define lcd_d7 2
#define lcd_d6 3
#define lcd_d5 4
#define lcd_d4 5
#define lcd_enable 11
#define lcd_rs 12

#define motor_1 14
#define motor_2 15
#define motor_3 16
#define motor_4 17

#define rtc_clock 18
#define rtc_data 19
// uRTCLib rtc;
uRTCLib rtc(0x68);
void time_setup();
void time_check();

#define ir_trigger 6
#define ir_echo 7
float ir_function();
//nano pinouts


char weekday[7][3] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
LiquidCrystal lcd(lcd_rs, lcd_enable, lcd_d4, lcd_d5, lcd_d6, lcd_d7);

void setup() {
  Serial.begin(9600);
  URTCLIB_WIRE.begin();
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);


  pinMode(ir_trigger, OUTPUT);
  pinMode(ir_echo, INPUT);

  pinMode(up_time, INPUT);
  pinMode(down_time, INPUT);
  pinMode(confirm_btn, INPUT);
  pinMode(scroll_btn, INPUT);
  lcd.clear();

  time_setup();
}

void loop() {

 time_check();

  float food_level;

  food_level = ir_function();
}

float ir_function() {
  float time_out;
  digitalWrite(ir_trigger, LOW);
  delayMicroseconds(5);
  digitalWrite(ir_trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(ir_trigger, LOW);

  time_out = pulseIn(ir_echo, HIGH);
  //gives time for pulse to return in microseconds, around 400 is around halfway down cup, 800 is totally empty
  Serial.print(time_out);
  Serial.println();

  delay(250);
  return time_out;
}
void time_setup() { /// typedef structs could simplify this codeblock down quite a bit

  int minute = 0, hour = 0, day_week = 1, day_month = 1, month = 1, year = 0;

  lcd.setCursor(0, 0);
  lcd.print("set minutes");
  do {  // set minutes
    if (digitalRead(up_time)) {
      minute++;
    } else if (digitalRead(down_time)) {
      minute--;
    }
    if (minute < 0) {
      minute = 59;
    } else if (minute >= 60) {
      minute = 0;
      lcd.clear();
      lcd.print("set minutes");
    }
    if(minute < 10){ // removes leading 0 from screen
      lcd.clear();
      lcd.print("set minutes");
    }
    lcd.setCursor(0, 1);
    lcd.print(minute);
    delay(500);
  } while (!digitalRead(confirm_btn));

  lcd.clear();
  delay(2000);
  lcd.print("set hour");
  do {  // set hour
    if (digitalRead(up_time)) {
      hour++;
    } else if (digitalRead(down_time)) {
      hour--;
    }
    if (hour < 0) {
      hour = 23;
    } else if (hour > 23) {
      hour = 0;
      lcd.clear();
      lcd.print("set hour");
    }
    if(hour < 10){ // removes leading 0 from screen
      lcd.clear();
      lcd.print("set hour");
    }
    lcd.setCursor(0, 1);
    lcd.print(hour);
    delay(500);
  } while (!digitalRead(confirm_btn));

  lcd.clear();
  delay(2000);
  lcd.print("set weekday");
   do {  // set weekday (sun = 1, sat = 7)
    if (digitalRead(up_time)) {
      day_week++;
    } else if (digitalRead(down_time)) {
      day_week--;
    }
    if (day_week < 1) {
      day_week = 7;
    } else if (day_week > 7) {
      day_week = 1;
    }   

    lcd.setCursor(0, 1);
    for(int i = 0; i < 3; i++){
      lcd.print(weekday[(day_week - 1)] [i]);
    }
    delay(500);

  } while (!digitalRead(confirm_btn));

  lcd.clear();
  delay(2000);
  lcd.print("set day of month");
   do {  // set day of month
    if (digitalRead(up_time)) {
      day_month++;
    } else if (digitalRead(down_time)) {
      day_month--;
    }
    if (day_month < 1) {
      day_month = 31;
    } else if (day_month > 31) {
      day_month = 1;
      lcd.clear();
    //  lcd.setCursor(0, 0);
      lcd.print("set day of month");
    }
    if(day_month < 10){ // removes leading 0 from screen
      lcd.clear();
      lcd.print("set day of month");
    }
    lcd.setCursor(0, 1);
    lcd.print(day_month);
    delay(500);
  } while (!digitalRead(confirm_btn)); 

  lcd.clear();
  delay(2000);
  lcd.print("set month");

   do {  // set month
    if (digitalRead(up_time)) {
      month++;
    } else if (digitalRead(down_time)) {
      month--;
    }
    if (month < 1) {
      month = 12;
    } else if (month > 12) {
      month = 1;
      lcd.clear();
      lcd.print("set month");
    }
    if(month < 10){ // removes leading 0 from screen
      lcd.clear();
      lcd.print("set month");
    }
    lcd.setCursor(0, 1);
    lcd.print(month);
    delay(500);
  } while (!digitalRead(confirm_btn)); 

  lcd.clear();
  delay(2000);
  lcd.print("set year 20XX");
   do {  // set weekday (sun = 1, sat = 7)
    if (digitalRead(up_time)) {
      year++;
    } else if (digitalRead(down_time)) {
      year--;
    }
    if (year < 0) {
      year = 99;
    } else if (year > 99) {
      year = 0;
      lcd.clear();
      lcd.print("set year 20XX");
    }
      if(year < 10){ // removes leading 0 from screen
      lcd.clear();
      lcd.print("set year 20XX");
    }
    lcd.setCursor(0, 1);
    lcd.print(year);
    delay(500);
  } while (!digitalRead(confirm_btn));

  lcd.clear();
  rtc.set(0, minute, hour, day_week, day_month, month, year);
  //rtc.set(second, minute, hour, dayOfWeek, dayOfMonth, month, year)

}
void time_check(){
  rtc.refresh();
  lcd.setCursor(0, 0);
  for(int i = 0; i < 3; i++){
    lcd.print(weekday[(rtc.dayOfWeek() - 1)] [i]);
  }
  lcd.print("/");
  lcd.print(rtc.day());

  lcd.print("/"); 
  lcd.print(rtc.month());

  lcd.print("/20");
  lcd.print(rtc.year());

  lcd.setCursor(0, 1);
  lcd.print(rtc.hour());
  lcd.print(":");
  lcd.print(rtc.minute());
  delay(1000);
  //output should be:

  //day/dd/mm/yyyy
  //hh:mm
}
