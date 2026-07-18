void runpreset() {

  processpreset();

  switch (currentpreset) {
    case 0:  // No preset to run
      aclval = 50;
      break;

    case 1:  // Preset 1 to be ran
      for (int i = 0; i < 1; i++) {
        preset1();
      }
      break;

    case 2:  // Preset 2 to be ran
      for (int i = 0; i < 1; i++) {
        preset2();
      }
      break;

    case 3:  // Preset 3 to be ran
      for (int i = 0; i < 1; i++) {
        preset4();
      }
      break;
  }
}

void processpreset() {
  if (LV) {
    currentpreset = 1;
  } else if (FV) {
    currentpreset = 2;
  } else if (RV) {
    currentpreset = 3;
  }
}

void acceleration(int startValue, int maxValue, unsigned long incrementSpeed) {
  // Check if resetAccel is true to reset the counter
  if (resetAccel) {
    Accel = startValue;  // Reset to startValue
    resetAccel = false;  // Reset the flag after resetting
  }

  // Increment only if startAccel is true
  if (startAccel) {
    // Check if it's time to increment based on incrementSpeed
    if (millis() - lastIncrementTime >= incrementSpeed) {
      lastIncrementTime = millis();  // Update the last increment time

      if (Accel < maxValue) {
        Accel++;         // Increment the value
        readSensors();   // Continuously read sensors
        actOnSensors();  // Continuously act on sensors
      } else {
        Accel = maxValue;  // Cap the value at maximum
        readSensors();     // Continuously read sensors
        actOnSensors();    // Continuously act on sensors
      }
    }
  }
}

void resetflags() {
  resetAccel = true;
  startAccel = false;
  reset = false;
}

void mainprgm() {

  statusAutoRunning();  // solid red: match running
  startAccel = true;
  resetAccel = true;
  while (executePG == true) {
    acceleration(0, aclval, 2);
    checkTimer();
    checkTimerStall();

    readSensors();  // Calculate the sensor state
    timedebounce();
    //testbias();
    actOnSensors();  // Act based on the sensor state
    delay(3);
  }
}

void buttonmainprgm() {

  statusAutoRunning();  // solid red: match running
  startAccel = true;
  resetAccel = true;
  while (true) {
    acceleration(0, aclval, 3);
    checkTimer();

    readSensors();  // Calculate the sensor state
    timedebounce();
    actOnSensors();  // Act based on the sensor state
    delay(3);
  }
}

// Function to read sensors and update `senState`
void readSensors() {
  senState = 0;

  if (Lvalue < LINE_THRESHOLD) {
    senState = 1;
    debounce = true;
    if (debounceOk == true) {
      resetflags();
      stop();
      delay(20);
      Backward();
      delay(150);
      Turnright();
      delay(115);
    }
  } else if (Rvalue < LINE_THRESHOLD) {
    senState = 2;
    debounce = true;
    if (debounceOk == true) {
      resetflags();
      stop();
      delay(20);
      Backward();
      delay(150);
      Turnleft();
      delay(115);
    }
  } else if (Rvalue < LINE_THRESHOLD && Lvalue < LINE_THRESHOLD) {
    senState = 3;
    debounce = true;
    if (debounceOk == true) {
      resetflags();
      stop();
      delay(20);
      Backward();
      delay(150);
      Turnright();
      delay(110);
    }
  }

  if (senState == 0) {  // Only read proximity sensors if on black field
    if (LSv) senState += 8;
    if (FLv) senState += 16;
    if (FSv) senState += 32;
    if (FRv) senState += 64;
    if (RSv) senState += 128;
  }
}

void testbias() {
  if (senState == 0) {
    motorLeft.speed(Accel * flbias);
    motorRight.speed(Accel * frbias);
  }
}

// Function to act on sensor states
void actOnSensors() {

  delay(5);

  switch (senState) {
    case 0:  // No line sensors, no opponent
      if (!reset) {
        resetAccel = true;
        startAccel = true;
        reset = true;
      }
      flag = false;
      stallflag = false;
      motorLeft.speed(Accel * flbias);
      motorRight.speed(Accel * frbias);
      break;

    case 8:  // Left proximity sensor
      resetAccel = true;
      Turnleft();
      delay(turnDelay);
      startAccel = true;
      break;

    case 24:  // Error L + FL
    case 16:  // Front-left proximity sensor
      motorLeft.speed(30);
      motorRight.speed(175);
      break;

    case 32:  // Front proximity sensor

      flag = true;
      // motorLeft.speed((Accel + (front)) * flbias);
      // motorRight.speed((Accel + (front)) * frbias);
      motorLeft.speed(Accel * flbias);
      motorRight.speed(Accel * frbias);
      break;

    case 56:  // Error L + FL + F
    case 48:  // Front-left & front sensors
      motorLeft.speed(30);
      motorRight.speed(125);
      break;

    case 192:  //Error R + FR
    case 64:   // Front-right proximity sensor
      motorLeft.speed(175);
      motorRight.speed(30);
      break;

    case 224:  //Error R + FR + F
    case 96:   // Front-right & front sensors
      motorLeft.speed(125);
      motorRight.speed(30);
      break;

    case 128:  // Right proximity sensor
      resetAccel = true;
      Turnright();
      delay(turnDelay);
      startAccel = true;
      break;

    case 120:  //Error L + FL + F + FR
    case 136:  //Error L + R
    case 240:  //Error R + FR + F + FL
    case 248:  //Error L + FL + F + FR + R
    case 112:  // Front-left, front & front-right sensors
      flag = true;
      stallflag = true;
      motorLeft.speed((Accel - front * 0.8 + stallfront) * flbias);
      motorRight.speed((Accel - front * 0.8 + stallfront) * frbias);
      break;
  }
}