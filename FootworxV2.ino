//variables for shift registers controlling LEDs
#define shiftCLK 8
#define latch 9
#define dataIN 10

//variables for speed, rounds, and movements per round
double speeds = 3; //speed customized by user; limited from 0 to 10, 10 being fastest
double rounds = 2; //rounds customized by and displayed to user; limited from 0 to 100
double movements = 5; //movements custmomized by and displayed to user; imited from 0 to 100
double breaks = 10; //breaks customized by user; limited from 0 to 30 seconds

double codedSpeed = (-(5/3) * speeds + 20) * 150; //speed used in code
  //NOTE: when lighting the lights, lights turn on for (codedSpeed - 250)ms and turn off for (250)ms
double codedBreak = breaks * 1000; //calculated break

//runtime varaibles to keep track of round and movements completed
double currentRound = 0;
double currentMovement = 0;

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

}

/*
 * FLASH METHOD
 * Used to prepare the user for a round.
 * It first flashes slow, then flashes fast in the last 3 seconds
 */
void flash(){
  //slow flash, each flash-on-flash-off cycle lasts 1 second
  for(int i = 0; i < breaks - 3; i++){
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

  //fast flash, each flash-on-flash-off cycle lasts 1/2 seconds (hence * 2 in for loop)
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
 *  MAIN METHOD
 *  Uses all defined functions and runs the code
 */
void loop() {
  //outer loop for rounds
  for(int i = 0; i < rounds; i++){
    flash();
    
    //inner loop for movements
    for(int j = 0; i < movements; j++){
      movement();
    }
  }
}
