/*
    cameraTrigger.ino
    Copyright (c) 2018 ItKindaWorks All right reserved.
    github.com/ItKindaWorks

    cameraTrigger.ino is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    cameraTrigger.ino is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with cameraTrigger.ino.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Metro.h"
#include <ESP8266WiFi.h>

const uint8_t AVG_SIZE = 20;
uint16_t threshold = 5;
uint16_t camDelay = 100;

const uint8_t FOCUS_PIN = D1;
const uint8_t SHUTTER_PIN = D2;


//1s reset time, manual reset
Metro resetTime = Metro (1000, false);



uint8_t storedVals[AVG_SIZE];

void setup() {
	WiFi.disconnect();
	Serial.begin(115200);
  pinMode(A0, INPUT);
	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(FOCUS_PIN, OUTPUT);
	pinMode(SHUTTER_PIN, OUTPUT);
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);
	delay(100);

	initAvg();
}

void loop(){

	if(checkForChange() && resetTime.check()){
		//Serial.println("Triggered");
		delayMicroseconds(camDelay * 1000);

		triggerCamera();

		//blink LED when the system is triggered
		digitalWrite(LED_BUILTIN, LOW);
		delay(20);
		digitalWrite(LED_BUILTIN, HIGH);
		delay(100);

		resetTime.reset();
	}

	handleSerial();
  delay(1);
}


//initialize the rolling avg array
void initAvg(){
	for(int i = 0; i < AVG_SIZE; i++){
		storedVals[i] = analogRead(A0);
		delay(20);
	}
}

bool checkForChange(){
	static int8_t index = 0;

	//if the index has reached the array limit, roll back over to 0
	if(index == AVG_SIZE){index = 0;}

	//store the latest value and print it out (for debugging)
	storedVals[index] = analogRead(A0);
	//Serial.println(storedVals[index]);

	//find the average
	int avg = 0;
	for(int i = 0; i < AVG_SIZE; i++){
		avg+=storedVals[i];
	}
	avg /= AVG_SIZE;

	//check for current reading being outside of the trigger threshold
	if(abs(storedVals[index] - avg) >  threshold){
		index++;
		return true;
	}

	//return false if there was no significant change
	index++;
	return false;
}


void triggerCamera(){

	digitalWrite(FOCUS_PIN, HIGH);
	delayMicroseconds(50 * 1000);
	digitalWrite(SHUTTER_PIN, HIGH);
	delayMicroseconds(50 * 1000);
	digitalWrite(FOCUS_PIN, LOW);
	digitalWrite(SHUTTER_PIN, LOW);
}


void handleSerial(){
	if(Serial.available()){

		//if the first char is 't' then update the threshold
		if(Serial.peek() == 't'){
			int input = Serial.parseInt();
			if(input > 0){
				threshold = input;
				Serial.printf("\nUpdated threshold to %d\n", threshold);
				delay(1000);
			}
		}

		//if the first char is 'c' then update the camera delay
		else if(Serial.peek() == 'c'){
			int input = Serial.parseInt();
			if(input > 0){
				camDelay = input;
				Serial.printf("\nUpdated Camera delay to %d\n", camDelay);
				delay(1000);
			}
		}

		//otherwise flush the garbage data from the serial line
		else{
			while(Serial.available()){
				Serial.read();
			}
		}
	}
}
