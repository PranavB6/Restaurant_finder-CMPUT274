

/*
Aurthor: Pranavkumar Bodawala
Student ID:1500669

Aurthor: Kelly luc
Student ID: 1498694

This Arduino program is a chat program that encrypts and
decrypts text

*/

#include <Arduino.h>

// Pin that will determine whether its a server or a client based on the
// input
const int idPin = 13;

const uint32_t mod = 2147483647;
const uint32_t gen = 16807;
const int timeout = 1000;

// Pin to get random numbers
const int AnalogPin = 1;


// Initialized Arduino functionality and the pin modes
void setup(){
	init();
	pinMode(idPin, INPUT);
	pinMode(AnalogPin, INPUT);

	Serial.begin(9600);
	Serial3.begin(9600);
}


void printArray(char array[], int size){
	for (int i=0; i < size; i++){
		Serial.print(array[i]);
	}
}

// The function to calculate (a*b) mod m where a and b are 31 bit integers
// and return the value
uint32_t mulMod( uint32_t sub, uint32_t b){

	uint32_t m = mod;
	uint32_t value;

	uint32_t newb = b % m;
	uint32_t result = 0;

	for(int i = 0; i < 32; i++){
		if (sub == 0){ break;	}

		// Calculating sub into binary so we can get the 2 powers
		value = sub % 2;

		/* For debugging purposes
		Serial.print("loop: ");
		Serial.println(i);
		Serial.print(sub);
		Serial.println(" it was sub");
		Serial.println(value);
		Serial.println(" it was value");
		*/
		if (value == 1){
			result = (result + newb) % m;

			// For debugging purposes
			// Serial.print("result is: ");
			// Serial.println(result);
		}

		// For debugging purposes
		// Serial.print("power value is: ");
		// Serial.println(power);
		newb =  ( newb * 2)% m;

		sub = sub >> 1;
	}

	return result;
}


// Random number generator
uint32_t getRandomNumber() {
	uint32_t result = 0;
	int32_t randN;

	// Creates a 31 bit number
	for (int i =0; i < 32; i++){
		delay(50);
		randN = analogRead(AnalogPin);
		randN = randN & 1;

		result = result | (randN<<i);
	}

	return result;
}

// Given by Professor
/** Waits for a certain number of bytes on Serial3 or timeout
* @param nbytes: the number of bytes we want
* @param timeout: timeout period (ms); specifying a negative number
*                turns off timeouts (the function waits indefinitely
*                if timeouts are turned off).
* @return True if the required number of bytes have arrived.
*/
bool wait_on_serial3( uint8_t nbytes, long timeout ) {
	unsigned long deadline = millis() + timeout;//wraparound not a problem
	while (Serial3.available()<nbytes && (timeout<0 || millis()<deadline))
	{
		delay(1); // be nice, no busy loop
	}
	return Serial3.available()>=nbytes;
}

// Given by Professor
/** Writes an uint32_t to Serial3, starting from the least-significant
* and finishing with the most significant byte.
*/
void uint32_to_serial3(uint32_t num) {
	Serial3.write((char) (num >> 0));
	Serial3.write((char) (num >> 8));
	Serial3.write((char) (num >> 16));
	Serial3.write((char) (num >> 24));
}

// Given by Professor
/** Reads an uint32_t from Serial3, starting from the least-significant
* and finishing with the most significant byte.
*/
uint32_t uint32_from_serial3() {
	uint32_t num = 0;
	num = num | ((uint32_t) Serial3.read()) << 0;
	num = num | ((uint32_t) Serial3.read()) << 8;
	num = num | ((uint32_t) Serial3.read()) << 16;
	num = num | ((uint32_t) Serial3.read()) << 24;
	return num;
}

// Calculates a new SharedKey based on the old one
// Given by Professor
/** Implements the Park-Miller algorithm with 32 bit integer arithmetic
 * @return ((current_key * 48271)) mod (2^31 - 1);
 * This is linear congruential generator, based on the multiplicative
 * group of integers modulo m = 2^31 - 1.
 * The generator has a long period and it is relatively efficient.
 * Most importantly, the generator's modulus is not a power of two
 * (as is for the built-in rng),
 * hence the keys mod 2^{s} cannot be obtained
 * by using a key with s bits.
 * Based on:
 * http://www.firstpr.com.au/dsp/rand31/rand31-park-miller-carta.cc.txt
 */
uint32_t next_key(uint32_t current_key) {
	const uint32_t modulus = 0x7FFFFFFF; // 2^31-1
	const uint32_t consta = 48271;  // we use that consta<2^16
	uint32_t lo = consta*(current_key & 0xFFFF);
	uint32_t hi = consta*(current_key >> 16);
	lo += (hi & 0x7FFF)<<16;
	lo += hi>>15;
	if (lo > modulus) lo -= modulus;
	return lo;
}


int main() {
	setup();

	uint32_t SharedKey = 0;
	uint32_t SharedKeyClient;
	uint32_t SharedKeyServer;
	uint8_t UserInput; // User Input from Serial
	uint8_t SerialInput; // Input from Serial3


	//------------------------------- Instruction for Server--------------------------
	if(digitalRead(idPin) == HIGH) {
		Serial.flush();
		Serial3.flush();

		uint32_t serverKey; // server private Key
		uint32_t ckey; // client public Key
		uint32_t skey; // server public key
		bool gotInput; // if there is input (of the correct number of bytes) in serial3
		char recievedChar = ' ';

		// States of the State function machine
		enum State { SETUPKEY, LISTEN, FIRST_CR , ACK, CR, DONE };
		State currentState = SETUPKEY;

		Serial.println("I am the Server");

		while (true)
		{

			// Calculating Private Key, and Public Key
			if (currentState == SETUPKEY) {
				// Serial.print("Current State: ");
				// Serial.println("SETUPKEY");

				serverKey = getRandomNumber();
				Serial.print("Server Private Key: ");
				Serial.println(serverKey);
				skey = mulMod(serverKey, gen);
				Serial.print("Server Public Key: ");
				Serial.println(skey);

				currentState = LISTEN;
			}

			// Listening for input from serial3, if 5 bytes are recieved, move to next State
			// If 5 bytes not recieved or timedout, go back to listening
			if (currentState == LISTEN) {
				Serial.print("-");
				// Serial.print("Current State: ");
				// Serial.println("LISTEN");

				gotInput = wait_on_serial3(5, timeout);
				if (gotInput) {currentState = FIRST_CR;}
			}

			// If correct character revieved, extract the public key from the other 4 bytes
			// and send an Acknowledgement 'A' and move to next state
			// If the first byte is not a 'C', go back to listening
			if (currentState == FIRST_CR) {
				// Serial.print("Current State: ");
				// Serial.println("FIRST_CR");
				Serial.print("-");
				recievedChar = Serial3.read();

				// Sends new character in order to establish handshake
				if (recievedChar == 'C') {
					ckey = uint32_from_serial3();

					Serial3.write('A'); // NOTE: use write instead of print to "write" a byte
					uint32_to_serial3(skey);
					Serial3.flush();
					// Serial.println(ckey);


					currentState = ACK;
				}

				// wait for a response
				else { currentState = LISTEN; }
			}

			/*
			Wait for achnowledgment. If an 'A' is recieved, both arduinos have exchanged information
			and have Acknowledged each other, so handshake is complete, move to DONE State.
			If a 'C' is revieved, then the other 4 bytes are the public key from the other arduino
			so move to CR. If anything else is recieved, it did not recieve correct information, go back to listening
			*/
			if (currentState == ACK) {
				// use for debugging
				// Serial.print("Current State: ");
				// Serial.println("ACK");
				Serial.print("-");
				gotInput = wait_on_serial3(1, timeout);

				//  Checks input recieved
				if (gotInput) {

					recievedChar = Serial3.read();

					// If its 'A', handshake successful
					if (recievedChar == 'A') {
						currentState = DONE;
					}

					// If 'C' is inputed then extract key
					else if (recievedChar == 'C') {
						currentState = CR;
					}

					// Wait for a response if anything else is recieved
					else {currentState = LISTEN;}
				}
			}

			// 'C' has already been recieved and this arduino's public key has already
			// been sent, so we just need to extract the public key from the other arduino

			if (currentState == CR) {
				// Serial.print("Current State: ");
				// Serial.println("CR");
				Serial.print("-");

				// This State assumes, a 'C' has been read already
				ckey = uint32_from_serial3();


				// Acknowledges the key
				currentState = ACK;
			}

			// Handshake between two Arduino is successful
			// Calculate and Display the Shared Key
			if (currentState == DONE) {
				// Serial.print("Current State: ");
				// Serial.println("DONE");



				// Calcute and Displace Shared Key
				Serial.print("*");
				Serial.println();

				// For debugging
				// Serial.print("Recieved Key: ");
				// Serial.println(ckey);

				SharedKeyServer = mulMod(ckey, serverKey);
				Serial.println();
				Serial.print("Shared Key Server: ");
				Serial.println(SharedKeyServer);
				break;

			}
		}

	}
	// -------------------------Instructions for the Client-------------------------
	else if(digitalRead(idPin) == LOW) {

		Serial.flush();
		Serial3.flush();

		uint32_t ckey; // client Public Key
		uint32_t skey; // server Public key
		uint32_t clientKey; // client private key
		bool gotInput; // if there is input (with the correct amount of bytes) in serial3
		char recievedChar = ' '; // For debugging

		enum State { SETUPKEY, TALK, ACK, DONE };
		State currentState = SETUPKEY;

		Serial.println("I am the Client");


		while(true) {

			// Calculate client private key, client public key
			if (currentState == SETUPKEY) {
				// Serial.print("Current State: ");
				// Serial.println("SETUPKEY");

				clientKey = getRandomNumber();
				Serial.print("Client Private Key: ");
				Serial.println(clientKey);
				ckey = mulMod(clientKey, gen);
				Serial.print("Client Public Key: ");
				Serial.println(ckey);

				currentState = TALK;
			}

			// Sends 'C' to initiate handshake along with its public key
			if (currentState == TALK) {
				// Serial.print("Current State: ");
				// Serial.println("TALK");
				Serial.print("-");
				Serial3.write('C'); // NOTE: use write instead of print to "write" a byte
				uint32_to_serial3(ckey);

				// Checks if there input in the serial monitor
				gotInput = wait_on_serial3(5, timeout);

				// check if input is 'A'
				if (gotInput && (Serial3.read() == 'A')) {currentState = ACK;}
			}

			// If the server has Acknowledged our key, extract its public key from the message
			// and send an Acknowledgement to show we have revieved its Acknowledgement
			if (currentState == ACK) {

				// For debugging
				// Serial.print("Current State: ");
				// Serial.println("ACK");
				//
				// recievedChar = Serial.read();
				// Serial.print("Recieved: ");
				// Serial.println(recievedChar);
				skey = uint32_from_serial3();
				Serial.print("-");
				Serial3.write('A');
				currentState = DONE;
			}

			// Handshake between two Arduino is successful
			if (currentState == DONE) {
				// Serial.print("Current State: ");
				// Serial.println("DONE");

				Serial.print("*");
				Serial.println();

				// For debugging
				// Serial.print("Recieved Key: ");
				// Serial.println(skey);

				// Calculate and Display Shared Key
				SharedKeyClient = mulMod(skey, clientKey);
				Serial.println();
				Serial.print("Shared Key Client: ");
				Serial.println(SharedKeyClient);
				break;
			}
		}

	}

	Serial.println();
	Serial.print("--------------------- HANDSHAKE SUCCESSFUL -------------------");
	Serial.println();


	// Assigning sharedkeys to Arduinos
	if (digitalRead(idPin) == HIGH){ SharedKey = SharedKeyServer;}
	else												  	{ SharedKey = SharedKeyClient;}


	// --------------------------------CHAT PROGRAM-------------------------------
	while(true){

		// Checks if there input
		if (Serial.available() > 0){

			// Change key
			SharedKey = next_key(SharedKey);

			UserInput = Serial.read();

			// Encrypting the msg
			uint8_t EncryptMsg = UserInput ^ ((uint8_t)SharedKey);

			// Sends message
			Serial3.write(EncryptMsg);



			// Was enter pressed? Cretes new line
			if (UserInput == '\r') {
				Serial.print("\n\r");

				// Serial.println(SharedKey);
			}

			// prints characters
			else {
				Serial.print((char)UserInput);

				// Serial.println(SharedKey);
			}

		}

		// Checks if theres input
		if (Serial3.available() >0) {
			Serial3.flush();

			// Change key
			SharedKey = next_key(SharedKey);

			// Recieves inputs
			SerialInput = (int)Serial3.read();

			// Decrypts message
			uint8_t DecryptMsg = SerialInput ^ ((uint8_t)SharedKey);

			// Was enter pressed? Creates new line
			if (DecryptMsg == '\r') {
				Serial.print("\n\r");
				// Serial.println(SharedKey);
			}

			// Prints message on monitor
			else{
				Serial.print((char)DecryptMsg);

				// Serial.println(SharedKey);
			}

		}
	}

	return 0;
}
