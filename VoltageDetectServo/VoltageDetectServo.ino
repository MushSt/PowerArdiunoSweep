
#include <Servo.h>
#include <LinkedList.h>

//angle of the Servo

//Constants:
const int start = 55;
const int finish = 180;
const int flatline = 125; //temp var for perfectly flat
const int stepSize = 1; //for calibration
const int sampleSize = 7; 
const int movementPause = 50;
const int deviceMovement = 250; //for large movements

//delay constant
const int delayTime = 1000;

//Servo objects
Servo tilt, pan;

//voltage and current sensors
int volt = A0;
int curr = A1;


LinkedList<float> powerData;
LinkedList<float> smoothedData;

void setup() {

  Serial.begin(9600);

  //attaches the servo objects to the corresponding pins
  tilt.attach(11);
  pan.attach(10);

  //calibrate the best position, sweep 2x for redundancy
  calibrate(pan);
  calibrate(tilt);
  powerData.clear();

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

//**********************CALIBRATE****************
/*
   does the initial calibration of the tilt servo

   Full sweep of the servo's range stored in a list
   move to optimal position
*/
void calibrate(Servo device) {
  powerData = LinkedList<float>();
  device.write(start);
  delay(deviceMovement);

  //get raw data and populate the list
  for(int i = start; i < finish; i += stepSize) {
    device.write(i);
    delay(movementPause);
    float pwr = calcPower();
    powerData.add(pwr);
    //Serial.println(pwr);
  }

  //smooth the raw data out to find the best position
  Serial.println("Smoothing data...");
  int bestIndex = smoothData(powerData);
  Serial.println("Best index is:");
  device.write(bestIndex + start); //optimum position
  delay(deviceMovement);
  
  Serial.println(bestIndex + start);
}

//**********************SMOOTH_DATA*******************
/*
 * Applies a mean filter to smooth out the data
 * returns the index of the peak
 */
int smoothData(LinkedList<float> rawData) {
  smoothedData = LinkedList<float>(); //to hold the smoothed data
  LinkedList<float> sample;
  
  Serial.print("Data size"); 
  Serial.println(rawData.size());
  
  int dataSize = rawData.size();
  int offset = sampleSize/2;

  //pad the smoothed data with 0s to represent the thrown away vals
  for(int i = 0; i < offset*2; ++i) {
    smoothedData.add(0);
  }

  //ignore the starting vals
  Serial.println(offset);
  Serial.println(dataSize-offset);
  for(int i = offset; i < dataSize-offset; ++i) {
    sample = LinkedList<float>(); //sublist to get an average
    for(int j = i - offset; j < i+offset; ++j) {
      //populate the sample
      float particle = rawData.get(j);
      sample.add(particle);
    }
    //sample has been populated, get the mean and place it into smoothed list
    float smoothed = getMean(sample);
    Serial.print("smoothed data point: " );
    Serial.print(smoothed);
    Serial.print(" ");
    Serial.println(i);
    smoothedData.add(i, smoothed);
    Serial.println(smoothedData.size());
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

  Serial.println(maxIndex);
  Serial.println(maxVal);
  return maxIndex + offset;
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

  /*
  for (int i = 0; i < 5; ++i) {
    voltage += analogRead(volt);
    delay(100);
  }
  voltage = voltage / 5;
  */
  voltage = analogRead(volt)*(5.0/1024.0);

  Serial.println(voltage);

  return voltage;
}

//**********************GETCURRENT*********************
/*
   function to get the current
*/
float getCurrent() {

  //Serial.print("current: ");
  //Serial.println(current);

  return 1;
}

