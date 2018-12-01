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
Adafruit_GFX_Button buttons[2];
char buttonLabel[2][10] = {"PRACTICE", "BLUETOOTH"};

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

void setup() {
  Serial.begin(9600);
  Serial.println("BEGIN TFT LCD TEST");
  Serial.print("TFT size is "); 
  Serial.print(tft.width()); Serial.print("x"); Serial.println(tft.height());

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

void loop() {
  //detect touch of finger
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);

  /* Used for testing where user is touching the screen
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    Serial.print(ts.readTouchX()); Serial.print(", "); Serial.println(ts.readTouchY());
  } */

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

    //invert button colors if pressed
    if (buttons[b].justReleased()) {
      Serial.print("Released: "); Serial.println(b);
      buttons[b].drawButton();  // draw normal
    }
    else if (buttons[b].justPressed()) {
      buttons[b].drawButton(true);  // draw invert!
    }

    delay(50); // Let's not kill the LCD now
  }
}
