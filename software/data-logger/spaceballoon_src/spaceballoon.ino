#include <DHT22.h>
// Only used for sprintf
#include <stdio.h>


// PINs in use on PRO Micro

// 18(A0)   -> SD:CS
// 14(MISO) -> SD:MOSI
// 16(MOSI) -> SD:MISO
// 13(SCLK) -> SD:CLK
// 19       -> DHT:DATA
// VCC      -> GPS:VIN
// 1(TX)    -> GPS:RX
// 0(RX)    -> GPS:TX
// 21(A2)   -> XBEE:DIN
// 20(A3)   -> XBEE:DOUT
// 5        -> LED:IN

#define DHT22_PIN 19
#define DHT22_PIN2 21

#define CS_PIN 18
#define LED_PIN 5
#define GPS_RX_PIN 0
#define GPS_TX_PIN 1

#define XBEE_RX_PIN 20
#define XBEE_TX_PIN 21


DHT22 myDHT22(DHT22_PIN);

// https://www.arduino.cc/en/Tutorial/Datalogger

#include <SPI.h>
#include <SD.h>
#define SAMPLE_INTERVAL_MS 2000

uint32_t logTime;

// software serial
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
//SoftwareSerial gps_serial(GPS_RX_PIN, GPS_TX_PIN);
//Adafruit_GPS GPS(&gps_serial);

HardwareSerial gps_serial = Serial1;
Adafruit_GPS GPS(&Serial1);


SoftwareSerial xbee(XBEE_RX_PIN, XBEE_TX_PIN);
int counter = 0;


void setup(void)
{
  // SERIAL PORT
  delay(3000);
  Serial.begin(9600);

  // LED
  pinMode(LED_PIN, OUTPUT);

  // SERIAL
  if (!SD.begin(CS_PIN)) {
    // Serial.println("CARD FAILED TO INITIALIZE");
    return;
  }
  // Serial.println("CARD INITIALIZED");

  // GPS
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  GPS.sendCommand(PGCMD_ANTENNA);
  delay(1000);
  //gps_serial.println(PMTK_Q_RELEASE); // Firmware Version
  // Serial.println("GPS INITIALIZED");

	// XBEE
	xbee.begin( 9600 );
  // Serial.println("XBEE INITIALIZED");


  // TIMING
  logTime = micros()/(1000UL*SAMPLE_INTERVAL_MS) + 1;
  logTime *= 1000UL*SAMPLE_INTERVAL_MS;
}

void loop(void)
{
	String transmit;
  String dataString;
  // The sensor can only be read from every 1-2s, and requires a minimum
  // 2s warm-up after power-on.
  logTime += 1000UL*SAMPLE_INTERVAL_MS;

  // WAIT FOR CORRECT LOG TIME
  int32_t diff;
	bool gps_parsed = false;

  // Serial.println("Waiting for GPS");
	do {
		char c = GPS.read();
		gps_parsed = false;
		if ( GPS.newNMEAreceived()) {
			if (GPS.parse(GPS.lastNMEA())){
					gps_parsed = true;
			}
		}
	} while (!gps_parsed);

	// Serial.println("Waiting for Timeout");
  do {
    diff = micros() - logTime;
  } while (diff < 0);

  digitalWrite(LED_PIN, 1);


  // DHT22
  // Serial.println("Requesting data...");
  if (myDHT22.readData() == DHT_ERROR_NONE){
			dataString += ",";
			dataString += myDHT22.getTemperatureC();
			dataString += ",";
      dataString += myDHT22.getHumidity();
  }else {
    dataString += ",,";
  }

	transmit =  "$trk,";
	transmit += GPS.hour ;
  transmit += ":";
	transmit += GPS.minute ;
  transmit += ":";
	transmit += round((float)GPS.seconds + (float)GPS.milliseconds/1000) ;
  transmit += ",";
	transmit += String(GPS.latitude,4);
  transmit += ",";
	transmit += String(GPS.longitude,4);
  transmit += ",";
  // explicit typecast to String to keep more than 2 digits
	transmit += String(GPS.latitudeDegrees,6) ; 
  transmit += ",";
	transmit += String(GPS.longitudeDegrees,6) ;
  transmit += ",";
	transmit += GPS.altitude ;
  transmit += ",";
	transmit += GPS.speed ;
  transmit += ",";
	transmit += String(GPS.angle,4) ;
  transmit += ",";
	transmit += (int)GPS.satellites ;
  transmit += ",";
	transmit += (int)GPS.fix ;
  transmit += ",";
	transmit += (int)GPS.fixquality ;
  transmit += ",";
	transmit += counter;
  transmit += dataString;

	xbee.println(transmit);

  File dataFile = SD.open("datalog.txt", FILE_WRITE);
	Serial.println(transmit);
  if (dataFile) {
    dataFile.println(transmit);
    dataFile.close();
  }
  else {
    // Serial.println("error opening datalog.txt");
  }
	counter += 1;
  digitalWrite(LED_PIN, 0);

}
