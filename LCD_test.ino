// Libraries
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>
#include <MCUFRIEND_kbv.h>

// Control pins for the LCD can be assigned to any digital
// or analog, but we'll just use these analog pins
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

// Assign human-readable names to some common 16-bit color values:
#define BLACK       0x0000
#define BLUE        0x001F
#define RED         0xF800
#define GREEN       0x07E0
#define CYAN        0x07FF
#define MAGENTA     0xF81F
#define YELLOW      0xFFE0
#define WHITE       0xFFFF
#define NAVY        0x000F      /*   0,   0, 128 */
#define DARKGREEN   0x03E0      /*   0, 128,   0 */
#define DARKCYAN    0x03EF      /*   0, 128, 128 */
#define MAROON      0x7800      /* 128,   0,   0 */
#define PURPLE      0x780F      /* 128,   0, 128 */
#define OLIVE       0x7BE0      /* 128, 128,   0 */
#define LIGHTGREY   0xC618      /* 192, 192, 192 */
#define DARKGREY    0x7BEF      /* 128, 128, 128 */
#define BLUE        0x001F      /*   0,   0, 255 */
#define GREEN       0x07E0      /*   0, 255,   0 */
#define CYAN        0x07FF      /*   0, 255, 255 */
#define RED         0xF800      /* 255,   0,   0 */
#define MAGENTA     0xF81F      /* 255,   0, 255 */
#define YELLOW      0xFFE0      /* 255, 255,   0 */
#define WHITE       0xFFFF      /* 255, 255, 255 */
#define ORANGE      0xFD20      /* 255, 165,   0 */
#define GREENYELLOW 0xAFE5      /* 173, 255,  47 */
#define PINK        0xF81F

//button array and variables
Adafruit_GFX_Button buttons[2];                           //for main screen
char buttonLabel[2][10] = {"PRACTICE", "BLUETOOTH"};
Adafruit_GFX_Button buttonsPractice[10];                  //for PRACTICE screen
char buttonPracticeLabel[4][8] = {"+", "-", "BACK", "START"};
char buttonStartLabel[2][10] = {"ROUNDS", "MOVEMENTS"};
Adafruit_GFX_Button buttonsBluetooth[1];
char buttonBluetoothLabel[1][5] = {"BACK"};

//pressure min and max
#define MINPRESSURE 10
#define MAXPRESSURE 1000
#define TS_MINX 130
#define TS_MAXX 905
#define TS_MINY 75
#define TS_MAXY 930

//initialize touch screen (?)
#define YP A2  // must be an analog pin, use "An" notation!
#define XM A3  // must be an analog pin, use "An" notation!
#define YM 8   // can be a digital pin
#define XP 9   // can be a digital pin
MCUFRIEND_kbv tft;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

//other important variables
boolean set = false; //for LOOP
int speeds = 5;      //for PRACTICE
int rounds = 3;
int movements = 2;
int breaks = 10;
int values[4] = {speeds, rounds, movements, breaks};

void setup() {
  Serial.begin(9600);

  tft.reset();
  
  uint16_t identifier = tft.readID(); // read LCD driver
  tft.begin(identifier);
  tft.setRotation(1); // LANDSCAPE
  tft.fillScreen(BLACK);

  //tft.fillRect(x, y, width, height, color);
  //tft.drawRect(x, y, width, height, color);
  //POINT RELATIVE TO TOP RIGHT CORNER
  tft.fillRect(80, 20, 320, 80, BLACK);
  tft.setCursor(89, 50);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.print("FOOTWORX BADMINTON TRAINER");

  //buttons[x].initButton(&tft, x, y, w, h, border, backgroundcolor, textcolor, text, textsize);
  //BUTTON RELATIVE TO MIDDLE POINT OF BUTTON
  buttons[0].initButton(&tft, 240, 150, 320, 80, NAVY, NAVY, WHITE, buttonLabel[0], 2);
  buttons[1].initButton(&tft, 240, 250, 320, 80, RED, RED, WHITE, buttonLabel[1], 2);
  buttons[0].drawButton();
  buttons[1].drawButton();
}

/*
 * LOOP 
 */

void loop() {
  //need a new setup?
  if (set == true){
    setter:
    Serial.print("NANI");
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
    p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
    p.y = tft.height()-(map(p.y, TS_MINY, TS_MAXY, tft.height(), 0));
  }

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

/*
 * PRACTICE function -> transitions to practice page 
 */
 
void practice() {
  // set up new tft screen
  setterPractice:
  Serial.println("right here asshat");
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
 * START function -> used to transition to keep track of current movement and round
 */

void start(){
  // new variables
  int counterRounds;
  int counterMovements;
  double codedSpeed = (-(5/3) * speeds + 20) * 150; //speed used in code
  
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
  for(counterRounds = 0; counterRounds < rounds; counterRounds++){
    for(counterMovements = 0; counterMovements <= movements; counterMovements++){
      tft.fillRect(270, 100, 150, 120, LIGHTGREY);
      tft.setCursor(340, 150); 
      tft.print(counterMovements);
      delay(codedSpeed);
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
      delay(breaks * 1000);
    }
  }
}

/*
 * BLUETOOTH function -> connectivity to phone 
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

