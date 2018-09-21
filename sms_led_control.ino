#include "Arduino.h"

#include <SoftwareSerial.h>

// Configure software serial port
SoftwareSerial SIM900(7, 8);

// The output pin
#define L_LED 4

// used to control debugging output to Serial monitor
#define L_DEBUG

// Size of character buffer we will be using
#define L_MAX 64

// Control extra diagnostic output
#define VERBOSE

// taken from very useful discussion (and last suggestion)
// here https://stackoverflow.com/questions/9146395/reset-c-int-array-to-zero-the-fastest-way
#define ARRAY_SIZE(a) (sizeof (a) / sizeof *(a))

#define ZERO(a, n) do{\
		size_t i_ = 0, n_ = (n);\
		for (; i_ < n_; ++i_)\
		(a)[i_] = 0;\
} while (0)

#define ZERO_A(a) ZERO((a), ARRAY_SIZE(a))

// The input buffer.
char inBuffer[L_MAX];

// The call-number buffer
char callNumber[20];

// counter used to move through the input-buffer array
byte count = 0;

/**************************************************
 * Send an SMS - in response to a 'status' enquiry.
 * Send the reply to the caller.
 **************************************************/
void sendSMS(char message[])
{
	SIM900.print("AT+CMGF=1\r"); // AT command to send SMS message
	delay(100);

	char phone_home[30];
	sprintf(phone_home, "AT + CMGS = \"%s\"", callNumber);
	SIM900.println(phone_home);
	delay(100);

	SIM900.println(message);  // message to send
	delay(100);
	SIM900.println((char)26); // End AT command with a ^Z, ASCII code 26
	delay(100);
	SIM900.println();
}

/*****************************************************************
 * Software equivalent of pressing the GSM shield "power" button.
 * The effect is to toggle on/off which is actually less
 * than convenient.
 *
 * I tend to use the reset button myself, so the SIM9090
 * remains on and connected, ready for immediate use.
 *****************************************************************/
void SIM900power()
{
	digitalWrite(9, HIGH);
	delay(1000);
	digitalWrite(9, LOW);
	delay(5000);
}

/************************************************
 * Clear the input buffer and reset the counter.
 *
 * NOTE DON'T reset the callNumber in here -
 * it is not always reset at the same time.
 ************************************************/
void clearInBuffer()
{
	// clear all buffers & counters
	ZERO_A(inBuffer);
	count = 0;
}

/***********************************
 * Responds to a known command else
 * it does nothing.
 ***********************************/
//
void do_response()
{

#ifdef VERBOSE
	// Done here and not in loop() so that we
	// do not break up and destroy the readability of
	// the incoming stream

	Serial.print("Caller : ");
	Serial.print(callNumber);
	Serial.print(" Message: ");
	Serial.println(inBuffer);
#endif
	if(strstr(inBuffer, "status"))
	{
		char statusMsg[15];
		sprintf(statusMsg, "LED is %s", digitalRead(L_LED) == LOW ? "OFF" : "ON");
		sendSMS(statusMsg);
	}
	else if(strstr(inBuffer, "on"))
	{
		digitalWrite(L_LED, HIGH);
	}
	else if(strstr(inBuffer, "off"))
	{
		digitalWrite(L_LED, LOW);
	}
	else if(strstr(inBuffer, "blink"))
	{
		for(byte i = 0; i<5; i++)
		{
			digitalWrite(L_LED, HIGH);
			delay(500);
			digitalWrite(L_LED, LOW);
			delay(500);
		}
	}
}

/*********************************
 * Initialise and configure once.
 *********************************/
void setup() {

	// set pin 4 to output-mode
	pinMode(L_LED, OUTPUT);

	// initialise the input-buffer to NULLs
	clearInBuffer();

	// Arduino communicates with SIM900 GSM shield at a baud rate of 19200
	// Make sure that corresponds to the baud rate of your module
	Serial.begin(19200);
	SIM900.begin(19200);

#ifdef POWER_UP
	// Give GSM shield time to log on to network
	SIM900power();
	delay(20000);
#else
	delay(1000);
#endif

	// AT command to set SIM900 to SMS mode
	SIM900.print("AT+CMGF=1\r");
	delay(100);

	// I have no idea what this does. It had no effect on the working program.
	// Set module to send SMS data to serial out upon receipt
	SIM900.print("AT+CNMI=2,2,0,0,0\r");
	delay(100);
}

/*************************************************************
 * Each invocation of loop()
 *
 * 1. Look for the next newline.
 * Do not proceed until we find one.
 * This just avoids any confusion about where
 * in a line we are.
 *
 * 2. Look for an incoming SMS.
 * This section copes with any extra empty
 * lines. Here, we look for a starting
 * string of:
 *
 * +CMT: "
 *
 * which is the start of an incoming SMS.
 *
 * Example string for status-query SMS:
 * +CMT: "+447012345678","","18/09/21,13:39:17+04"
 * Status
 *
 * 3. Extract the mobile number that sent the command or query
 * by reading the incoming data up to the next ".
 * Assume it is the mobile number with no
 * newlines or anything. Once it has been read in, discard
 * the rest of the line. We do not need it.
 *
 * 4. The next line will have the message itself.
 * Extract it. Now we have all we need to respond.
 *
 * 5. Respond to all known commands and queries.
 * Everything else is ignored. Note that only the
 * status query NEEDS the mobile number, the other
 * commands just set the digital pin to HIGH or LOW.
 */
void loop() {

	char c;
	bool sms = false;

	/***************************************
	 * (Don't start until something arrives)
	 ***************************************/
	if(SIM900.available() <= 0)
	{
		delay(100);
		return;
	}

	/*********************************
	 * 1. Look for a newline.
	 *********************************/
	sms = false;
	while((SIM900.available() > 0))
	{
		c = SIM900.read(); // character at a time
		Serial.print(c);
		if(c == '\n')
		{
			break;
		}
	}

	/**********************************
	 * 2. Do we have an incoming SMS?
	 **********************************/
	sms = false;
	clearInBuffer();
	while(
			(SIM900.available() > 0)
			&&
			(sms == false)
	)
	{
		c = SIM900.read();
		Serial.print(c);
		if(isPrintable(c))
		{
			inBuffer[count++] = tolower(c);
		}

		if(
				(count == 7)
				&&
				(strcmp("+cmt: \"", inBuffer) == 0)
		)
		{
			sms = true;
		}
	}

	// Not an SMS. Start looking again.
	if(sms == false)
	{
		return;
	}

	/**********************************
	 * 3. Extract the mobile number
	 **********************************/
	count = 0;
	ZERO_A(callNumber);

	while(SIM900.available() > 0)
	{
		c = SIM900.read();
		Serial.print(c);
		if(c == '\"')
		{
			break;
		}
		else
		{
			callNumber[count++] = c;
		}
	}

	// consume the rest of this line and discard
	while(SIM900.available() > 0)
	{
		c = SIM900.read();
		Serial.print(c);
		if(c == '\n')
		{
			break;
		}
	}

	/*********************************
	 * 4. Extract the message itself.
	 *********************************/
	clearInBuffer();
	while(
			(SIM900.available() > 0)
	)
	{
		c = SIM900.read();
		Serial.print(c);
		if(isPrintable(c))
		{
			inBuffer[count++] = tolower(c);
		}
		else
		{
			break; // assume end of usable message
		}
	}

	/*************************
	 * 5. Process the message
	 *************************/
	if(
			(strlen(callNumber) > 0)
			&&
			(strlen(inBuffer) > 0)
	)
	{
		do_response();
	}

	// All done here. Let loop() start over.
}
