#include <Servo.h>

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

//instance variables
int check_direction;
boolean firstCheck;
boolean up;

void setup() {
  Serial.begin(9600);

  //attach servo objects to the respective pins
  tilt.attach(11);
  pan.attach(10);

  //calibrate?
  calibrate(pan);
  calibrate(tilt);

  check_direction = 1;
  firstCheck = true;
  up = true; //keep track of direction to minimize errors
}

//**** Main Loop ****
void loop() {
  if(firstCheck) {
    firstCheck = false;
    up = updatePosition(tilt, check_direction);
    if(!up) {
      check_direction = -check_direction;
      updatePosition(tilt, check_direction); //move back to original position
    }
  }
  else {
    if(!updatePosition(tilt, check_direction)) {
      check_direction = -check_direction;
      updatePosition(tilt, check_direction);
    }
  }

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
  int bestIndex = chooseSmoothingMethod(0, data, idx);

  device.write(bestIndex + start + lag_step);
  delay(movement_pause);
}

//**** check and movement after calibration ********
boolean updatePosition(Servo device, int direction) {
  //algorithm is to do sample_size of successive reads on voltage
  //and then use the median filter on that reading
  float currentVoltage = calcCurrVoltage();

  int currentAngle = device.read();
  device.write(currentAngle + direction); //increment the angle up or down by 1

  float updateVoltage = calcCurrVoltage();

  return updateVoltage > currentVoltage;
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

    float sampleMean = calcMedian(dataSample);
    smoothedData[i-offset] = sampleMean;
  }
  int bestIndex = getMaxIndex(smoothedData, dataSize - 2*offset);

  return bestIndex + offset;
}


int thresholdSmoothing(float data[], int dataSize) {
  //first split the data into blocks (sample_size)
  int numBlocks = dataSize/sample_size;
  float block[sample_size];

  for(int i = 0; i < numBlocks; ++i) {
    for(int j = 0; j < sample_size; ++j) {
      block[j] = data[i*sample_size + j];
    }
    float mean = calcMean(block);
    float range = calcThreshold(block, mean);

    //overwrite the ones that are over the range into the mean
    for(int j = 0; j < sample_size; ++j) {
      if(block[j] > mean+range || block[j] < mean-range) {
        block[j] = mean; //throw out the outliers by making them equal the average
        //overwrite original data
        data[i*sample_size+j] = block[j];
      }
    }

    return getMaxIndex(data, dataSize);
  }
}


//**** Helper functions ********
float calcCurrVoltage() {
  float voltageReadings[sample_size];
  for(int i = 0; i < sample_size; ++i) {
    voltageReadings[i] = getVoltage();
  }
  float currentVoltage = calcMedian(voltageReadings);
  return currentVoltage;
}

float calcThreshold(float dataBlock[], float mean) {
  //threshold algorithm: get average and standard dev
  //get a range based on that (mean +- range)
  //returns the range, cut out all things greater than the value range
  float multiplier = 0.3;

  float std = calcStandardDev(dataBlock, mean);

  return std*multiplier;
}

float calcStandardDev(float dataBlock[], float mean) {
  float sum = 0;
  for(int i = 0; i < sample_size; ++i) {
    sum += (dataBlock[i] - mean) * (dataBlock[i] - mean);
  }
  sum = sum / sample_size;
  sum = sqrt(sum);

  return sum;
}

float calcMean(float dataBlock[]) {
  //size is assumed to be equal to sample_size
  float sum = 0;

  for(int i = 0; i < sample_size; ++i) {
    sum += dataBlock[i];
  }

  return sum/sample_size;
}

// calculates the median
float calcMedian(float dataBlock[]) {
  //TODO, assume dataBlock size is sample_size
  float temp;
  // sorts array (ascending)
  for(int i = 0; i < sample_size-1; i++) {
      for(int j = i+1; j < sample_size; j++) {
          if(dataBlock[j] < dataBlock[i]) {
              // swap elements
              temp = dataBlock[i];
              dataBlock[i] = dataBlock[j];
              dataBlock[j] = temp;
          }
      }
  }

  if(sample_size%2==0) {
      return((dataBlock[sample_size/2] + dataBlock[sample_size/2 - 1]) / 2.0);
  }
  else {
      // else return the element in the middle
      return dataBlock[sample_size/2];
  }
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
