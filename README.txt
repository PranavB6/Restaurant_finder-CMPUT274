
Name: Pranavkumar Bodawala

StudentID: 1500669



Name: Kelly Luc
Student ID: 1498694 

NOTE: 
	* Sometimes random characters are being printed when serial-mon is being initialized
	* if both arduinos are not making a successful handshake restart the program

HOW TO RUN:

	1. load it in atom on the VM
	2. make sure that both Arduinos are connected into their ports
	3. in the terminal where the file is located use make upload on both Arduinos using "make upload-x"
	    where x is the port that the Arduino is connected to
	4. use "serial-mon-x" to open the display for the specific Arduino
	5. when both serial mons say "handshake successful," type away  
 
ACCESSORIES:
	* 2X Arduino Mega Board (AMG)
	
WIRING INSTRUCTIONS:

*NOTE* Arduino0 and Arduino1 will be use to differentiate the two arduinos when wiring

Communication between Arduinos:
	Arduino0 TX3 pin <--> Arduino1 RX3 pin
	Arduino1 TX3 pin <--> Arduino0 RX3 pin

	Arduino0 13 pin <--> Arduino1 GND
	Arduino1 13 pin <--> Arduino1 5v pin

