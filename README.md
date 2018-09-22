An Arduino Uno project demonstrating how to

1. control an LED via received SMS messages
2. reply to a status-query & send a status message to the inquirer's phone

Note: I used an LED, but you can replace that with other peripherals you want
to control via the pin on the Arduino.

I cribbed, used, and abused code from lots of sources while learning to do this,
including articles and tutorials found on:

* seeedstudio.com
* randomnerdtutorials.com
* instructables.com

and probably other places too. This final form shows clearly how I parsed the 
incoming message to extract the mobile number. That took the longest.

I don't like dense, unreadable code, so what you see is no doubt verbose, but
also readable and well-commented. I hope it is helpful.

Bits used:

* Arduino Uno R3
* Linksprite SIM900 GPRS/GSM shield (an old one given to me)
* An activated SIM card (unlocked, or you will need to change the source-code).
* 1 LED
* 1 220ohm resistor
* breadboard
* wires

Connect pin4 to resistor end #1, resistor end #2 to led long leg,
led short leg to GND. Jumpers both on SWSerial, and I powered the shield from 
an external supply and so moved the selector switch, next to the shield's 
power jack, to Ext. See [photo](./SIM900.jpg)

NOTE #1: This is only a DEMO PROJECT. For instance, there is no protection
against misuse by 3rd parties who know the phone number of the SIM card.

NOTE #2: Sending an SMS messages to operate the LED will of course eat into your phone's allowance. Sending a status query and getting a reply will use 1 message from your phone's allowance, and 1 message from the allowance that the SIM in the 
shield has.

NOTE #3: My editor uses a tabwith=4. Github seems to use 8. You can see the 
code as I do by appending ?ts=4 to the file URL.  i.e. 
https://github.com/ccx3/sms_led_control/blob/master/sms_led_control.ino?ts=4
