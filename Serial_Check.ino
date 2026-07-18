void finalcheck() {
  //checkvalue of all sensor

  Serial.print("LL: ");
  Serial.print(Lvalue);
  Serial.print("   ");
  Serial.print("LT: ");
  Serial.print(LSv);
  Serial.print("   ");
  Serial.print("FL: ");
  Serial.print(FLv);
  Serial.print("   ");
  Serial.print("FT: ");
  Serial.print(FSv);
  Serial.print("   ");
  Serial.print("FR: ");
  Serial.print(FRv);
  Serial.print("   ");
  Serial.print("RT: ");
  Serial.print(RSv);
  Serial.print("   ");
  Serial.print("RL: ");
  Serial.print(Rvalue);
  Serial.print("   ");
  Serial.print("AF: ");
  Serial.print(FV);
  Serial.print("   ");
  Serial.print("Senstate: ");
  Serial.print(senState);
  Serial.print("   ");
  Serial.print("IR Module: ");
  Serial.print(executePG);
    Serial.print("   ");
  Serial.print("Preset: ");
  Serial.print(currentpreset);
  Serial.println();
  delay(10);
}