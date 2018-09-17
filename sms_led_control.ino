#include "arduino_secrets.h"
/****************
 * See README for details
 ****************/
 
#include <SoftwareSerial.h>

// Configure software serial port
SoftwareSerial SIM900(7, 8);

// The output pin
#define L_LED 4

// used to control debugging output to Serial monitor
// #define L_DEBUG

// Size of character buffer we will be using
#define L_MAX 64

// taken from very useful discussion (and last suggestion)
// here https://stackoverflow.com/questions/9146395/reset-c-int-array-to-zero-the-fastest-way
#define ARRAY_SIZE(a) (sizeof (a) / sizeof *(a))

#define ZERO(a, n) do{\
   size_t i_ = 0, n_ = (n);\
   for (; i_ < n_; ++i_)\
     (a)[i_] = 0;\
} while (0)

#define ZERO_A(a) ZERO((a), ARRAY_SIZE(a))

// The input-buffer.
char msg[L_MAX];

// counter used to move through the input-buffer array
byte count = 0;

void setup() {
  
  // set pin 4 to output-mode
  pinMode(L_LED, OUTPUT);
  
  // initialise the input-buffer to NULLs
  clearBuffers();
  
  // Arduino communicates with SIM900 GSM shield at a baud rate of 19200
  // Make sure that corresponds to the baud rate of your module
  SIM900.begin(19200);
  Serial.begin(19200); 
  
  // Give GSM shield time to log on to network
  delay(1000);
  Serial.println("Ready!");

  // AT command to set SIM900 to SMS mode
  SIM900.print("AT+CMGF=1\r");
  delay(100);
  
  // Set module to send SMS data to serial out upon receipt
  SIM900.print("AT+CNMI=2,2,0,0,0\r");
  delay(100);
}

void loop() {

  count = 0;
  char c;
  
  // read in SMS
  while(SIM900.available() > 0) {
    c = SIM900.read(); // character at a time
    Serial.print(c);

    // character is lower-cased before adding to input-buffer
    msg[count++] = tolower(c);
    
    // stop reading at newline or full buffer
    if(c == 13 || count >= L_MAX)
    {
      do_response(); // react to SMS
      break;
    }
  }
}

void sendSMS(char message[])
{
  SIM900.print("AT+CMGF=1\r");                                                        // AT command to send SMS message
  delay(100);
  
  char phone_home[30];
  sprintf(phone_home, "AT + CMGS = \"%s\"", SECRET_MOBILE);
  SIM900.println(phone_home);
  delay(100);
  
  SIM900.println(message);        // message to send
  delay(100);
  SIM900.println((char)26);                       // End AT command with a ^Z, ASCII code 26
  delay(100); 
  SIM900.println();
  // delay(5000);                                     // give module time to send SMS
  // SIM900power();                                   // turn off module
}

// The effective function where 
// we just obey orders.
//
// All easily-understood really.
void do_response()
{
  if(strstr(msg, "l-sts"))
  {
    char statusMsg[15];
    sprintf(statusMsg, "LED is %s", digitalRead(L_LED) == LOW ? "OFF" : "ON");
    sendSMS(statusMsg);
  }
  else if(strstr(msg, "l-on"))
  {
    digitalWrite(L_LED, HIGH);
  }
  else if(strstr(msg, "l-off"))
  {
    digitalWrite(L_LED, LOW);
  }
  else if(strstr(msg, "l-blink"))
  {
    for(byte i = 0; i<5; i++)
    {
      digitalWrite(L_LED, HIGH);
      delay(500);
      digitalWrite(L_LED, LOW);
      delay(500);
    }
  }
  clearBuffers();
}

void clearBuffers()
{
  // clear all buffers & counters
  Serial.flush();
  SIM900.flush();
  ZERO_A(msg);
  count = 0;
}
