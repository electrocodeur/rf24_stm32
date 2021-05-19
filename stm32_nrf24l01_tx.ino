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
RF24 radio(PB0, PA4); //CE, CSN

#define axe_x         PA3

struct data {
  int x;
  float temp;
};

data myData;

void setup() {

  Serial.begin(115200);
  Serial.println("TX: THIS IS THE TRANSMITTER CODE - YOU NEED THE OTHER ARDIUNO TO SEND BACK A RESPONSE");
  pinMode(axe_x, INPUT);
  // Initie l'objet radio
  radio.begin();

  // Réglez la puissance de transmission au plus bas disponible pour éviter les problèmes liés à l'alimentation
  radio.setPALevel(RF24_PA_LOW);

  //Vitesse de transmission etre 250KBPS et 2MBPS, plus la vitesse est grande moins est la portee
  radio.setDataRate(RF24_1MBPS);

  // Eviter de mettre le channel entre 0 et 100 parce que le wifi peut creer des interferences
  radio.setChannel(104);

  // Donne une chance au receveur
  radio.setRetries(200, 50);

  // Ouvre un tube d'écriture et de lecture sur chaque radio, avec des adresses opposées
  radio.openReadingPipe(1, 0xB3B4B5B601);
  radio.openWritingPipe(0xB3B4B5B602);

  radio.setAutoAck(true);

  //information de Debugging
  radio.printDetails();
}

void loop() {
  int valx = analogRead(axe_x);
  float temp = 20.12;
  myData.x = valx;
  myData.temp = temp;
  radio.stopListening();

  Serial.println("TX: Sent out data");
  if (!radio.write(&myData, sizeof(myData))) {
    Serial.println("TX: No ACK");
  } else {
    Serial.println("TX: ACK");
  }

  // Maintenant lis une response
  radio.startListening();

  // Mais nous n'écouterons pas longtemps
  unsigned long started_waiting_at = millis();

  // Boucle ici jusqu'à ce que nous obtenions une indication que certaines données sont prêtes à être lues (ou que nous expirions)
  while (!radio.available()) {

    // aucune réponse reçue dans notre délai
    if (millis() - started_waiting_at > 250) {
      Serial.print("TX: Got no reply");
      delay(2000);
      return;
    }
  }

  // Maintenant lisez les données qui nous attendent dans le buffer du nRF24L01
  radio.read(&myData, sizeof(myData));
  delay(250);

  // Montre à l'utilisateur ce que nous avons envoyé et ce que nous avons récupéré
  Serial.print("Temperature: "); Serial.println(myData.temp);
  Serial.print("Joystick  : "); Serial.println(myData.x);
  Serial.println("-------------------\n");

  delay(1000);
}
