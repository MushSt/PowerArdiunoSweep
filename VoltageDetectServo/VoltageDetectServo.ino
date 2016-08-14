
/*
AttoPilot Current and Voltage Sensing Demo
N.Poole, Sparkfun Electronics, 2011

"I don't care what you do with it, and neither does the script." (apathyware)

Physical Connections:
-------------------------
Arduino  | Peripherals
-------- | --------------
Pin 3  --- SerLCD "RX"
Pin A0 --- AttoPilot "V"
Pin A1 --- AttoPilot "I"
GND ------ AttoPilot "GND"
GND ------ SerLCD "GND"
5V ------- SerLCD "VCC"

This demo will read the Voltage and Current from the "AttoPilot Voltage and Current Sense Board,"
convert the raw ADC data to Volts and Amps and display them as floating point numbers on the
Serial Enabled LCD. (If you would like to do without the Serial LCD, I have included commented code
for reading the results through the Serial Terminal.)

*/

#include <Servo.h>

int VRaw; //This will store our raw ADC data
int IRaw;
float VFinal; //This will store the converted data
float IFinal;
//angle of the Servo
const int start = 35;
const int finish = 180;
const int flatline = 125; //temp var for perfectly flat 

//delay constant
const int delayTime = 1000;

//Servo objects
Servo tilt, pan;

//voltage and current sensors
int volt = A0;
int curr = A1;

void setup() {

  Serial.begin(9600);

  //attaches the servo objects to the corresponding pins
  tilt.attach(11);
  pan.attach(10);

  //give servo default values, starts at 900 and ends at 2100
  pan.write(flatline);
  delay(15);
  tilt.write(170);
  delay(15);
}


void loop() { 
  //Cleanup for LCD (Don't include this line if you are 
  //using a serial terminal instead.

  float currPower = calcPower();
  
  updatePosition(currPower);

  //using only voltage:


  delay(delayTime);

  /*
  //Alternate Display code for terminal.
  If you wish to use the terminal instead of an
  LCD, use this display code instead of the above.
  
  Serial.print(VFinal);
  Serial.println("   Volts");
  Serial.print(IFinal);
  Serial.println("   Amps");
  Serial.println("");
  Serial.println("");
  delay(200);
  
  */
}

//**********************UPDATEONVOLTAGE**********************
/*
 * updates the position based on the voltage reading
 */
void updateOnVoltage(float currVoltage) {
  int currAngle = tilt.read();
  float nextVoltage; //for use in the loops
   
  tilt.write(currAngle+1); //increase the angle to check if its the right direction
  delay(1000);
  int upVoltage = getVoltage();

  //end points
  if(upVoltage > currVoltage) {
    nextVoltage = upVoltage;
    Serial.print("going up");
    for(int i = currAngle; i < finish; ++i) {
      //move to next point
      tilt.write(i);
      delay(1000);
      currVoltage = nextVoltage;
      nextVoltage = getVoltage();

      //stopping condition
      if(nextVoltage < currVoltage) {
        tilt.write(--i); //move back and stop
        break;
      }
    }
  }
  else if(upVoltage < currVoltage){
    nextVoltage = upVoltage;
    for(int i = currAngle; i > start; --i) {
      //move to next point
      tilt.write(i);
      delay(1000);
      currVoltage = nextVoltage;
      nextVoltage = getVoltage();

      //stopping condition;
      if(nextVoltage < currVoltage) {
        tilt.write(++i);
        Serial.print("breaking\n");
        break;
      }
    }
  }
  else {
    Serial.print("do nothing/done");
  }
}
 

//***********************UPDATEPOSITION***********************
/*
 * updates the position given the current Power
 */
void updatePosition(float currPower) {
  int currAngle = tilt.read();
  boolean goUp = checkUpAngle(currPower, currAngle);

  //end points
  if(goUp) {
    Serial.print("going up\n");
    float nextPower = currPower;
    for(int i = currAngle; i < finish; ++i) {
      //move to next point
      tilt.write(i);
      delay(1000);
      currPower = nextPower;
      nextPower = calcPower();

      //stopping condition
      if(nextPower < currPower) {
        tilt.write(--i); //move back and stop
        break;
      }
    }
  }
  else {
    Serial.print("going down\n");
    float nextPower = currPower;
    for(int i = currAngle; i > start; --i) {
      //move to next point
      tilt.write(i);
      delay(1000);
      currPower = nextPower;
      nextPower = calcPower();

      //stopping condition;
      if(nextPower < currPower) {
        tilt.write(++i);
        break;
      }
    }
  }

  
}
//**********************CHECKUPANGLE******************
/*
 * checks if increasing the angle is the correct
 * direction
 */
boolean checkUpAngle(float currPower, int currAngle) {
  tilt.write(currAngle + 1);
  float checkPower = calcPower();

  return checkPower > currPower;
}

//**********************CALCPOWER*********************
/*
 * function to calculate the power
 */
float calcPower() {
  return getVoltage() * getCurrent();
}

//**********************GETVOLTAGE*********************
/*
 * function to get the voltage
 */
float getVoltage() {
  float voltage = 0;
  //analogRead(volt);

  for(int i = 0; i < 4; ++i) {
    voltage += analogRead(volt);
    delay(500);
  }
  voltage = voltage/4;

  //Conversion
  //voltage = voltage/49.44; //45 Amp board
  //VFinal = VRaw/12.99; //90 Amp board
  //VFinal = VRaw/12.99; //180 Amp board

  Serial.print(voltage);
  Serial.print("\n");
    
  return voltage;
}

//**********************GETCURRENT*********************
/*
 * function to get the current
 */
float getCurrent() {
  float current = analogRead(curr);

  //current = current/14.9; //45 Amp board
  //IFinal = IRaw/7.4; //90 Amp board
  //IFinal = IRaw/3.7; //180 Amp board
  
  return 10;
}

