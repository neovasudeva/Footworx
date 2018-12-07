// Libraries
#include <Adafruit_GFX.h>    
#include <Adafruit_TFTLCD.h> 
#include <TouchScreen.h>
#include <MCUFRIEND_kbv.h>

// Control pins for the LCD 
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

// Assign human-readable names to some common 16-bit color values:
#define BLACK       0x0000
#define BLUE        0x001F
#define RED         0xF800
#define GREEN       0x07E0
#define WHITE       0xFFFF
#define NAVY        0x000F      /*   0,   0, 128 */
#define DARKGREEN   0x03E0      /*   0, 128,   0 */
#define LIGHTGREY   0xC618      /* 192, 192, 192 */
#define DARKGREY    0x7BEF      /* 128, 128, 128 */
#define GREEN       0x07E0      /*   0, 255,   0 */

//button array and variables used on different pages
Adafruit_GFX_Button buttons[2];                           //for MAIN screen
char buttonLabel[2][10] = {"PRACTICE", "BLUETOOTH"};
Adafruit_GFX_Button buttonsPractice[10];                  //for PRACTICE screen
char buttonPracticeLabel[4][8] = {"+", "-", "BACK", "START"};
char buttonStartLabel[2][10] = {"ROUNDS", "MOVEMENTS"};
Adafruit_GFX_Button buttonsBluetooth[1];                  //for BLUETOOTH screen
char buttonBluetoothLabel[1][5] = {"BACK"};

//pressure min and max for touch detection
#define MINPRESSURE 10
#define MAXPRESSURE 1000
#define TS_MINX 130
#define TS_MAXX 905
#define TS_MINY 75
#define TS_MAXY 930

//initialize touch screen
#define YP A2  // must be an analog pin, use "An" notation!
#define XM A3  // must be an analog pin, use "An" notation!
#define YM 8   // can be a digital pin
#define XP 9   // can be a digital pin
MCUFRIEND_kbv tft;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

//variables for shift registers controlling LEDs
#define shiftCLK 12
#define latch 11
#define dataIN 10

//variables for speed, rounds, and movements per round and other important variables
int speeds = 3; //speed customized by user; limited from 0 to 10, 10 being fastest
int rounds = 2; //rounds customized by and displayed to user; limited from 0 to 100
int movements = 5; //movements custmomized by and displayed to user; imited from 0 to 100
int breaks = 10; //breaks customized by user; limited from 0 to 30 seconds

double codedSpeed; //speed used in code
  //NOTE: lights turn on for (codedSpeed - 250)ms and turn off for (250)ms
double codedBreak; //calculated break

int values[4] = {speeds, rounds, movements, breaks};  //for PRACTICE
boolean set = false; //for LOOP

//array to for MOVEMENT METHOD to turn on a single random light
byte lights[] = {0b10000000, 0b010000000, 0b00100000, 0b00010000, 0b00001000, 0b00000100, 0b00000010, 0b00000001};

/*
 * SETUP METHOD
 * sets up all variables and begins serial communication
 */
void setup() {
  pinMode(shiftCLK, OUTPUT);
  pinMode(latch, OUTPUT);
  pinMode(dataIN, OUTPUT);

  //begin serial communication for debugging
  Serial.begin(9600);

  //setup of MAIN page
  tft.reset();
  uint16_t identifier = tft.readID(); // read LCD driver
  tft.begin(identifier);
  tft.setRotation(1); // LANDSCAPE
  tft.fillScreen(BLACK);

  //setup title of MAIN page
  tft.fillRect(80, 20, 320, 80, BLACK);
  tft.setCursor(89, 50);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.print("FOOTWORX BADMINTON TRAINER");

  //setup buttons in MAIN page
  buttons[0].initButton(&tft, 240, 150, 320, 80, NAVY, NAVY, WHITE, buttonLabel[0], 2);
  buttons[1].initButton(&tft, 240, 250, 320, 80, RED, RED, WHITE, buttonLabel[1], 2);
  buttons[0].drawButton();
  buttons[1].drawButton();
}

/*
 * FLASH METHOD
 * Used to prepare the user for a round.
 * It first flashes slow, then flashes fast in the last 3 seconds
 */
void flash(int number){
  //slow flash, each flash-on-flash-off cycle lasts 1 second
  for(int i = 0; i < number; i++){
    digitalWrite(latch, LOW);
    shiftOut(dataIN, shiftCLK, LSBFIRST, 0b11111111); 
    shiftOut(dataIN, shiftCLK, LSBFIRST, 0b11111111); 
    digitalWrite(latch, HIGH);
    delay(500);
    digitalWrite(latch, LOW);
    shiftOut(dataIN, shiftCLK, LSBFIRST, 0b00000000); 
    shiftOut(dataIN, shiftCLK, LSBFIRST, 0b00000000); 
    digitalWrite(latch, HIGH);
    delay(500);
  }

  //fast flash, each flash-on-flash-off cycle lasts 1/2 seconds (hence, * 2 in for-loop)
  for(int i = 0; i < 3 * 2; i++){
    digitalWrite(latch, LOW);
    shiftOut(dataIN, shiftCLK, LSBFIRST, 0b11111111); 
    shiftOut(dataIN, shiftCLK, LSBFIRST, 0b11111111); 
    digitalWrite(latch, HIGH);
    delay(250);
    digitalWrite(latch, LOW);
    shiftOut(dataIN, shiftCLK, LSBFIRST, 0b00000000); 
    shiftOut(dataIN, shiftCLK, LSBFIRST, 0b00000000); 
    digitalWrite(latch, HIGH);
    delay(250);
  }
  
}

/*
 * MOVEMENT METHOD
 * Generates random numbers to simulate movement to a single corner
 * When randomRegister == 1, it displays light on top register
 * When randomRegister == 2, it displays light on bottom register
 * It will do a full turn-on-turn-off cycle for a light
 */
 
void movement(){
  //random numbers to choose register and a single light
  int randomRegister = (int) random(1, 3);
  int randomLight = (int) random(8);

  //displaying light on register
  if(randomRegister == 1){
    //turn on
    digitalWrite(latch, LOW);
    shiftOut(dataIN, shiftCLK, LSBFIRST, lights[randomLight]); //top register
    shiftOut(dataIN, shiftCLK, LSBFIRST, 0b00000000); 
    digitalWrite(latch, HIGH);
    delay(codedSpeed - 250);
    //turn off
    digitalWrite(latch, LOW);
    shiftOut(dataIN, shiftCLK, LSBFIRST, 0b00000000); //top register
    shiftOut(dataIN, shiftCLK, LSBFIRST, 0b00000000); 
    digitalWrite(latch, HIGH);
    delay(250);
  } else {
    //turn on
    digitalWrite(latch, LOW);
    shiftOut(dataIN, shiftCLK, LSBFIRST, 0b00000000); 
    shiftOut(dataIN, shiftCLK, LSBFIRST, lights[randomLight]); //bottom register
    digitalWrite(latch, HIGH);
    delay(codedSpeed - 250);
    //turn off
    digitalWrite(latch, LOW);
    shiftOut(dataIN, shiftCLK, LSBFIRST, 0b00000000); 
    shiftOut(dataIN, shiftCLK, LSBFIRST, 0b00000000); //bottom register
    digitalWrite(latch, HIGH);
    delay(250);
  }
}

/*
 * PRACTICE function 
 * transitions to PRACTICE page, where user sets up all parameters in order
 * to do footwork
 */
 
void practice() {
  // set up new tft screen
  setterPractice:
  tft.reset();
  uint16_t identifier = tft.readID(); // read LCD driver
  tft.begin(identifier);
  tft.setRotation(1); // LANDSCAPE
  tft.fillScreen(BLACK);

  //draw labels for speed, rounds, movements, and rest
  tft.fillRect(5, 5, 470, 50, BLACK);
  tft.setCursor(50, 25);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.print("SPEED   ROUNDS  MOVEMENTS  REST");
  
  //draw buttons
  int x = 81;
  int yPlus = 90;
  int yMinus = 200;
  for(int i = 0; i < 4; i++) {
      buttonsPractice[i*2].initButton(&tft, x, yPlus, 50, 50, RED, RED, WHITE, buttonPracticeLabel[0], 2);
      buttonsPractice[(i*2) + 1].initButton(&tft, x, yMinus, 50, 50, NAVY, NAVY, WHITE, buttonPracticeLabel[1], 2);
      buttonsPractice[i*2].drawButton();
      buttonsPractice[(i*2)+1].drawButton();

      x += 106;
  }

  //textboxes to display numbers
  x = 56;
  yPlus = 125;
  for (int i = 0; i < 4; i++){
    tft.fillRect(x, yPlus, 50, 40, BLACK);
    tft.setCursor(x+15, yPlus+15);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.print(values[i]);

    x += 106;
  }

  //draw buttons for "BACK" and "START"
  buttonsPractice[8].initButton(&tft, 140, 273, 120, 65, DARKGREEN, DARKGREEN, WHITE, buttonPracticeLabel[2], 2);
  buttonsPractice[9].initButton(&tft, 340, 273, 120, 65, DARKGREEN, DARKGREEN, WHITE, buttonPracticeLabel[3], 2);
  buttonsPractice[8].drawButton();
  buttonsPractice[9].drawButton();

  //will continue to stay on PRACTICE screen until "START" or "BACK" is pressed
  while(true){
    //detect touch of finger
    digitalWrite(13, HIGH);
    TSPoint p = ts.getPoint();
    digitalWrite(13, LOW);
    
    // if sharing pins, you'll need to fix the directions of the touchscreen pins
    //pinMode(XP, OUTPUT);
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);
    //pinMode(YM, OUTPUT);
    
    if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
      p.x = p.x;
      p.y = p.y;
      p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
      p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
    }
    
    for (int b = 0; b < 10; b++) {
      //check if buttons were pressed
      if ((buttonsPractice[b].contains(p.x, p.y)) && p.x > 10) {
        Serial.print("Pressing: "); Serial.println(b);
        buttonsPractice[b].press(true);  // tell the button it is pressed
      } else {
        buttonsPractice[b].press(false);  // tell the button it is NOT pressed
      }
    
      //invert button colors if pressed and transition to next screen
      if (buttonsPractice[b].justReleased()) {
        Serial.print("Released: "); Serial.println(b);
        buttonsPractice[b].drawButton();  // draw normal
      }
      if (buttonsPractice[b].justPressed()) {
        buttonsPractice[b].drawButton(true);  // draw invert !

        //switch case used for incrementing and decrementing variables
        switch (b){
          case 0:
            speeds++;
            if(speeds >= 0 && speeds <= 10){
              tft.fillRect(56, 126, 50, 40, BLACK);
              tft.setCursor(71, 140);
              tft.setTextColor(WHITE);
              tft.setTextSize(2);
              tft.print(speeds);
            } else {
              speeds--;
            }
            break;
          case 1:
            speeds--;
            if(speeds >= 0 && speeds <= 10){
              tft.fillRect(56, 126, 50, 40, BLACK);
              tft.setCursor(71, 140);
              tft.setTextColor(WHITE);
              tft.setTextSize(2);
              tft.print(speeds);
            } else {
              speeds++;
            }
            break;
          case 2:
            rounds++;
            tft.fillRect(162, 126, 50, 40, BLACK);
            tft.setCursor(177, 140);
            tft.setTextColor(WHITE);
            tft.setTextSize(2);
            tft.print(rounds);
            break;
          case 3:
            rounds--;
            if(rounds >= 0){
              tft.fillRect(162, 126, 50, 40, BLACK);
              tft.setCursor(177, 140);
              tft.setTextColor(WHITE);
              tft.setTextSize(2);
              tft.print(rounds);
            } else {
              rounds++;
            }
            break;
          case 4:
            movements++;
            tft.fillRect(268, 126, 50, 40, BLACK);
            tft.setCursor(283, 140);
            tft.setTextColor(WHITE);
            tft.setTextSize(2);
            tft.print(movements);
            break;
          case 5:
            movements--;
            if(movements >= 0){
              tft.fillRect(268, 126, 50, 40, BLACK);
              tft.setCursor(283, 140);
              tft.setTextColor(WHITE);
              tft.setTextSize(2);
              tft.print(movements);
            } else {
              movements++;
            }
            break;
          case 6:
            breaks++;
            tft.fillRect(374, 126, 50, 40, BLACK);
            tft.setCursor(389, 140);
            tft.setTextColor(WHITE);
            tft.setTextSize(2);
            tft.print(breaks);
            break;
          case 7:
            breaks--;
            if(breaks >= 0){
              tft.fillRect(374, 126, 50, 40, BLACK);
              tft.setCursor(389, 140);
              tft.setTextColor(WHITE);
              tft.setTextSize(2);
              tft.print(breaks);
            } else {
              breaks++;
            }  
            break;
          case 8:
            goto beginning;
          case 9: 
            start();
            goto setterPractice;
        }
      }
      delay(5); // Let's not kill the LCD now
    }
  }
  beginning: 
  delay(1);
}


/*
 * START function 
 * used to transition to START page, which keeps track 
 * of current movement and round while the user is doing footwork
 */

void start(){
  // new variables
  int counterRounds;
  int counterMovements;
  codedSpeed = (-(5/3) * speeds + 20) * 150; //speed used in code
  codedBreak = breaks * 1000; //Break time used in code
  
  // set up new tft screen
  tft.reset();
  uint16_t identifier = tft.readID(); // read LCD driver
  tft.begin(identifier);
  tft.setRotation(1); // LANDSCAPE
  tft.fillScreen(BLACK);

  //create textboxes for labels
  tft.fillRect(5, 30, 460, 50, BLACK);
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.setCursor(10, 40);
  tft.print("    ROUNDS    MOVEMENTS");

  // create textboxes for current round and current movement
  tft.fillRect(60, 100, 150, 120, LIGHTGREY);
  tft.fillRect(270, 100, 150, 120, LIGHTGREY);
  tft.setCursor(130, 150);
  tft.print(0);
  tft.setCursor(340, 150); 
  tft.print(0);

  //ACTUAL EXECUTION OF CODE
  flash(10);
  for(counterRounds = 0; counterRounds < rounds; counterRounds++){
    for(counterMovements = 0; counterMovements <= movements; counterMovements++){
      tft.fillRect(270, 100, 150, 120, LIGHTGREY);
      tft.setCursor(340, 150); 
      tft.print(counterMovements);
      movement();
    }

    tft.fillRect(60, 100, 150, 120, LIGHTGREY);
    tft.setCursor(130, 150); 
    tft.print(counterRounds + 1);

    tft.fillRect(270, 100, 150, 120, LIGHTGREY);
    tft.setCursor(340, 150); 
    tft.print(0);
    
    if(counterRounds == rounds - 1){
      delay(1000);
    } else {
      flash(breaks - 3);
    }
  }
}

/*
 * BLUETOOTH function
 * Connects to bluetooth and allows a user to control the lights 
 * on the board manually
 */

void bluetooth() {
  // set up new tft screen
  tft.reset();
  uint16_t identifier = tft.readID(); // read LCD driver
  tft.begin(identifier);
  tft.setRotation(1); // LANDSCAPE
  tft.fillScreen(BLACK);

  //draw labels for speed, rounds, movements, and rest
  tft.fillRect(5, 5, 470, 50, BLACK);
  tft.setCursor(50, 25);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.print("SPEED   ROUNDS  MOVEMENTS   REST");

  //textboxes to display numbers
  int x = 56;
  int yPlus = 125;
  for (int i = 0; i < 4; i++){
    tft.fillRect(x, yPlus, 50, 40, BLACK);
    tft.setCursor(x+15, yPlus+15);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.print(values[i]);

    x += 106;
  }

  //draw button for "BACK"
  buttonsBluetooth[0].initButton(&tft, 140, 273, 120, 65, DARKGREEN, DARKGREEN, WHITE, buttonBluetoothLabel[0], 2);
  buttonsBluetooth[0].drawButton();

  //will continue to stay on PRACTICE screen until "START" or "BACK" is pressed
  while(true){
    //detect touch of finger
    digitalWrite(13, HIGH);
    TSPoint p = ts.getPoint();
    digitalWrite(13, LOW);
    
    // if sharing pins, you'll need to fix the directions of the touchscreen pins
    //pinMode(XP, OUTPUT);
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);
    //pinMode(YM, OUTPUT);
    
    if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
      p.x = p.x;
      p.y = p.y;
      p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
      p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
    }
    
    if ((buttonsBluetooth[0].contains(p.x, p.y)) && p.x > 10) {
      Serial.print("Pressing: "); Serial.println(0);
      buttonsBluetooth[0].press(true);  // tell the button it is pressed
    } else {
      buttonsBluetooth[0].press(false);  // tell the button it is NOT pressed
    }
  
    //invert button colors if pressed and transition to next screen
    if (buttonsBluetooth[0].justReleased()) {
      Serial.print("Released: "); Serial.println(0);
      buttonsBluetooth[0].drawButton();  // draw normal
    }

    //if user clicks on "BACK" button
    if (buttonsBluetooth[0].justPressed()) {
      buttonsBluetooth[0].drawButton(true);  // draw invert !
      break;
    }
  }
  beginning: 
  delay(1);
}

/* 
 *  MAIN METHOD
 *  Uses all defined functions and runs the code
 */
void loop() {
  //new setup for when other pages go back to MAIN page
  //NOTE: not used when device is first opened
  if (set == true){
    setter:
    tft.reset();
    uint16_t identifier = tft.readID(); // read LCD driver
    tft.begin(identifier);
    tft.setRotation(1); // LANDSCAPE
    tft.fillScreen(BLACK);

    tft.fillRect(80, 20, 320, 80, BLACK);
    tft.setCursor(89, 50);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.print("FOOTWORX BADMINTON TRAINER");
    
    buttons[0].initButton(&tft, 240, 150, 320, 80, NAVY, NAVY, WHITE, buttonLabel[0], 2);
    buttons[1].initButton(&tft, 240, 250, 320, 80, RED, RED, WHITE, buttonLabel[1], 2);
    buttons[0].drawButton();
    buttons[1].drawButton();
  }
  
  //create TSpoint object for detection of finger
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);

  //pinMode(XP, OUTPUT);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  //pinMode(YM, OUTPUT);

  //sets up detection of pressure and maps it
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    p.x = p.x;
    p.y = p.y;
    p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
    p.y = tft.height()-(map(p.y, TS_MINY, TS_MAXY, tft.height(), 0));
  }

  //detects pressing of buttons and changes page appropriately
  for (int b = 0; b <= 1; b++) {
    //check if buttons were pressed
    if ((buttons[b].contains(p.x, p.y)) && p.x > 10) {
      Serial.print("Pressing: "); Serial.println(b);
      buttons[b].press(true);  // tell the button it is pressed
    } else {
      buttons[b].press(false);  // tell the button it is NOT pressed
    }

    //invert button colors if pressed and transition to next screen
    if (buttons[b].justReleased()) {
      Serial.print("Released: "); Serial.println(b);
      buttons[b].drawButton();  // draw normal
      if(b == 0){
        practice();
        goto setter;
      } else if (b == 1) {
        bluetooth();
        goto setter;
      }
    }
    if (buttons[b].justPressed()) {
      buttons[b].drawButton(true);  // draw invert !
    }
    delay(5); // Let's not kill the LCD now
  }
}
