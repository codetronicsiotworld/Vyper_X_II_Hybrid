void preset1() {
  for (int i = 0; i <= 2; i++) {  // Repeat 3 times
    readSensors();
    delay(10);
    motorLeft.speed(50 * LBias);
    motorRight.speed(50 * RBias);
    delay(80);
    readSensors();
    delay(10);
    motorLeft.speed(100 * LBias);
    motorRight.speed(100 * RBias);
    delay(120);
    readSensors();
    delay(10);

    stop();
    aclval = 0;
    front = 0;

    unsigned long startTime = millis();  // Record the start time

    while (millis() - startTime < 2000) {  // Check if 2 seconds have passed
      readSensors();                       // Continuously read sensors
      timedebounce();
      actOnSensors();  // Continuously act on sensors
      if (executePG == false) {
        break;
      }
    }
    if (executePG == false) {
      break;
    }
  }
  aclval = 65;
}

void preset2() {
  for (int i = 0; i <= 9; i++) {  // Repeat 10 times
    readSensors();
    timedebounce();
    delay(10);
    motorLeft.speed(50 * LBias);
    motorRight.speed(50 * RBias);
    delay(80);
    readSensors();
    timedebounce();
    delay(10);
    motorLeft.speed(100 * LBias);
    motorRight.speed(100 * RBias);
    delay(120);
    readSensors();
    timedebounce();
    delay(10);

    stop();
    aclval = 0;
    front = 0;

    unsigned long startTime = millis();  // Record the start time

    while (millis() - startTime < 2000) {  // Check if 2 seconds have passed
      readSensors();                       // Continuously read sensors
      timedebounce();
      actOnSensors();     // Continuously act on sensors
      checkTimerStall();  // Continuosly check for stuck opponenent
      if (executePG == false) {
        break;
      }
    }
    if (executePG == false) {
      break;
    }
  }
  aclval = 65;
}

void preset3() {
  motorLeft.speed(-200);
  motorRight.speed(200);
  delay(30);
  motorLeft.speed(50);
  motorRight.speed(50);
  delay(50);
  motorLeft.speed(180);
  motorRight.speed(180);
  delay(100);
  motorLeft.speed(250);
  motorRight.speed(250);
  delay(110);
  stop();
  delay(50);
  motorLeft.speed(200);
  motorRight.speed(0);
  delay(220);
  aclval = 60;
}

void preset4() {
  aclval = 40;
  front = 100;
}