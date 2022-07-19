//Welcome :)
//Here are a few notes to help you understand the code
//First off, the photo resistor is mounted on the servo and the servo scans its full range of motion searching for the greatest light source.
//There is a default time interval for each scan however, it can be changed by the user
//The button at the centre of the bread board can be used to trigger a scan.
//The other button, to the far-right or left is used to change the sensitivity level or scan interval.
//The sensitivity level is a fraction of the light intensity that must be attained in order to trigger a re-scan
//The default level is 2. That is, half of the old level. If the old level = 50 and the new level is less than 25 or greater than 75
//the system would trigger a re-scan.

#include <Arduino.h>
#include <Servo.h>

Servo myservo;

#define PUSH_BTN1 8   //this is the push button to invoke the servo
#define PUSH_BTN2 7   //this is the push button to change the sensitivity level or the scan interval.
#define LED_PIN 11
#define POS 0         //starting p9osition of the servo

int pos;     //pos of the servo
unsigned long lsTime;   //last time the environment was scanned
int lInt;     //light intensity at a particular point
int mInt;     //maximum light intensity found
int mPos;     //position at which the maximum light intensity was found
int pMint;    //previous maximum intensity

unsigned long lastDb1Time;
int oldState1;
int newState1;

unsigned long lastDb2Time;
int oldState2;
int newState2;

int dlay = 50;

unsigned long sInt = 50000;     //user scan interval
int sLvl = 2;                   //sensitivity level
String value;


//we always want to start the scan from zero so reset the servo to zero position.
void resetServo() {
	int spost = myservo.read();
	Serial.print("spost: ");
	Serial.println(spost);
	if (spost != 0) {
		for (int i = spost; i >= 0; i -= 1) {
			myservo.write(i);
			delay(15);
		}
	}
}

int ledB;
void scan() {
	resetServo();
	pMint = mInt;
	for (pos = 0; pos <= 180; pos += 1) {
		myservo.write(pos);
		lInt = analogRead(A0);
		ledB = map(lInt, 0, 1023, 0, 255);
		//Serial.print("ledB: ");
		//Serial.println(ledB);
		analogWrite(LED_PIN, ledB);
		if (lInt > mInt) {
			mInt = lInt;
			mPos = pos;
		}
		delay(15);
	}

	for (pos = 180; pos >= 0; pos -= 1) {
		myservo.write(pos);
		lInt = analogRead(A0);
		ledB = map(lInt, 0, 1023, 0, 255);
		analogWrite(LED_PIN, ledB);
		if (lInt > mInt) {
			mInt = lInt;
			mPos = pos;
		}
		delay(15);
	}

	lsTime = millis();      //record the last scan time
	String displayStr = String(mPos) + ":" + String(mInt);
	Serial.println(displayStr);
	Serial.println("*********************");
	myservo.write(mPos);    //turn servo to the position with highest intensity
	delay(10000);           //wait for 10s in case there is a light change and the environs has to be rescanned
} //scan()

//scan after the interval set by the user
void scanTime() {

	Serial.println("time interval is up, scanning again...");
	scan();
}//scanTime()

void setup() {
	Serial.begin(115200);

	pinMode(LED_PIN, OUTPUT);
	pinMode(PUSH_BTN1, INPUT_PULLUP);
	pinMode(PUSH_BTN2, INPUT_PULLUP);
	myservo.attach(12);  // attaches the servo on pin 10 to the servo object
	myservo.write(POS);  //set the servo to zero degrees
}

int oldInt;
unsigned long lastcTime;
void loop() {

	//scan if there is a pronounced change in light level
	int newInt = analogRead(A0);    //new Intensity
	ledB = map(newInt, 0, 1023, 0, 255);
	analogWrite(LED_PIN, ledB);

	if(millis() > lastcTime + 1000) {
		lastcTime = millis();

		if(newInt < (oldInt - (oldInt/sLvl)) || newInt > (oldInt + (oldInt/sLvl)) ) {
			Serial.print("oldInt: ");
			Serial.println(oldInt);

			Serial.print("newInt: ");
			Serial.println(newInt);

			Serial.println("light level has changed, scanning again...");
			scan();
		}
		oldInt = newInt;
	}

	if((millis() == lsTime + sInt))
		scanTime();

	//scan if the BTN_1 is pushed
	int state1 = !digitalRead(PUSH_BTN1);
	if(state1 != oldState1)     //button State has changed
		lastDb1Time = millis();

	if(millis() - lastDb1Time > dlay) {
		if(state1 != newState1)
			newState1 = state1;

		if(newState1 == HIGH) {
			Serial.println("the button was pushed, scanning...");

			scan();
			Serial.println("*********************");
		}
	}
	oldState1 = state1;

	//collect user input if BTN_2 is pushed
	int state2 = !digitalRead(PUSH_BTN2);
	if (state2 != oldState2)
		lastDb2Time = millis();

	if ((millis() - lastDb2Time ) > dlay) {
		if (state2 != newState2) {
			newState2 = state2;

			if (newState2 == HIGH) {
				while (Serial.available() > 0) { int rchar = Serial.read(); }  //make sure the buffer is emptyI
				Serial.println("What will you like to change?");
				Serial.println("Input 'I' to change the scan interval");
				Serial.println("Input 'L' to change the sensitivity level");
				while (Serial.available() == 0) { }
				char val = Serial.read();
				Serial.print("val: ");
				Serial.println(val);

				if (val == 'I') {
					Serial.println("Input the scan interval you want: ");
					while (Serial.available() == 0) { }
					if (Serial.available()) { int sInt = Serial.parseInt(); Serial.print("sInt: "); Serial.println(sInt); }
				}
				else if (val == 'L') {
					Serial.println("Input the sensitivity level you want: ");
					while (Serial.available() == 0) { }
					if (Serial.available()) { int sLvl = Serial.parseInt(); Serial.print("sLvl: "); Serial.println(sLvl); }
				}
			}//if newState2 == HIGH
		}//state2 != newState2
	}//(millis() - lastDb2Time ) > 5
	oldState2 = state2;
}



