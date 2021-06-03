// cmd: command
#include<Servo.h>

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal.h>
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE ); 

Servo Door;

#define FLOOR 630 
#define DOOR_TIME 3000

#define SDI 2
#define CLK 3
#define LE 4 

#define SW1 7
#define SW3 22
#define SW2D 24
#define SW2U 26
#define SWclose 28
#define SWopen 30
#define SW2 32

#define SERVO 12

const int stepPin = 5; 
const int dirPin = 6; 

byte data;
int floor_height = 0; // height = steps
bool done_height = false; // flag indicate when finish adjust the 0 height
bool dir = true;
int floor_push[3] = {0,0,0};
int cur_floor = 0;
bool break_flag = 0; // for event when there is a cmd along the direction



int data7[16] = {0,0,0,0 ,0,0,0,0 ,1,1,1,1 ,1,1,1,1}; // control LED7
//                                   g,f,a  b,c,d,e
short extract = 1;
char character[] = {0x3F,0x0C,0x5B,0x5E,0x6C,0x76,0x77,0x1C,0x7F,0x7E};
//                  0   ,1   ,2   ,3   ,4   ,5   ,6   ,7   ,8   ,9


int pre1 = 1;
int pre2 = 1;
int pre2U = 1;
int pre2D = 1;
int pre3 = 1;
int preO = 1;
int preC = 1;
int pre_state(int port, int set_port = -1);
void setup() {
  Serial.begin(9600);
  pinMode(stepPin,OUTPUT); 
  pinMode(dirPin,OUTPUT);

  pinMode(SDI,OUTPUT);
  pinMode(CLK,OUTPUT);
  pinMode(LE,OUTPUT);

  pinMode(SW1,INPUT);
  pinMode(SW3,INPUT);
  pinMode(SW2D,INPUT);
  pinMode(SW2U,INPUT);
  pinMode(SWclose,INPUT);
  pinMode(SWopen,INPUT);
  pinMode(SW2,INPUT);
  
  digitalWrite(LE,LOW);
  digitalWrite(CLK,LOW);

  lcd.backlight();
  lcd.begin(16, 2); // set up the LCD's number of columns and rows
  
  Door.attach(SERVO);
  Door.write(90);
}
void loop() {
  
  read_data();
  if(data == 'w' && done_height == false) {
    dir = 1;
    digitalWrite(dirPin,HIGH);
    for(int x = 0; x < 10; x++) {
      digitalWrite(stepPin,HIGH); 
      delayMicroseconds(2000); 
      digitalWrite(stepPin,LOW); 
      delayMicroseconds(2000); 
      if(done_height)
        floor_height += dir;
    }
    print_inf();
  }
  else if(data == 's' && done_height == false) {
    dir = -1;
    digitalWrite(dirPin,LOW);
    for(int x = 0; x < 10; x++) {
      digitalWrite(stepPin,HIGH); 
      delayMicroseconds(2000); 
      digitalWrite(stepPin,LOW); 
      delayMicroseconds(2000); 
      if(done_height)
        floor_height+=dir;
    }
    print_inf();
  }
  else if(data == 'f') {
    done_height = true;
  }
  else if(data == '1' || data == '2' || data == '3') {
    floor_push[data - 48 -1] = 1;
  }
  else if(data == 'u' || data == 'd') {
    floor_push[1] = 1;
  }
  else if(data == 'g') {
    print_inf();
  }
  three_led7(48+ cur_floor + 1);
  goto_floor(cmd());
  
}

void read_data() {
//  if (Serial.available() > 0) {
//    data = Serial.read();
//    Serial.print("I received: ");
//    Serial.println(char(data));
//    
//  }
//  else {
//    data = ' ';
//  }

//  if(digitalRead(SW1)==0) {
//    data = '3';
//  }
//  else if(digitalRead(SW2)==0)
//    data = '2';
//  else if(digitalRead(SW3)==0)
//    data = '1';
//  else if(digitalRead(SW2U)==0)
//    data = 'u';
//  else if(digitalRead(SW2D)==0)
//    data = 'd';
//  else if(digitalRead(SWopen)==0) {
//    data = 'o';
//  }
//  else if(digitalRead(SWclose)==0)
//    data = 'c';
//  else {
//    data = ' ';
//  }


  if(check_click(SW1)) {
    data = '3';
  }
  else if(check_click(SW2))
    data = '2';
  else if(check_click(SW3))
    data = '1';
  else if(check_click(SW2U))
    data = 'u';
  else if(check_click(SW2D))
    data = 'd';
  else if(check_click(SWopen)) {
    data = 'o';
  }
  else if(check_click(SWclose))
    data = 'c';
  else {
    data = ' ';
  }


}

int cmd() { // search for the next command base on the direction priority
  if(dir) {
    for(int i = cur_floor; i < 3; i++) {
      if(floor_push[i] == 1) {
        return i+1;
      }
    }
    for(int i = cur_floor; i >= 0; i--) {
      if(floor_push[i] == 1) {
        return i+1;
      }
    }
  }
  else {
    for(int i = cur_floor; i >= 0; i--) {
      if(floor_push[i] == 1) {
        return i+1;
      }
    }
    for(int i = cur_floor; i < 3; i++) {
      if(floor_push[i] == 1) {
        return i+1;
      }
    }
  }

  return 0; // no cmd
}

void print_inf() {
    Serial.print("   Current: ");
    Serial.print(cur_floor+1);
    Serial.print("   Dir: ");
    Serial.print(dir);
    Serial.print("   ");
    for(int i = 0; i<3; i++) {
      Serial.print(floor_push[i]);
      Serial.print(" ");
    }
    Serial.print("  Height:");
    Serial.println(floor_height);
}
void goto_floor(int f) {
  if(f == 0) {
    return; //no cmd
  }
  int dif = (f-1) - cur_floor; // find the difference between the cmd and current floor
  
  if(dif > 0) { // determind the elevator direction(up/down)
    dir = 1;
  }
  else if(dif < 0) {
    dir = 0;
  }
  else if(dif == 0) {
    floor_push[cur_floor] = 0; 
    after_finish_cmd();
    return; // at the current floor -> no cmd
  }

  bool remember = 0;
  digitalWrite(dirPin,dir); // set direction for step motor
  for(int i = 0; i < abs(dif); i++) {
    for(int x = 0; x < FLOOR; x++) {
      read_data();
      if(data == '1' || data == '2' || data == '3') {
        floor_push[data - 48 -1] = 1;
        if( ((data - 48 - 1) - cur_floor == 1 && dir ==1) || ((data - 48 - 1) - cur_floor == -1 && dir == 0)  ) // check if thuận đường
          break_flag = true;
      }
      if(data == 'u') {
        floor_push[1] = 1;
        if( (1 - cur_floor) == 1 && dir == 1) {
          break_flag = true;
        }
        else {
          remember = 1;
        }
      }
      if(data == 'd') {
        floor_push[1] = 1;
        if( (1 - cur_floor) == -1 && dir == 0) {
          break_flag = true;
        }
        else {
          remember = 1;
        }
      }
      if(update_led()) {
        three_led7(48+ cur_floor + 1);
      }
      lcd_display(dir);
      digitalWrite(stepPin,HIGH); 
      delayMicroseconds(2000); 
      digitalWrite(stepPin,LOW); 
      delayMicroseconds(2000);
      if(dir==1) {
        floor_height++;
      }
      else if(dir == 0) {
        floor_height--;
      }
    }
    
    if(dir==1) {
      floor_push[++cur_floor] = 0;
    }
    else if(dir == 0) {
      floor_push[--cur_floor] = 0;
    }
    if(remember) {
      floor_push[1] = 1;
    }
    three_led7(48+ cur_floor + 1);
    print_inf();
    if(break_flag) {
      break_flag = false;
      break;
    }
  }
  after_finish_cmd();
}

void after_finish_cmd() {
  lcd.clear();
  three_led7(48+ cur_floor + 1);
  
  bool break_door = 1;
  long long int mil = 0;
  int door_angle = 180;
  long long mil_20 = 0;
  do {
    break_door = 1;
    Door.write(180);
    mil = millis();
    door_angle = 180;
    //delay(500);
    while(millis() - mil < DOOR_TIME) {
      read_data();
      if(data == '1' || data == '2' || data == '3') {
        floor_push[data - 48 -1] = 1;
      }
      else if(data == 'u' || data == 'd') {
        floor_push[1] = 1;
      }
      if(update_led()) {
        three_led7(48+ cur_floor + 1);
      }
      if(data == 'o' ) {
        mil+=DOOR_TIME;
      }
      if(data == 'c') {
        break;
      }
    }
    
    mil = millis();
    
    mil_20 = millis();
    while(millis() - mil < 1800) {
      if(millis() - mil_20 >= 20) {
        mil_20 = millis();
        door_angle--;
        Door.write(door_angle);
      }
      read_data();
      if(data == '1' || data == '2' || data == '3') {
        floor_push[data - 48 -1] = 1;
      }
      else if(data == 'u' || data == 'd') {
        floor_push[1] = 1;
      }
      if(update_led()) {
        three_led7(48+ cur_floor + 1);
      }
      if(data == 'o') {
        break_door = 0;
        break;
      }
    }
  }
  while(!break_door);
  
}

void led7(char ch, bool led) {
  char extract = 1;
  for(int i = 15; i>=9; i--) {
    data7[i] = (extract & character[ch-48]);
    extract <<= 1;
  }
  data7[8] = !led;
  
  for(int i = 0; i<16; i++) {
    digitalWrite(SDI,data7[i]);
    digitalWrite(CLK,LOW);
    digitalWrite(CLK,HIGH);
  }
  digitalWrite(LE,HIGH);
  digitalWrite(LE,LOW);
}
bool update_led() {
  static bool preled[3]={0,0,0};
  static int preled7 =0;
  
  bool flag_dif_led = 0;
  bool flag_dif_led7 = 0;
  for(int i = 0; i<3;i++) {
    if(floor_push[i] != preled[i]) {
      preled[i] = floor_push[i]; 
      flag_dif_led = 1;
    }
  }
  if(preled7 != cur_floor) {
    preled7 = cur_floor;
    flag_dif_led7 = 1;
  }
  return (flag_dif_led || flag_dif_led7);
}
void three_led7(char ch) {
  led7(ch,floor_push[2]);
  led7(ch,floor_push[1]);
  led7(ch,floor_push[0]);
}

bool check_click(int port)
{
  if(check_state(port) == LOW && pre_state(port) == 1)
  {
    pre_state(port,0);
    return true;
  }
  else if(check_state(port) == HIGH && (pre_state(port) == 0))
  {
    pre_state(port,1);
    
  }
  return false;
}
int pre_state(int port, int set_port ) {
  if(set_port != -1) {
    switch(port) {
      case SW1:
        pre1 = set_port;
      case SW2:
        pre2 = set_port;
      case SW2U:
        pre2U = set_port;
      case SW2D:
        pre2D = set_port;
      case SW3:
        pre3 = set_port;
      case SWopen:
        preO = set_port;
      case SWclose:
        preC = set_port;
    }
  }
  
  switch(port) {
    case SW1:
      return pre1;
    case SW2:
      return pre2;
    case SW2U:
      return pre2U;
    case SW2D:
      return pre2D;
    case SW3:
      return pre3;
    case SWopen:
      return preO;
    case SWclose:
      return preC;
  }
}
bool check_state(int port)
{
  if(digitalRead(port) == 0)
  {
    delay(5);
    if(digitalRead(port) == 0)
    {
      delay(5);
      if(digitalRead(port) == 0)
      return LOW;
    }
  }
  return HIGH;
}
void lcd_display(bool dir) {
  static int counter_lcd = 16-1;
  static long long mil_lcd = millis();
  
  if(millis() - mil_lcd >= 75) {
    mil_lcd = millis();
    if(dir == 1) {
      lcd.scrollDisplayRight();
    }
    else if(dir == 0) {
      lcd.scrollDisplayLeft();
    }
    counter_lcd++;
    if(counter_lcd == 16) {
      counter_lcd = 0;
      lcd.clear();
      if(dir == 1) {
        lcd.setCursor(0,0); //col,row
        lcd.print("--->");
      }
      else if(dir == 0) {
        lcd.setCursor(15,0); // col,row
        lcd.print("<---");
      }
    }
  }
}
