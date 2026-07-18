void customMove1() {
  for (int fadeValue = 0; fadeValue <= 500; fadeValue += 1) {
    motorLeft.speed(160);
    motorRight.speed(160);
    delay(1);
  }
}

void customMove2() {
  for (int fadeValue = 30; fadeValue <= 200; fadeValue += 20) {
    motorLeft.speed(fadeValue);
    motorRight.speed(fadeValue * 0.55);
    delay(2);
  }
}

void customMove3() {
  motorLeft.speed(250);
  motorRight.speed(180);
}

void checkTimer() {
  // If flag is pulled high, start the timer
  if (flag) {
    if (!timerActive) {
      // Timer has not started, so start it
      timerStart = millis();
      timerActive = true;
    }

    // If 3 seconds have passed, set 'front' to 30
    if (millis() - timerStart >= 500) {
      front = 40;
      timerActive = false;  // Disable the timer after completing
    }
  } else {
    // If flag is low, reset the timer and keep 'front' unchanged
    timerActive = false;
    timerStart = 0;
    front = 0;
  }
}

void checkTimerStall() {
  // If flag is pulled high, start the timer
  if (stallflag) {
    if (!stalltimerActive) {
      // Timer has not started, so start it
      stalltimerStart = millis();
      stalltimerActive = true;
    }

    // If 3 seconds have passed, set 'front' to 30
    if (millis() - stalltimerStart >= 2000) {
      stallfront = 150;
      stalltimerActive = false;  // Disable the timer after completing
    }
  } else {
    // If flag is low, reset the timer and keep 'front' unchanged
    stalltimerActive = false;
    stalltimerStart = 0;
    stallfront = 0;
  }
}

void timedebounce() {
  // If debounce is pulled high, start the timer
  if (debounce) {
    if (!debounceActive) {
      // Timer has not started, so start it
      debounceStart = millis();
      debounceActive = true;
    }

    // If 10 milliseconds have passed, set 'debounceOk' to true
    if (millis() - debounceStart >= 25) {
      debounceOk = true;
      debounceActive = false;  // Disable the timer after completing
    }
  } else {
    // If debounce is low, reset the timer and keep 'debounceOk' false
    debounceActive = false;
    debounceStart = 0;
    debounceOk = false;
  }
}

void stop() {
  motorLeft.speed(0);  // stop
  motorRight.speed(0);
}

void Backward() {
  motorLeft.speed(-160);
  motorRight.speed(-160);
}

void Turnleft() {
  motorLeft.speed(-200);
  motorRight.speed(200);
}

void Turnright() {
  motorLeft.speed(200);
  motorRight.speed(-200);
}

void Forward() {
  motorLeft.speed(160);
  motorRight.speed(160);
}

void search() {
  motorLeft.speed(-80);
  motorRight.speed(-80);
}