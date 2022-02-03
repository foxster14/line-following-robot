#define IRPrinterDetector 42
#define ObstacleUltraEcho 13
#define ObstacleUltraTrig 12
#define PaperUltraEcho 3
#define PaperUltraTrig 2
const byte IRPins[5] ={50, 48, 52, 46, 44};
int motorSpeedA = 0;
int motorSpeedB = 0;
byte state = 0;
const byte moving = 1;
const byte loading = 2;
const byte unloading = 3;
const byte waiting = 4;
byte route = NULL;
byte returnTally = 0;

struct MotorController
{
  byte EN_PIN;
  byte IN1_PIN;
  byte IN2_PIN;

  bool forward(bool state, int speed=-1)
  {
    digitalWrite(IN1_PIN, !state);
    digitalWrite(IN2_PIN, state);

    if (speed > -1) setSpeed(speed);
  } 
  
  void setSpeed(byte speed)
  {
    analogWrite(EN_PIN, speed);
  }

  void setup()
  {
    pinMode(EN_PIN, OUTPUT);
    pinMode(IN1_PIN, OUTPUT);
    pinMode(IN2_PIN, OUTPUT);
    setSpeed(0);
    forward(true);
  }
} rightMotor, leftMotor;

void setup() {
  // put your setup code here, to run once:
  pinMode(IRPrinterDetector, INPUT);
  pinMode(ObstacleUltraEcho, INPUT);
  pinMode(ObstacleUltraTrig, OUTPUT);
  pinMode(PaperUltraEcho, INPUT);
  pinMode(PaperUltraTrig, OUTPUT);
  
  rightMotor.EN_PIN = A0;
  rightMotor.IN1_PIN = 4;
  rightMotor.IN2_PIN = 5;
  rightMotor.setup();

  leftMotor.EN_PIN = A1;
  leftMotor.IN1_PIN = 6;
  leftMotor.IN2_PIN = 7;
  leftMotor.setup();
  
  for(int i = 0; i < 6; i++) {
    pinMode(IRPins[i], INPUT);
  }

  state = loading;
}

float distanceFormulaPaper() {
  float duration = 0;
  float distance = 0;
  digitalWrite(PaperUltraTrig, LOW);
  delayMicroseconds(2);
  digitalWrite(PaperUltraTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(PaperUltraTrig, LOW);
  duration = pulseIn(PaperUltraEcho, HIGH);
  distance = (duration * .0343) / 2;
  return distance;
}

float distanceFormulaObstacle() {
  float duration = 0;
  float distance = 0;
  digitalWrite(ObstacleUltraTrig, LOW);
  delayMicroseconds(2);
  digitalWrite(ObstacleUltraTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(ObstacleUltraTrig, LOW);
  duration = pulseIn(ObstacleUltraEcho, HIGH);
  distance = (duration * .0343) / 2;
  return distance;
}

bool paperLoaded() {
   return(distanceFormulaPaper() > 50 && distanceFormulaPaper() <= 250);
}

bool setRoute() {
  int IRPaperVal = digitalRead(IRPrinterDetector);
  int IRCodeVal = NULL;

  if (paperLoaded() == true) {
    if (IRPaperVal = LOW) { 
      IRCodeVal = 1; // 1 means white is detected while 2 is A return
    }  
    else if (IRPaperVal == HIGH) {
    IRCodeVal = 0; // 0 means black is detected, while 3 is B return
    }
  }
 route = IRCodeVal;
}

bool obstacle() {
  return(distanceFormulaObstacle() <= 350);
}

byte lineFollower() {
  byte read = 0;
  for(int i = 0; i < 6; i++) {
    read ^= digitalRead(IRPins[i]) << i;
  }
  return read;
}

void LeftTurn() {
  leftMotor.forward(false, 255);
  rightMotor.forward(true, 255);
  delay(2000);
}

void RightTurn() {
  leftMotor.forward(true, 255);
  rightMotor.forward(false, 255);
  delay(2000);
}

void loop() {
  // put your main code here, to run repeatedly:
  switch(state) {
    case moving:
    if (obstacle() == true) {
      state = waiting;
    }
    switch (lineFollower()) {
      case 0b00100: //Forward
        rightMotor.forward(true, 255);
        leftMotor.forward(true, 255);
        break;
      
      case 0b01000: //Slght Left
      case 0b10000:
      rightMotor.forward(true, 255);
      leftMotor.setSpeed(0);
        break;

      case 0b00010: //Slight Right
      case 0b00001:
      leftMotor.forward(true, 255);
      rightMotor.setSpeed(0);
        break;

      case 0b11100:
      if(route == 1) {
        LeftTurn();
      }

      else if (route == 3) {
        LeftTurn();
      }
        break;

      case 0b00111:
        
      if (route == 0) {
        RightTurn();
      }
      break;

      case 0b11111:
      if(route == 1) {
        leftMotor.setSpeed(0);
        rightMotor.setSpeed(0);
        state = unloading;
      }
      else if (route == 2) {
        if (returnTally == 0) {
          RightTurn();
          returnTally++;
        }
        else if(returnTally == 1){
          leftMotor.setSpeed(0);
          rightMotor.setSpeed(0);
          state = loading;
        }
      }
      else if (route == 0) {
        leftMotor.setSpeed(0);
        rightMotor.setSpeed(0);
        state = unloading;
      }
      else if (route == 3) {
        leftMotor.setSpeed(0);
        rightMotor.setSpeed(0);
        state = loading;
      }
        break;
    }
      break;

    case loading:
      if (paperLoaded() == true) {
        setRoute();
        state = moving;
      }
      break;

    case unloading:
    delay(10000);
    RightTurn;
    RightTurn;
    if (route == 1) {
      route = 2;
    }

    if (route == 0) {
      route = 3;
    }
    state = moving;
      break;

    case waiting:
    rightMotor.setSpeed(0);
    leftMotor.setSpeed(0);
    if (obstacle == false) {
      state = moving;
    }
      break;
  }
}