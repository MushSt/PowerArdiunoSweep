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
#include <LinkedList.h>

int VRaw; //This will store our raw ADC data
int IRaw;
float VFinal; //This will store the converted data
float IFinal;
//angle of the Servo

//Constants:
const int start = 35;
const int finish = 180;
const int flatline = 125; //temp var for perfectly flat
const int stepSize = 5; //for calibration
const int sampleSize = 7; 
const int movementPause = 250;

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

  //calibrate the best position, sweep 2x for redundancy
  calibrateTilt();
  calibratePan();
  calibrateTilt();
  calibratePan();
}

//*****************MAIN_LOOP********************
void loop() {

  
  
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

//**********************CALIBRATE_TILT****************
/*
   does the initial calibration of the tilt servo

   Full sweep of the servo's range stored in a list
   move to optimal position
*/
void calibrateTilt() {
  LinkedList<float> powerData = LinkedList<float>();

  //get raw data and populate the list
  for(int i = start; i < finish; ++i) {
    tilt.write(i);
    delay(movementPause);
    powerData.add(calcPower());
  }

  //smooth the raw data out to find the best position
  int bestIndex = smoothData(powerData);
  tilt.write(bestIndex + start); //optimum position
}

//**********************CALIBRATE_PAN*****************
/*
   does the initial calibration of the pan servo

   Full sweep of servo's range stored in a list
   move to optimal position
 */
void calibratePan() {
  LinkedList<float> powerData = LinkedList<float>();

  //get raw data and populate the list
  for(int i = start; i < finish; ++i) {
    pan.write(i);
    delay(movementPause);
    powerData.add(calcPower());
  }

  //smooth raw data
  int bestIndex = smoothData(powerData);
  pan.write(bestIndex + start);
}

//**********************SMOOTH_DATA*******************
/*
 * Applies a mean filter to smooth out the data
 * returns the index of the peak
 */
int smoothData(LinkedList<float> rawData) {
  LinkedList<float> sample = LinkedList<float>(); //sublist to get an average
  LinkedList<float> smoothedData = LinkedList<float>(); //to hold the smoothed data

  int dataSize = rawData.size();
  int offset = sampleSize/2;

  //pad the smoothed data with 0s to represent the thrown away vals
  for(int i = 0; i < offset*2; ++i) {
    smoothedData.add(0);
  }

  //ignore the starting vals
  for(int i = offset; i < dataSize-offset; ++i) {
    for(int j = i - offset; j < sampleSize; ++j) {
      //populate the sample
      sample.add(rawData.get(j));
    }
    //sample has been populated, get the mean and place it into smoothed list
    float smoothed = getMedian(sample);
    smoothedData.add(i, smoothed);
  }

  //find the max value, and in the event there is a plateau, take the middle one
  float maxVal = smoothedData.get(0);
  int maxIndex = 0;
  
  for(int i = 0; i < dataSize; ++i) {
    float nextVal = smoothedData.get(i);
    if(nextVal > maxVal) {
      maxVal = nextVal;
      maxIndex = i;
    }
  }

  return maxIndex;
}

//**********************GET_MEDIAN*****************
/*
 * calculates and returns the median of a list
 */
float getMedian(LinkedList<float> sublist) {
  //sort into new list
  int listSize = sublist.size();
  LinkedList<float> sorted = LinkedList<float>();
  
  //sort the list
  for(int i = 0; i < listSize; ++i) {
    int sortedSize = sorted.size();
    float nextVal = sublist.pop();
    
    //inner loop to place removed element 
    for(int j = 0; j < sortedSize; ++j) {
      //reached the end of the list
      if(j == sortedSize-1) {
        sorted.add(nextVal);
      }
      //place the value if its less than the next element
      if(nextVal < sorted.get(j)) {
        sorted.add(j, nextVal);
      }
    }
  }

  return sorted.get(listSize/2);
}


//**********************GET_MEAN*******************
/*
 * calculates and returns the mean of a list
 */
float getMean(LinkedList<float> sublist) {
  int sizeOfList = sublist.size();
  float mean = 0;

  for(int i = 0; i < sizeOfList; ++i) {
    mean += sublist.pop();
  }

  return mean/sizeOfList;
}

//**********************CALCPOWER*********************
/*
   function to calculate the power
*/
float calcPower() {
  return getVoltage() * getCurrent();
}

//**********************GETVOLTAGE*********************
/*
   function to get the voltage
*/
float getVoltage() {
  float voltage = 0;
  //analogRead(volt);

  for (int i = 0; i < 5; ++i) {
    voltage += analogRead(volt);
    delay(100);
  }
  voltage = voltage / 5;

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
   function to get the current
*/
float getCurrent() {
  float current = analogRead(curr);

  //current = current/14.9; //45 Amp board
  //IFinal = IRaw/7.4; //90 Amp board
  //IFinal = IRaw/3.7; //180 Amp board

  return 10;
}

