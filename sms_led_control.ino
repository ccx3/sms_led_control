/****************
 * See README for details
 ****************/
 
#include <SoftwareSerial.h>

// Configure software serial port
SoftwareSerial SIM900(7, 8);

// The output pin
int led = 4;

// used to control debugging output to Serial monitor
const int DEBUG = 1;

// Size of character buffer we will be using
const int MAX = 64;

// The input-buffer.
char msg[MAX];

// counter used to move through the input-buffer array
int count = 0;


void setup() {
  
  // set pin 4 to output-mode
  pinMode(led, OUTPUT);
  
  // initialise the input-buffer to NULLs
  clearMsg();
  
  // Arduino communicates with SIM900 GSM shield at a baud rate of 19200
  // Make sure that corresponds to the baud rate of your module
  SIM900.begin(19200);
  // For serial monitor
  Serial.begin(19200); 
  
  // Give GSM shield time to log on to network
  delay(20000);
  if(DEBUG) Serial.println("Ready to rock and roll!...");

  // AT command to set SIM900 to SMS mode
  SIM900.print("AT+CMGF=1\r");
  delay(100);
  
  // Set module to send SMS data to serial out upon receipt 
  SIM900.print("AT+CNMI=2,2,0,0,0\r");
  delay(100);
}

// re-set the input-buffer array and its counter
void clearMsg()
{
  for(int i = 0; i < MAX; i++)
  {
    msg[i] = NULL;
  }
  count = 0;
}

void loop() {

  count = 0;
  char c;
  
  // read in SMS
  while(SIM900.available() > 0) {
    c = SIM900.read(); // character at a time
    if(DEBUG) Serial.print(c);
    
    // character is lower-cased before adding to input-buffer
    msg[count++] = tolower(c);
    
    // stop reading at newline or full buffer
    if(c == 13 || count >= MAX)
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
  SIM900.println("AT + CMGS = \"+44XXXXXXXXXX\"");                                     // recipient's mobile number, in international format
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
    sprintf(statusMsg, "LED is %s", digitalRead(led) == LOW ? "OFF" : "ON");
    sendSMS(statusMsg);
  }
  if(strstr(msg, "l-on"))
  {
    digitalWrite(led, HIGH);
  }
  if(strstr(msg, "l-off"))
  {
    digitalWrite(led, LOW);
  }
  if(strstr(msg, "l-blink"))
  {
    for(int i = 0; i<5; i++)
    {
      digitalWrite(led, HIGH);
      delay(500);
      digitalWrite(led, LOW);
      delay(500);
    }
  }
  // initialise the input-buffer to NULLs
  clearMsg();
}
