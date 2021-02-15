/*  Modified by Forrest Lee Erickson
 *  Date: 20210203
 *  Removing old style HardwareSerial.h code.
 *  Add command F to force freeze to stop capture and report.
 *  Add command C to sequence Start, Stop and report.
 *  20210206 Shortened some strings so that DEBUG can be set with out running out of dunamic memory
 *  20210208 Connect signal in to A0. Connect signal in trigger D7. Set comparitor treshold to band gap. This captures data from O scope reference. 
 *  20210209 Tested with 1KHz 2Vp-p scope sqaure wave into A0. Same signal is trigger on D7. Set Comparator reference to 1.1 Band Gap ref.
 *  Is acble to trigger and capture data when prescaler is set to 
 *  This sketch captures data for prescale values of 128, 64, and 32. I may have had
 *  intermitent capture at 16 but can not reproduce.  
 *  The maxium sample rate is a burst of (16MHZ / 32)  /13 = 38.4KSps.
 *  
 */


//-----------------------------------------------------------------------------
// Girino.ino
//-----------------------------------------------------------------------------
// Copyright 2012 Cristiano Lino Fontana
//
// This file is part of Girino.
//
//	Girino is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	Girino is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with Girino.  If not, see <http://www.gnu.org/licenses/>.
//
//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include "Girino.h"

//-----------------------------------------------------------------------------
// Global Constants
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

volatile  boolean wait;
         uint16_t waitDuration;
volatile uint16_t stopIndex;
volatile uint16_t ADCCounter;
volatile  uint8_t ADCBuffer[ADCBUFFERSIZE];
volatile  boolean freeze;

          uint8_t prescaler;
          uint8_t triggerEvent;
          uint8_t threshold;

             char commandBuffer[COMBUFFERSIZE+1];

//-----------------------------------------------------------------------------
// Main routines
//-----------------------------------------------------------------------------
//
// The setup function initializes registers.
//
void setup (void) {		// Setup of the microcontroller
	// Open serial port with a baud rate of BAUDRATE b/s
	Serial.begin(BAUDRATE);
  pinMode(samplePin, OUTPUT);      // set the second LED pin out
  pinMode(samplePinGND, OUTPUT);      // set the second LED pin GND pin out
  pinMode(triggerPin, OUTPUT);      // set the third LED pin out
  pinMode(triggerPinGND, OUTPUT);      // set the third LED pin GND pin out
  
  digitalWrite(samplePinGND, LOW);
  digitalWrite(samplePin, LOW);
  digitalWrite(triggerPinGND, LOW);
  digitalWrite(triggerPin, LOW);

  

	dshow("# setup()");
	// Clear buffers
	memset( (void *)ADCBuffer, 0, sizeof(ADCBuffer) );
	memset( (void *)commandBuffer, 0, sizeof(commandBuffer) );
	ADCCounter = 0;
	wait = false;
	waitDuration = ADCBUFFERSIZE - 32;
	stopIndex = -1;
	freeze = false;

  
	triggerEvent = 3;   //Rising edge
	threshold = 127;

//  setVoltageReference(0); // Set for AREF
  setVoltageReference(1); // Set for AVCC
  setTriggerEvent(triggerEvent); //
  setADCPrescaler(prescaler);

	// Activate interrupts
	sei();

	initPins();

//  prescaler = 128;
  prescaler = 32;
//  prescaler = 16;
  initADC();
  setADCPrescaler(prescaler);

// An alternate way from: https://forum.arduino.cc/index.php?topic=17450.0
  ACSR =
 (0<<ACD) |   // Analog Comparator: Enabled
 (0<<ACBG) |   // Analog Comparator Bandgap Select: AIN0 is applied to the positive input
 (1<<ACO) |   // Analog Comparator Output: On
 (1<<ACI) |   // Analog Comparator Interrupt Flag: Clear Pending Interrupt
 (1<<ACIE) |   // Analog Comparator Interrupt: Enabled
 (0<<ACIC) |   // Analog Comparator Input Capture: Disabled
 (1<<ACIS1) | (1<ACIS0);   // Analog Comparator Interrupt Mode: Comparator Interrupt on Rising Output Edge
 stopAnalogComparator();
 

	Serial.println("Girino ready");
	//printStatus();
}

void loop (void) {
//	dprint(ADCCounter);
//	dprint(stopIndex);
//	dprint(wait);
//	dprint(freeze);

//  #if DEBUG == 1
  #if 0
	Serial.println( ADCSRA, BIN );
	Serial.println( ADCSRB, BIN );
	#endif

	// If freeze flag is set, then it is time to send the buffer to the serial port
	if ( freeze )
	{
//    Serial.println("Write buffer.");
    Serial.println("\n\n *****************");
		dshow("# Frozen");
		// Send buffer through serial port in the right order
//		Serial.print("Buffer: ");
// Binary write   
//    Serial.write( ADCBuffer, ADCBUFFERSIZE );

//int ii=0;
//    for (ii =((uint8_t *)ADCBuffer + ADCCounter); ii < (ADCBUFFERSIZE - ADCCounter); ii++) {
//    for (ii =0; ii < ADCBUFFERSIZE; ii++) {

//Human Fiendly format
    for (int ii = 0; ii <= ADCBUFFERSIZE; ii++) {
          Serial.print( ADCBuffer[ii]); 
          Serial.print(", ");     
    }
    Serial.println("");
		Serial.println("End of Buffer");

		//Serial.write( (uint8_t *)ADCBuffer + ADCCounter, ADCBUFFERSIZE - ADCCounter );
		//Serial.write( (uint8_t *)ADCBuffer, ADCCounter );

		// Turn off errorPin
		//digitalWrite( errorPin, LOW );
//FLE		cbi(PORTB, PORTB5);

		wait = false;
		freeze = false;

		// Clear buffer
		//memset( (void *)ADCBuffer, 0, sizeof(ADCBuffer) );

		//startADC();
		// Let the ADC fill the buffer a little bit
		//delay(1);
		//startAnalogComparator();

//		#if DEBUG == 1
//		delay(3000);
//		#endif
	}

	if ( Serial.available() > 0 ) {
		// Read the incoming byte
		char theChar = Serial.read();
			// Parse character
		switch (theChar) {

      case 'c':     // 'c' Capture (to start, stop, freeze to read data)
      case 'C': {
        // Wait for COMMANDDELAY ms to be sure that the Serial buffer is filled
        delay(COMMANDDELAY);
        fillBuffer( commandBuffer, COMBUFFERSIZE );
        // Start
        Serial.println("Started");
        startADC();
        // Let the ADC fill the buffer a little bit
        delay(100);
        startAnalogComparator();
        //Stop
        Serial.println("Stopped");
        stopAnalogComparator();
        stopADC();        
         // Set freeze
        freeze = true;
        Serial.println("Freeze");
        }
        break;
        
      case 'f':     // 'f' to set freeze and force read data
      case 'F': {
        // Wait for COMMANDDELAY ms to be sure that the Serial buffer is filled
        delay(COMMANDDELAY);
        fillBuffer( commandBuffer, COMBUFFERSIZE );
        // Set freeze
        freeze = true;
        Serial.println("Freeze");
        }
        break;
        
			case 's':			// 's' for starting ADC conversions        
				Serial.println("Started");
				// Clear buffer
				memset( (void *)ADCBuffer, 0, sizeof(ADCBuffer) );
				startADC();
				// Let the ADC fill the buffer a little bit
				delay(1);       
				startAnalogComparator();
				break;
        
			case 'S':			// 'S' for stopping ADC conversions
				Serial.println("Stopped");
				stopAnalogComparator();
				stopADC();
				break;
        
			case 'p':			// 'p' for new prescaler setting
			case 'P': {
				// Wait for COMMANDDELAY ms to be sure that the Serial buffer is filled
				delay(COMMANDDELAY);
				fillBuffer( commandBuffer, COMBUFFERSIZE );

				// Convert buffer to integer
				uint8_t newP = atoi( commandBuffer );

				// Display moving status indicator
				Serial.print("Prescaler: ");
				Serial.println(newP);

				prescaler = newP;
				setADCPrescaler(newP);
				}
				break;

			case 'r':			// 'r' for new voltage reference setting
			case 'R': {
				// Wait for COMMANDDELAY ms to be sure that the Serial buffer is filled
				delay(COMMANDDELAY);

				fillBuffer( commandBuffer, COMBUFFERSIZE );

				// Convert buffer to integer
				uint8_t newR = atoi( commandBuffer );

				// Display moving status indicator
				Serial.print("Vref: ");
				Serial.println(newR);

				setVoltageReference(newR);
				}
				break;

			case 'e':			// 'e' for new trigger event setting
			case 'E': {
				// Wait for COMMANDDELAY ms to be sure that the Serial buffer is filled
				delay(COMMANDDELAY);

				fillBuffer( commandBuffer, COMBUFFERSIZE );

				// Convert buffer to integer
				uint8_t newE = atoi( commandBuffer );

				// Display moving status indicator
				Serial.print("Trig: ");
				Serial.println(newE);

				triggerEvent = newE;
				setTriggerEvent(newE);
				}
				break;

			case 'w':			// 'w' for new wait setting
			case 'W': {
				// Wait for COMMANDDELAY ms to be sure that the Serial buffer is filled
				delay(COMMANDDELAY);

				fillBuffer( commandBuffer, COMBUFFERSIZE );

				// Convert buffer to integer
				uint8_t newW = atoi( commandBuffer );

				// Display moving status indicator
				Serial.print("Wait: ");
				Serial.println(newW);

				waitDuration = newW;
				}
				break;

			case 't':			// 'w' for new threshold setting
			case 'T': {
				// Wait for COMMANDDELAY ms to be sure that the Serial buffer is filled
				delay(COMMANDDELAY);

				fillBuffer( commandBuffer, COMBUFFERSIZE );

				// Convert buffer to integer
				uint8_t newT = atoi( commandBuffer );

				// Display moving status indicator
				Serial.print("Threshold: ");
				Serial.println(newT);

				threshold = newT;
				analogWrite( thresholdPin, threshold );
				}
				break;

			case 'd':			// 'd' for display status
			case 'D':
        delay(COMMANDDELAY);
        fillBuffer( commandBuffer, COMBUFFERSIZE );
				printStatus();
				break;

			default:
				// Display error message
				Serial.print("ERROR: Command not found, it was: ");
				Serial.println(theChar,HEX);
				error();
		}
	}
}
