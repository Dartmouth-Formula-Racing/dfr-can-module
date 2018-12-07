void setup() {
  // put your setup code here, to run once:
  Serial.begin(1000000);       // start serial port

}

void loop() {
  // put your main code here, to run repeatedly:
  int readings[] = {};
  
  for (int i = 0; i < 4; i++) {
    readings[i] = analogRead(i);
  }
  
  char textBuffer[50];

  sprintf(textBuffer, "%04d %04d %04d %04d", analogRead(A0), analogRead(A1), analogRead(A2), analogRead(A3));

//  Serial.println(textBuffer);
  Serial.println(analogRead(0));
}
