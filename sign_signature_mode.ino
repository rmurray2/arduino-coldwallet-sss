#include "uECC.h"
#include <EEPROM.h>

const struct uECC_Curve_t * curve = uECC_secp256k1();

const byte numChars = 66; // add two to length for start and end marker characters
char receivedChars[numChars];   // an array to store the received data

boolean newData = false;

byte byteArray[32] = {0};

byte nibble(char c)
{
  if (c >= '0' && c <= '9')
    return c - '0';

  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;

  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;

  return 0;  // Not a valid hexadecimal character
}


//overwrite recovered private key in eeprom 
void resetPkEeprom(){
  uint8_t j;
   for(int i = 0; i < 32; i++)
{
  j = i+33;
  EEPROM.write(j, 0);
};
};

String convertBytesKeyToHexKey(uint8_t bytesKey[], int size) {
  String str = String();

  for (byte i = 0; i < size; i = i + 1) {
    String int2Hex = String(bytesKey[i], HEX);
    if (int2Hex.length() ==  1) {
      int2Hex = "0" + int2Hex;
    }
    str.concat(int2Hex);
  }

  return str;
}

void hexCharacterStringToBytes(byte *byteArray, const char *hexString)
{
  bool oddLength = strlen(hexString) & 1;

  byte currentByte = 0;
  byte byteIndex = 0;

  for (byte charIndex = 0; charIndex < strlen(hexString); charIndex++)
  {
    bool oddCharIndex = charIndex & 1;

    if (oddLength)
    {
      // If the length is odd
      if (oddCharIndex)
      {
        // odd characters go in high nibble
        currentByte = nibble(hexString[charIndex]) << 4;
      }
      else
      {
        // Even characters go into low nibble
        currentByte |= nibble(hexString[charIndex]);
        byteArray[byteIndex++] = currentByte;
        currentByte = 0;
      }
    }
    else
    {
      // If the length is even
      if (!oddCharIndex)
      {
        // Odd characters go into the high nibble
        currentByte = nibble(hexString[charIndex]) << 4;
      }
      else
      {
        // Odd characters go into low nibble
        currentByte |= nibble(hexString[charIndex]);
        byteArray[byteIndex++] = currentByte;
        currentByte = 0;
      }
    }
  }
}

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
 
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

void showNewData() {
    if (newData == true) {
        Serial.print("Entered transaction hash ... ");
        Serial.println(receivedChars);
        newData = false;
    }
}


void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
Serial.println("Enter transaction or message hash: ");
}

void loop() {
  // put your main code here, to run repeatedly:
  
 uint8_t tx_hash[32];
 uint8_t signature[64];

 uint8_t val;
 uint8_t pk[32];
 uint8_t j;
  // read pk
  for(uint8_t i = 0; i < 32; i++)
{     j = i+33;
      val = EEPROM.read(j);
      pk[i] = val;
      //Serial.println(val);
};
//Serial.println(convertBytesKeyToHexKey(pk, 32));

recvWithStartEndMarkers(); // get tx_hash

hexCharacterStringToBytes(tx_hash, receivedChars);

//Serial.println("here");
 if (newData == true) {
//Serial.println("in");
uECC_sign(pk, tx_hash, 32, signature, uECC_secp256k1());
showNewData();
Serial.println("Signed Gnosis Transaction Hash ... 0x" + convertBytesKeyToHexKey(signature, 64)); // + "1b" or "1c"
//Serial.println("Signed Message Hash ... 0x" + convertBytesKeyToHexKey(signature, 64) + "1b");
resetPkEeprom();
Serial.println("Private key cleared from eeprom...");
 };

//delay(5000);

}
