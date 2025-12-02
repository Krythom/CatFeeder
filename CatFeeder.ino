/*
* File: Embedded_final_project.ino
* Author: Bradley Duguay W0516067 // Alexander Marmura W0516675
* Date: 2025/11/04
* Description: Meow :3
*/
#include "Arduino.h"
#include "uRTCLib.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <AccelStepper.h>
//libraries

#define up_btn 13
#define down_btn 8
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
#define MotorInterfaceType 4

void motor_control();

#define rtc_clock 19
#define rtc_data 18
// uRTCLib rtc;
uRTCLib rtc(0x68);
void date_setup();
void time_check();
int lastMinute;
int lastHour;

#define ir_trigger 6
#define ir_echo 7
float ir_function();
float distance;
//nano pinouts & variables

typedef struct timeOfDay {
  int hour;
  int minute;
};

timeOfDay feedTimes[6];
AccelStepper myStepper(MotorInterfaceType, motor_1, motor_3, motor_2, motor_4);

char weekday[7][3] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
char months[12][3] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
LiquidCrystal lcd(lcd_rs, lcd_enable, lcd_d4, lcd_d5, lcd_d6, lcd_d7);

void setup() {
  Serial.begin(9600);
  URTCLIB_WIRE.begin();

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.clear();

  //IR Sensor setup
  pinMode(ir_trigger, OUTPUT);
  pinMode(ir_echo, INPUT);

  //Button setup
  pinMode(up_btn, INPUT);
  pinMode(down_btn, INPUT);
  pinMode(confirm_btn, INPUT);
  pinMode(scroll_btn, INPUT);

  myStepper.setMaxSpeed(100.0);
  myStepper.setAcceleration(50.0);
  myStepper.setSpeed(10); 

  //Load feed times from EEPROM
  for (int i = 0; i < 6; i++) {
    EEPROM.get(i * sizeof(timeOfDay), feedTimes[i]);
  }
  
  lastMinute = rtc.minute();
  time_check();
  delay(100);  //avoids confirm button being read high on startup
}

void loop() {
  if (digitalRead(confirm_btn))
  {
    lcd.clear();
    delay(500);
    MainMenu();
  }
  rtc.refresh();
  if (rtc.minute() != lastMinute) //only redraw screen when time has updated
  {
    lastMinute = rtc.minute();
    lastHour = rtc.hour();
    time_check();

for(int i = 0; i < 6; i++ ){
      if(lastHour == feedTimes[i].hour){
        if(lastMinute == feedTimes[i].minute){
          motor_control();
          delay(2500);
          distance = ir_function();
          if(distance < 400){
            Serial.println("low food");
          }
        }
    }
}

    //Check if current time == any of the feed times
    //If yes call function to dispense food
    //Either in dispense food function or after it call ir function
    //If food level is low enough call another function to display an alert to refill food
  }
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

void MainMenu() {
  int selection = 0;
  lcd.clear();
  lcd.print("1: Date Setup");
  lcd.setCursor(0, 1);
  lcd.print("2: Feed Times  X");
  lcd.setCursor(0, 0);
  lcd.blink();

  do {
    if (digitalRead(scroll_btn)) {
      selection = (selection + 1) % 3;  // 3 options
      switch (selection) {              //move cursor to appropriate location
        case 0:
          lcd.setCursor(0, 0);
          break;
        case 1:
          lcd.setCursor(0, 1);
          break;
        case 2:
          lcd.setCursor(15, 1);
          break;
      }
      delay(500);
    }
  } while (!digitalRead(confirm_btn));

  lcd.noBlink();
  delay(500);
  switch (selection) { //go to selsected menu
    case 0:
      date_setup();
      break;
    case 1:
      SetFeedTimes();
    default:
      time_check();
      break;
  }
}

void SetFeedTimes() {
  int selection = 0;
  bool screenSetup = true;

  do {
    if (screenSetup)  //avoid refreshing the full screen unless necessary
    {
      lcd.blink();
      lcd.clear();
      lcd.print("Edit Feed Times");
      lcd.setCursor(0, 1);
      for (int i = 1; i < 7; i++) {
        if (feedTimes[i - 1].hour == -1)  //disabled time
        {
          lcd.print("/ ");
        } else {
          lcd.print(i);
          lcd.print(" ");
        }
      }
      lcd.print("X");
      lcd.setCursor(selection * 2, 1);
      screenSetup = false;
    }

    if (digitalRead(up_btn)) {
      selection = (selection + 1) % 7;  //7 options
      lcd.setCursor(selection * 2, 1);
      delay(500);
    } else if (digitalRead(down_btn)) {
      selection = (selection + 6) % 7;
      lcd.setCursor(selection * 2, 1);
      delay(500);
    } else if (digitalRead(scroll_btn) && selection != 6)  //disables/enables selected feed time
    {
      if (feedTimes[selection].minute == -1)  //re-enable selection
      {
        lcd.print(selection + 1);
        lcd.setCursor(selection * 2, 1);
        feedTimes[selection].minute = 0;
        feedTimes[selection].hour = 0;
      } else  //disable selection
      {
        lcd.print("/");
        lcd.setCursor(selection * 2, 1);
        feedTimes[selection].minute = -1;
        feedTimes[selection].hour = -1;
      }

      delay(500);
    } else if (digitalRead(confirm_btn) && selection != 6 && feedTimes[selection].minute != -1)  //goes to time setting if selection is not disabled and selection is not to exit
    {
      delay(500);
      feedTimes[selection] = SetTime(feedTimes[selection].hour, feedTimes[selection].minute);
      screenSetup = true;
      delay(500);
    }
  } while (!(digitalRead(confirm_btn) && selection == 6));
  lcd.noBlink();

  for (int i = 0; i < 6; i++)  //save the new feed times to eeprom
  {
    EEPROM.put(i * sizeof(timeOfDay), feedTimes[i]);
  }

  delay(500);

}

timeOfDay SetTime(int hour, int minute) {  //this can prob be shorter but it works
  timeOfDay setTime = { hour, minute };
  int cursorPos = 1;
  lcd.clear();
  lcd.print("Set Time");
  lcd.setCursor(0, 1);
  if (hour < 10) {
    lcd.print("0");
  }
  lcd.print(hour);
  lcd.print(":");
  if (minute < 10) {
    lcd.print("0");
  }
  lcd.print(minute);
  lcd.setCursor(1, 1);
  lcd.blink();

  do {
    if (digitalRead(scroll_btn)) {
      //alternate between setting hours and minutes
      if (cursorPos == 1) {
        cursorPos = 4;
      } else {
        cursorPos = 1;
      }
      lcd.setCursor(cursorPos, 1);
      delay(500);
    } else if (digitalRead(up_btn)) {
      if (cursorPos == 1)  //hours
      {
        setTime.hour = (setTime.hour + 1) % 24;
        lcd.setCursor(0, 1);
        if (setTime.hour < 10) {
          lcd.print("0");
        }
        lcd.print(setTime.hour);
        lcd.setCursor(cursorPos, 1);
      } else  //minutes
      {
        setTime.minute = (setTime.minute + 1) % 60;
        lcd.setCursor(3, 1);
        if (setTime.minute < 10) {
          lcd.print(0);
        }
        lcd.print(setTime.minute);
        lcd.setCursor(cursorPos, 1);
      }
      delay(500);
    } else if (digitalRead(down_btn)) {
      if (cursorPos == 1)  //hours
      {
        setTime.hour = (setTime.hour + 23) % 24;  //equivalent to subtracting 1 but because of how % works need to do this instead
        lcd.setCursor(0, 1);
        if (setTime.hour < 10) {
          lcd.print("0");
        }
        lcd.print(setTime.hour);
        lcd.setCursor(cursorPos, 1);
      } else  //minutes
      {
        setTime.minute = (setTime.minute + 59) % 60;  //equivalent to -5
        lcd.setCursor(3, 1);
        if (setTime.minute < 10) {
          lcd.print(0);
        }
        lcd.print(setTime.minute);
        lcd.setCursor(cursorPos, 1);
      }
      delay(500);
    }
  } while (!digitalRead(confirm_btn));

  lcd.noBlink();
  return setTime;
}

void date_setup() {
  int minute = 0, hour = 0, day_week = 0, day_month = 1, month = 0, year = 0;
  //Set current time
  timeOfDay currentTime = SetTime(0, 0);
  hour = currentTime.hour;
  minute = currentTime.minute;
  lcd.clear();
  delay(500);

  lcd.print("Set Weekday");
  do {  // set weekday (sun = 0, sat = 6)
    if (digitalRead(up_btn)) {
      day_week = (day_week + 1) % 7;
      delay(500);
    } else if (digitalRead(down_btn)) {
      day_week = (day_week + 6) % 7;
      delay(500);
    }
    lcd.setCursor(0, 1);
    for (int i = 0; i < 3; i++) {
      lcd.print(weekday[(day_week)][i]);
    }
  } while (!digitalRead(confirm_btn));
  lcd.clear();
  delay(500);

  //TODO: maybe? add protection against setting fake dates
  lcd.print("Set Day of Month");
  do {  // set day of month
    if (digitalRead(up_btn)) {
      day_month++;
      delay(500);
    } else if (digitalRead(down_btn)) {
      day_month--;
      delay(500);
    }
    if (day_month < 1) { //left these ifs alone cause the modulo solution is kinda annoying to deal with when 0 isn't a valid number
      day_month = 31;
    } else if (day_month > 31) {
      day_month = 1;
    }
    lcd.setCursor(0, 1);
    if (day_month < 10) 
    {
      lcd.print("0");
    }
    lcd.print(day_month);
  } while (!digitalRead(confirm_btn));
  lcd.clear();
  delay(500);

  lcd.print("Set Month");
  do {  // set month (jan = 0, dec = 6)
    if (digitalRead(up_btn)) {
      month = (month + 1) % 12;
      delay(500);
    } else if (digitalRead(down_btn)) {
      month = (month + 11) % 12;
      delay(500);
    }
    lcd.setCursor(0, 1);
    for (int i = 0; i < 3; i++) {
      lcd.print(months[(month)][i]);
    }
  } while (!digitalRead(confirm_btn));
  lcd.clear();
  delay(500);

  lcd.print("Set Year 20XX");
  do {  // set weekday (sun = 1, sat = 7)
    if (digitalRead(up_btn)) {
      year = (year + 1) % 100;
      delay(500);
    } else if (digitalRead(down_btn)) {
      year = (year + 99) % 100;
      delay(500);
    }
    lcd.setCursor(0, 1);
    if (year < 10) {
      lcd.print("0");
    }
    lcd.print(year);
  } while (!digitalRead(confirm_btn));
  lcd.clear();
  delay(500);

  rtc.set(0, minute, hour, day_week, day_month, month, year);
}

void time_check() {
  rtc.refresh();
  lcd.clear();
  for (int i = 0; i < 3; i++) {
    lcd.print(weekday[(rtc.dayOfWeek())][i]);
  }
  lcd.print(" ");
  for (int i = 0; i < 3; i++) {
    lcd.print(months[rtc.month()][i]);
  }
  lcd.print(" ");
  lcd.print(rtc.day());
  lcd.print(" 20");
  lcd.print(rtc.year());

  lcd.setCursor(0, 1);
  if (rtc.hour() < 10) {
    lcd.print(0);
  }
  lcd.print(rtc.hour());
  lcd.print(":");
  if (rtc.minute() < 10) {
    lcd.print(0);
  }
  lcd.print(rtc.minute());
  delay(1000);
}
void motor_control(){
  myStepper.move(1024);
   if (myStepper.distanceToGo() != 0){
    do{
     myStepper.run();
    }while(myStepper.distanceToGo() != 0);
  }
}
