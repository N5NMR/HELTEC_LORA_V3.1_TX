/**
 * Beispiel: Standalone LoRa-Sensor – reiner Sender.
 * Dieser Code liest periodisch Sensorwerte (z. B. Temperatur und Luftdruck)
 * und sendet sie über LoRa. Es findet kein Empfang statt.


 https://heltec.org/project/wifi-lora-32-v3/


 */
#include <heltec_unofficial.h>
#include <Adafruit_BME280.h>

// ----- Konfiguration -----
#define PAUSE               30             // 300 Pause zwischen Sendungen in Sekunden
#define FREQUENCY           866.3           // Frequenz in MHz (Europa)
#define BANDWIDTH           250.0           // LoRa-Bandbreite in kHz
#define SPREADING_FACTOR    9               // LoRa-Spreading Factor (5 bis 12)
#define TRANSMIT_POWER      0              // Sendeleistung in dBm MAX:20

// ----- Globale Variablen -----
long packetCounter = 0;                  // Laufende Paketnummer
uint64_t lastTxTime = 0;                 // Zeitpunkt der letzten Sendung
uint64_t txDuration = 0;                 // Dauer der letzten Sendung (in ms)
uint64_t minPause = 0;                   // Mindestwartezeit zur Einhaltung des Duty-Cycle

// Sensorobjekt (BME280)
Adafruit_BME280 bme;

// Initialisiert den BME280-Sensor.
void setupSensors() 
{
  
  // if (!bme.begin(0x76)) 
 // {   // Adresse ggf. anpassen
  //   both.println("BME280 Sensor nicht gefunden!");
  // } 
  //else 
  //{
  //   both.println("BME280 Sensor initialisiert.");
  // }
}

// Liest die Sensorwerte und formatiert sie als Nachricht.
String readSensorData() 
{
  //For testing only Random Numbers. Fix your Sensor here
  int tempR = random(15,25);
  int preR = random(950, 1100);
  float temperature = tempR;  //bme.readTemperature();
  float pressure = preR;      //bme.readPressure() / 100.0F;  // Umrechnung in hPa
  String sensorData = "T:" + String(temperature, 2) + "C, P:" +
                      String(pressure, 2) + "hPa, ID:" + String(packetCounter);
  return sensorData;
}

void setup() 
{
  heltec_setup();
  
  both.println("Radio init");
  RADIOLIB_OR_HALT(radio.begin());
  
  both.printf("Frequenz: %.2f MHz\n", FREQUENCY);
  RADIOLIB_OR_HALT(radio.setFrequency(FREQUENCY));
  both.printf("Bandbreite: %.1f kHz\n", BANDWIDTH);
  RADIOLIB_OR_HALT(radio.setBandwidth(BANDWIDTH));
  both.printf("Spreading Factor: %i\n", SPREADING_FACTOR);
  RADIOLIB_OR_HALT(radio.setSpreadingFactor(SPREADING_FACTOR));
  both.printf("TX-Leistung: %i dBm\n", TRANSMIT_POWER);
  RADIOLIB_OR_HALT(radio.setOutputPower(TRANSMIT_POWER));
  
  // Empfangsmodus wird nicht gestartet, da das Gerät nur senden soll.
  // Starte den Sensor:
  setupSensors();
}

void loop() 
{
  heltec_loop();
  
  // Sende periodisch alle PAUSE Sekunden
  if (millis() - lastTxTime >= (PAUSE * 1000)) 
  {
    String sensorData = readSensorData();
    both.printf("TX: [%s]\n", sensorData.c_str());
    
    // LED als visuelles Feedback einschalten
    heltec_led(50);  // LED auf 50% Helligkeit
    txDuration = millis();
    
    // Sende das Sensorpaket
    RADIOLIB(radio.transmit(sensorData.c_str()));
    txDuration = millis() - txDuration;
    heltec_led(0);  // LED ausschalten
    
    if (_radiolib_status == RADIOLIB_ERR_NONE) 
    {
      both.printf("Übertragung erfolgreich (%i ms)\n", (int)txDuration);
    } 
    else 
    {
      both.printf("Fehler beim Senden (%i)\n", _radiolib_status);
    }
    
    // Mindestwartezeit für max. 1% Duty Cycle berechnen
    minPause = txDuration * 100;
    lastTxTime = millis();
    packetCounter++;
  }
}
