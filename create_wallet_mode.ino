#include <stdio.h>
#include <string.h>

#include "uECC.h"
#include "Entropy.h"
#include "ethers.h"
#include <EEPROM.h>

#define randomSeed(s) srandom(s)

const struct uECC_Curve_t * curve = uECC_secp256k1();
String walletAddress = "N/A";
String privateKey = "N/A";
int memaddr = 0;

static int RNG(uint8_t *dest, unsigned size) {

    uint8_t seed_value;
    while (size) {
      // The random method returns an unsigned 32-bit value, which can be cast as a
      // signed value if needed.  The function will wait until sufficient entropy is
      // available to return, which could cause delays of up to approximately 500ms
      seed_value = Entropy.randomByte();
      *dest = seed_value;
      ++dest;
      --size;
    }
    return 1;
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

void generateWallet(uint8_t *pk) {
 
  uint8_t pub[64];
  uint8_t *address = (uint8_t*)malloc(20);
  uECC_make_key(pub, pk, curve);

  uint8_t hashed[32];
  ethers_keccak256(pub, 64, hashed);
  memcpy(address, &hashed[12], 20);
  walletAddress =  "0x" + String( convertBytesKeyToHexKey(address, 20)); //20
  privateKey = String(convertBytesKeyToHexKey(pk, 32));
  delay(1500);
  Serial.println("Public address: " + walletAddress);
  delay(1500);
  //Serial.println("Private Key: "  + privateKey);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  uECC_set_rng(&RNG);
  Entropy.initialize();
  randomSeed(Entropy.random());
}

void loop() {
    uint8_t pk[32];
    generateWallet(pk);
    
  //TODO ask for pin and encrypt pk written to eeprom
    for(int i = 0; i < 32; i++)
  {
     EEPROM.write(i, pk[i]);
  };
    delay(250000);
    Serial.println("Private key is now written into eeprom ...");

/*
//encrypt with 
   for(int i = 0; i < 32; i++)
{
  pk_encrypted[i] = pk[i] + pin;
}
*/
}
