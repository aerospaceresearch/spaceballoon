// Test code for Adafruit GPS modules using MTK3329/MTK3339 driver
//
// This code shows how to listen to the GPS module in an interrupt
// which allows the program to have more 'freedom' - just parse
// when a new NMEA sentence is available! Then access data when
// desired.
//
// Tested and works great with the Adafruit Ultimate GPS module
// using MTK33x9 chipset
//    ------> http://www.adafruit.com/products/746
// Pick one up today at the Adafruit electronics shop 
// and help support open source hardware & software! -ada

//This code is intended for use with Arduino Leonardo and other ATmega32U4-based Arduinos

#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>

SoftwareSerial xbee(2, 3); // for leonardo and micro, this is for standard arduinos 2, 3 RX, TX

// Connect the GPS Power pin to 5V
// Connect the GPS Ground pin to ground
// If using software serial (sketch example default):
//   Connect the GPS TX (transmit) pin to Digital 8
//   Connect the GPS RX (receive) pin to Digital 7
// If using hardware serial:
//   Connect the GPS TX (transmit) pin to Arduino RX1 (Digital 0)
//   Connect the GPS RX (receive) pin to matching TX1 (Digital 1)

// If using software serial, keep these lines enabled
// (you can change the pin numbers to match your wiring):
//SoftwareSerial mySerial(8, 7);
//Adafruit_GPS GPS(&mySerial);

// If using hardware serial, comment
// out the above two lines and enable these two lines instead:
Adafruit_GPS GPS(&Serial1);
HardwareSerial mySerial = Serial1;

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO  true

int pin_activate = 12;
int tx_active = 0;

void setup()  
{
    
  // connect at 115200 so we can read the GPS fast enough and echo without dropping chars
  // also spit it out
  Serial.begin(115200);
  delay(5000);
  Serial.println("Adafruit GPS library basic test!");

  // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);
  
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
  // the parser doesn't care about other sentences at this time
  
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz

  // Request updates on antenna status, comment out to keep quiet
  GPS.sendCommand(PGCMD_ANTENNA);

  delay(1000);
  // Ask for firmware version
  mySerial.println(PMTK_Q_RELEASE);
  
  // set the data rate for the SoftwareSerial port
  xbee.begin( 9600 );
  
  pinMode(pin_activate, INPUT); // set pin to input
  pinMode(11, OUTPUT); // set pin to input
}

int counter = 0;

uint32_t timer = millis();
void loop()                     // run over and over again
{
  for (int i=0; i <= 5; i++){
    digitalWrite(11, LOW);
    delay(10);
    digitalWrite(11, HIGH);
    delay(10);
  }
  for (int i=0; i <= 20; i++){
    digitalWrite(11, LOW);
    delay(10);
    digitalWrite(11, HIGH);
    delay(25);
  }
  for (int i=0; i <= 10; i++){
    digitalWrite(11, LOW);
    delay(10);
    digitalWrite(11, HIGH);
    delay(10);
  }
  digitalWrite(11, LOW);
  delay(1000);
  
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
  if ((c) && (GPSECHO))
    Serial.write(c); 
  
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences! 
    // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
    //Serial.println(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false
  
    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another
  }

  // if millis() or timer wraps around, we'll just reset it
  if (timer > millis())  timer = millis();

  // approximately every 2 seconds or so, print out the current stats
  if (millis() - timer > 3333) { 
    timer = millis(); // reset the timer
    
    Serial.print("\nTime: ");
    Serial.print(GPS.hour, DEC); Serial.print(':');
    Serial.print(GPS.minute, DEC); Serial.print(':');
    Serial.print(GPS.seconds, DEC); Serial.print('.');
    Serial.println(GPS.milliseconds);
    Serial.print("Date: ");
    Serial.print(GPS.day, DEC); Serial.print('/');
    Serial.print(GPS.month, DEC); Serial.print("/20");
    Serial.println(GPS.year, DEC);
    Serial.print("Fix: "); Serial.print((int)GPS.fix);
    Serial.print(" quality: "); Serial.println((int)GPS.fixquality); 
    if (GPS.fix) {
      Serial.print("Location: ");
      Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
      Serial.print(", "); 
      Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
      
      Serial.print("Speed (knots): "); Serial.println(GPS.speed);
      Serial.print("Angle: "); Serial.println(GPS.angle);
      Serial.print("Altitude: "); Serial.println(GPS.altitude);
      Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
    }
    
    
    // preparing transmission
    float gps_longitude_float = GPS.longitude;
    float gps_latitude_float = GPS.latitude;
    
    String gps_longitude_string = "" + String(int(gps_longitude_float))+ "."+String(getDecimal(gps_longitude_float)); //combining both whole and decimal part in string with a full  
    int index_dot = gps_longitude_string.indexOf('.');
    long longitude_long_minute = long(gps_longitude_string.substring(0,index_dot-2).toInt()) * 60.0;
    longitude_long_minute = longitude_long_minute + long(gps_longitude_string.substring(index_dot-2,index_dot).toInt());
    String longitude_minute = "+";
    String geosector = String(GPS.lon);
    if(geosector.equals("W")){
        longitude_minute = "-";
    }
    longitude_minute = longitude_minute + String(longitude_long_minute + gps_longitude_string.substring(index_dot,index_dot+1+5));
    
    String gps_latitude_string = "" + String(int(gps_latitude_float))+ "."+String(getDecimal(gps_latitude_float)); //combining both whole and decimal part in string with a full  
    index_dot = gps_latitude_string.indexOf('.');
    long latitude_long_minute = long(gps_latitude_string.substring(0,index_dot-2).toInt()) * 60.0;
    latitude_long_minute = latitude_long_minute + long(gps_latitude_string.substring(index_dot-2,index_dot).toInt());
    String latitude_minute = "+";
    geosector = String(GPS.lat);
    if(geosector.equals("S")){
        latitude_minute = "-";
    }
    latitude_minute = latitude_minute + String(latitude_long_minute + gps_latitude_string.substring(index_dot,index_dot+1+5));
    
    String transmit = "";
    
    transmit = transmit + "$trk,";
    transmit = transmit + GPS.hour + ":";
    transmit = transmit + GPS.minute + ":";
    transmit = transmit + round((float)GPS.seconds + (float)GPS.milliseconds/1000) +",";
    transmit = transmit + latitude_minute + ",";
    transmit = transmit + longitude_minute + ",";
    transmit = transmit + GPS.altitude + ",";
    transmit = transmit + GPS.speed + ",";
    transmit = transmit + GPS.angle + ",";
    transmit = transmit + (int)GPS.satellites + ",";
    transmit = transmit + (int)GPS.fix + ",";
    transmit = transmit + (int)GPS.fixquality + ",";
    transmit = transmit + counter;
    transmit = transmit + "\n";
    
    // activate the transmission if activation line is high.
    // IF activation line was high one time, it will stay active. failsafe in case of dropping voltage on activation line
    if (digitalRead(pin_activate) == 1 || tx_active == 1){
      xbee.print(transmit);
      counter += 1;
      tx_active = 1;
    }
    
    Serial.println(digitalRead(pin_activate));
  }
  
}


//function to extract decimal part of float
long getDecimal(float val)
{
 int intPart = int(val);
 long decPart = 10000000*(val-intPart); //I am multiplying by 1000 assuming that the foat values will have a maximum of 3 decimal places
                                   //Change to match the number of decimal places you need
 if(decPart>0)return(decPart);           //return the decimal part of float number if it is available 
 else if(decPart<0)return((-1)*decPart); //if negative, multiply by -1
 else if(decPart=0)return(00);           //return 0 if decimal part of float number is not available
}


