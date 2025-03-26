#include <EEPROM.h>

#define IN1 3 
#define IN2 4 
#define ENA 5
#define ADDRESS 0
#define ROTATION 74
#define RADIUS 0.026
#define FORMULA 100
#define CONVERT 60
#define CLOCK 600

int posGivenValue;
int8_t currentPos;
char validate;


void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);

  Serial.begin(115200); // setting up the desired Baud rate
  while (!Serial) {
    // wait for serial port to connect. Needed for native USB port only
  }
}

void loop() {
  Serial.println("\n--------------------------------------------");
  Serial.print("Current pinion position: ");
  currentPos = (int8_t)EEPROM.read(ADDRESS); // get the current positon from an address of the ROM memmory
  Serial.println(currentPos);

  Serial.print("Set the desire location and confirm it (ex: 5 y/n): ");
  while (Serial.available() == 0) {
    //wait for input from the Serial Monitor
  }  
  String str = Serial.readString();
  sscanf(str.c_str(), "%d %c", &posGivenValue, &validate); // read the input data
  Serial.print(String(posGivenValue) + " ");
  Serial.println(validate);

  // switch case depending on different confirm messages
  switch (validate) {
    case 'n':
      Serial.print("No action to be taken, return to the start of the program!");
      break;
    case 'y':
      rotaryMotionToLinerMotion();
      break;
    default:
      Serial.print("No recognized character! Plase input the correct one: 'y' - confirm, 'n' - deny.");
      break;
  }

  Serial.println("\n--------------------------------------------");
}

// function to visualise the change from rotary motion to linear motion
void rotaryMotionToLinerMotion() {
  if (posGivenValue > 1 || posGivenValue < -1) {
    Serial.print("Exceeding rack's limits! Please enter one of these values: '-1', '0', '1'");
  } else if (currentPos == posGivenValue) {
    Serial.print("No change in position, staying steady!");
  } else if (currentPos > posGivenValue) {
    gearMotorMotion(HIGH, LOW);  // rotate the pinion counterclockwise
  } else {
    gearMotorMotion(LOW, HIGH);  // rotate the pinion clockwise
  }
}

// function to set up the rotation speed and direction, having as parameters values that can be HIGH or LOW
void gearMotorMotion(uint8_t val1, uint8_t val2) {
  // map the rotation value from [0,250] interval to [0,230] interval
  int rpmVal = map(ROTATION, 0, 255, 0, 230);
  
  // formula to transform revolutions per minute (RPM) into linear velocity
  float linerVelocity = (2 * PI * RADIUS * rpmVal * FORMULA) / CONVERT;  // from m/s to cm/s
  Serial.print("The speed is: " + String(linerVelocity) + " (cm/s)");
  

  // set the values for the gear motor
  digitalWrite(IN1, val1);
  digitalWrite(IN2, val2);
  analogWrite(ENA, ROTATION);

  int delayValue = 0;
  if (val1 == HIGH) {
    delayValue = (currentPos - posGivenValue) * CLOCK;
  } else {
    //600 ms is the optimal delay, not to exceed the rack's physical dimension limit
    delayValue = (posGivenValue - currentPos) * CLOCK;
  }
  delay(delayValue);

  // stop the gear motor
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  if (digitalRead(IN1) == LOW && digitalRead(IN2) == LOW) {
    //at this stage, the gear motor is not in motion, thus the value is hardcoded to be 0
    Serial.print("The speed is: " + String(0) + " (cm/s)");
  }

  EEPROM.update(ADDRESS, posGivenValue); // update the address of the ROM memory with the current given position
}
