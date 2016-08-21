#include <Servo.h>
#include <LinkedList.h>

/* Specs:
less fluctuating voltage readings
current readings should now show up

no need for LinkedList (use array!)
*/

//TODO:
/*
- change linkedlist to array where i can
- function to check up/check down


*/

// Constants:
const int start = 55;     //min angle for tilt
const int finish = 180;   //max angle for tilt/pan
const int flatline = 125; //the angle where top is flat
const int sample_size = 7;
const int lag_step = 4;
const int movement_pause = 100;
const int step_pause = 50;
const int loop_delay = 10000;

const float conversion_factor = (5.0/1024.0);

// Servo Objects:
Servo tilt, pan;

// Voltage and current sensors:
int voltage = A0;
int current = A1;

void setup() {
  Serial.begin(9600);

  //attach servo objects to the respective pins
  tilt.attach(11);
  pan.attach(10);

  //calibrate?
  calibrate(pan);
  calibrate(tilt);
  
}

//**** Main Loop ****
void loop() {
  //checkUp();
  //checkDown();
  delay(200000);
}

// Initial Calibration
void calibrate(Servo device) {
  //using a full sweep and 3 methods to smooth the data
  device.write(start);
  delay(movement_pause); //wait for servo to get into position

  float data[finish - start];

  //get raw data and populate the list
  int idx = 0;
  for(int i = start; i < finish; ++i) {
    device.write(i);
    delay(step_pause);

    float volts = getVoltage();
    data[idx] = volts;
    ++idx;

    //Optional printing power values
    //Serial.println(power);
  }

  //data smoothing part
  //choose from 3 smoothing methods: 0median(H), 1mean(E/M), 2threshold(E/M)
  int bestIndex = chooseSmoothingMethod(1, data, idx);

  device.write(bestIndex + start + lag_step);
  delay(movement_pause);
}

//**** check and movement after calibration ********
void checkUP(Servo device) {

}

void checkDown(Servo device) {

}

//**** Data Smoothing Functions *****
int meanSmoothing(float data[], int dataSize) {
  //throw away first (lag_step - 1) data samples
  int offset = sample_size/2;
  float smoothedData[dataSize - 2*offset];

  for(int i = offset; i < dataSize - offset; ++i) {
    float dataSample[sample_size];

    //populate the sample
    int index = 0;
    for(int j = i-offset; j <= i+offset; ++j) {
      dataSample[index++] = data[j];
    }

    float sampleMean = calcMean(dataSample);
    smoothedData[i-offset] = sampleMean;
  }
  int bestIndex = getMaxIndex(smoothedData, dataSize - 2*offset);

  return bestIndex + offset;
}

int medianSmoothing(float data[], int dataSize) {

}

int thresholdSmoothing(float data[], int dataSize) {
  
}


//**** Helper functions ********
float calcThreshold(float dataBlock[]) {
  //threshold algorithm: get average and standard dev
  //get a range based on that (mean +- range)
  //returns the range, cut out all things greater than the value range
}

float calcMean(float dataBlock[]) {
  //size is assumed to be equal to sample_size
  float sum = 0;
  int sampleSize = sizeof(dataBlock);

  for(int i = 0; i < sample_size; ++i) {
    sum += dataBlock[i];
  }  

  return sum/sample_size;
}

// calculates the median
float calcMedian(float dataBlock[]) {

}

// Gets the maximum index in an array
int getMaxIndex(float data[], int dataSize) {
  float maxVal = data[0];
  int maxIdx = 0;

  for(int i = 1; i < dataSize; ++i) {
    if(maxVal < data[i]) {
      maxVal = data[i];
      maxIdx = i;
    }
  }

  //Serial.println(maxIdx);
  return maxIdx;
}

//**** VOLTAGE READING FUNCTIONS *****

// get the voltage
float getVoltage() {
  float volts = analogRead(voltage);
  //volts = volts * conversion_factor;

  volts = volts * conversion_factor;

  //*optional printing
  //Serial.println(volts);

  return volts;
}


//**** WRAPPER METHOD ******
int chooseSmoothingMethod(int choice, float data[], int dataSize) {
  int bestIndex; 
  switch(choice) {
    case 0:
      return medianSmoothing(data, dataSize);
    case 1: 
      return meanSmoothing(data, dataSize);
    case 2:
      return thresholdSmoothing(data, dataSize);
    default:
      Serial.println("Bad input, using thresholdSmoothing as default");
      return thresholdSmoothing(data, dataSize);
  }
  return 0;
}

