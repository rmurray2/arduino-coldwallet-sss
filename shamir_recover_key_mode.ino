#include <EEPROM.h>
#include <bc-shamir.h>
#include "uECC.h"

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
        Serial.print("Entered user share ... ");
        Serial.println(receivedChars);
        newData = false;
    }
}


void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
Serial.println("Enter primary user share:");

}

void loop() {

 uint8_t val;
 uint8_t arduino_share[32];

  // read arduino share
  for(uint8_t i = 0; i < 32; i++)
{
      val = EEPROM.read(i);
      arduino_share[i] = val;
      //Serial.println(val);
}


  recvWithStartEndMarkers();

  uint8_t payload[32];
  uint8_t user_share[32];
  //uint8_t tx_hash[32];



  hexCharacterStringToBytes(payload, receivedChars);
  
  if (newData == true) { // payload has been received


  for (int j = 0; j < 32; j++){
    
    user_share[j] = payload[j];
    //tx_hash[j] = payload[j+32];
    //Serial.println(tx_hash[j]);

  }

  delay(2500);
  showNewData();
  

  /// RECOVERY
  char* output_shares[2];

  uint8_t secret_datao[32];
  uint8_t recovery_share_indexes[2] = {0,1};
  output_shares[0] = arduino_share;
  output_shares[1] = user_share;
  recover_secret(2, recovery_share_indexes, (const uint8_t **)output_shares, 32, secret_datao);
  //recover_secret(threshold, recovery_share_indexes, (const uint8_t **)shares, share_len, secret_datao);
  

  //prints hex private key 
  //Serial.println(convertBytesKeyToHexKey(secret_datao, 32));  

  //write recovered private key "above" the arduino share 
  uint8_t j;
   for(int i = 0; i < 32; i++)
{
  j = i+33;
  EEPROM.write(j, secret_datao[i]);
}
Serial.println("Recovered private key written to eeprom ...");

};

}

