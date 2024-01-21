#include <bc-shamir.h>

#include <stdio.h>
#include <string.h>
#include "Entropy.h"
#include <EEPROM.h>

#define randomSeed(s) srandom(s)


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


char* data_to_hex(uint8_t* in, size_t insz)
{
  char* out = malloc(insz * 2 + 1);
  uint8_t* pin = in;
  const char * hex = "0123456789abcdef";
  char* pout = out;
  for(; pin < in + insz; pout += 2, pin++){
    pout[0] = hex[(*pin>>4) & 0xF];
    pout[1] = hex[ *pin     & 0xF];
  }
  pout[0] = 0;
  return out;
}



void genrandom(uint8_t *buf, size_t count, void* ctx) {
  uint8_t b = 0;
  int randNumber;
  for(int i = 0; i < count; i++) {
    randNumber = random(2000000); 
    buf[i] = randNumber;
  }
}


void setup() {
  Serial.begin(9600);
  Entropy.initialize();
  randomSeed(Entropy.random());
}

void loop() {
  int val;
  
  // Read private key from eeprom
  uint8_t pk[32];
  for(int i = 0; i < 32; i++)
{
  val = EEPROM.read(i);
  pk[i] = val;
};
delay(500);
 Serial.println("Splitting key...");
 delay(500);
// Split private key into shares
  uint8_t share_count = 3;
  uint8_t threshold = 2;


  size_t result_len = share_count * 32;
  uint8_t result_data[result_len];

char* output_shares[share_count];

split_secret(threshold, share_count, pk, 32, result_data, NULL, genrandom);


delay(1000);
  for(int i = 0; i < share_count; i++) {
    size_t offset = i * 32;
    output_shares[i] = data_to_hex(result_data + offset, 32);
  }


for(int i = 1; i < sizeof(output_shares); i++){

    if (i==1) {
    Serial.println("Primary user share (use this to recover private key): ");
    Serial.println(output_shares[i]);
    delay(2500);
    };
    Serial.println("");
    if (i==2){
    Serial.println("Backup user share (store this as a backup): ");
    Serial.println(output_shares[i]);
    delay(2500);    

    }
};
//  Write zeroth share to eeprom
Serial.println("Writing Aduino share to eeprom...");
Serial.println("Private key is now cleared from eeprom...");
 for(int i = 0; i < 32; i++)
{
  EEPROM.write(i, result_data[i]);
}
Serial.print("Done");
delay(250000);

//TODO encrypt share using permenent pin, display share



}