#include "Arduino.h"
#include <SPI.h>
#include <RF24-STM.h>

/*
   NRF24L01     Arduino Uno  Arduino Mega  Blue_Pill(stm32f01C)
   ___________________________________________________
   VCC  RED    |    3.3v   |     3.3v       |      3.3v
   GND  BROWN  |    GND    |     GND        |      GND
   CSN  YELOW  |   Pin10   |     Pin10      | A4 NSS1   (PA4)  \
   CE   ORANGE |   Pin9    |     Pin9       | B0 digital(PB0)    |     NB
   SCK  GREEN  |   Pin13   |     Pin52      | A5 SCK1   (PA5)  |- All these pins
   MOSI BLUE   |   Pin11   |     Pin51      | A7 MOSI1  (PA7)  |  are 3.3v tolerant
   MISO PURPLE |   Pin12   |     Pin50      | A6 MISO1  (PA6)  /

   TOUJOURS ALIMENTER LE MODULE EN 3.3V

*/

RF24 radio(PB0, PA4); // CE, CSN

struct data {
  int x;
  float temp;
};

data myDataRx;

void setup() {
  Serial.begin(115200);
  Serial.println("THIS IS THE RECEIVER CODE - YOU NEED THE OTHER ARDUINO TO TRANSMIT");

  // Initie l'objet radio
  radio.begin();

  // Réglez la puissance de transmission au plus bas disponible pour éviter les problèmes liés à l'alimentation
  radio.setPALevel(RF24_PA_LOW);

  //Vitesse de transmission etre 250KBPS et 2MBPS, plus la vitesse est grande moins est la portee
  radio.setDataRate(RF24_1MBPS);

  // Eviter de mettre le channel entre 0 et 100 parce que le wifi peut creer des interferences
  radio.setChannel(104);
  // Donne une chance au receveur
  radio.setRetries(255, 5);

  // Ouvre un tube d'écriture et de lecture sur chaque radio, avec des adresses opposées
  radio.openReadingPipe(1, 0xB3B4B5B602);
  radio.openWritingPipe(0xB3B4B5B601);

  // Start the radio listening for data
  radio.startListening();
  radio.setAutoAck(true);

  //information de Debugging
  radio.printDetails();
}

void loop() {
  if (radio.available()) {
    // Allez lire les données et mettez-les dans cette variable
    //while (radio.available()) {
    radio.read(&myDataRx, sizeof(data));

    delay(100);
    Serial.println("RX: Received");

    // D'abord, arrêtez d'écouter pour que nous puissions parler
    radio.stopListening();

    Serial.print("Temperature: ");
    Serial.println(myDataRx.temp);

    // Change quelque chose
    myDataRx.temp++;

    //Augmentation de la valeur puis ré-envoie
    Serial.println("RX: Replied");

    if (!radio.write(&myDataRx, sizeof(data))) {
      Serial.println("RX: No ACK");
    } else {
      Serial.println("RX: ACK");
    }

    // Maintenant, reprenez l'écoute pour que nous attrapions les prochains paquets.
    radio.startListening();
  }
}
