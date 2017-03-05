#include <TimeAlarms.h>
#include <WiFlyHQ.h>
#include <SoftwareSerial.h>
#include <avr/wdt.h>

boolean initialiseSensorFlag;
unsigned long initialiseSensorTimestamp;

long previousMillis = 0;
long interval = 1000;

unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 30000;//sampe 30s ;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;

SoftwareSerial wifiSerial(2,3); //WIFI SHIELD CLIENT

#define DEBUG 1                       
const int pinRx = 8;                  
const int pinTx = 7;                  
SoftwareSerial sensor(pinTx,pinRx);  

const unsigned char cmd_get_sensor[] = 
{
    0xff, 0x01, 0x86, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x79
};
unsigned char dataRevice[9];          //CO2 Sensor
int temperature;                      // Store Temp - CO2 sensor
int CO2PPM;                           // Store CO2 ppm - CO2 sensor
float uvindex;                        // Store UV - UV Sensor
float mq2ratio;                       // Store mq2 - MQ2 Sensor

WiFly wifly;                          //WIFI
String data;                          //WIFI

const char mySSID[] = "mywifi";
const char myPassword[] = "mykey";
const char site[] = "myserver.com";

void terminal();

void setup(){ 
  Serial.begin(9600);
  Alarm.timerRepeat(1800, MainAlarm); //Every 30 mins
  Serial.println("Alarms set!");
  wdt_enable(WDTO_8S);
}

void loop(){
  Alarm.delay(10); // wait one second between clock display
  if (wifly.moduleCrashed==true){
    Serial.println("moduleCrashed");
    wdt_enable(WDTO_8S);
    while (true);
  }
  
  wdt_reset();
  
  if(initialiseSensorFlag == true && millis() - initialiseSensorTimestamp > 3000){
    initialiseSensorFlag = false;    
    SampleUVAlarm(); // samples UV & MQ2
    wdt_reset();
    wifiStuff(); // connect to Wifi
    wdt_reset();
    reportToCloud(); // post to web
  }
}

void wifiStuff(){
  ///////////////////WiFi CLIENT SETUP////////////////////////////////////////////
   char buf[32];
   Serial.print("Free memory: ");
   Serial.println(wifly.getFreeMemory(),DEC);

   wifiSerial.begin(9600);
   if (!wifly.begin(&wifiSerial, &Serial)) {
       Serial.println("Failed to start wifly");
  terminal();
   }
   /* Join wifi network if not already associated */
   if (!wifly.isAssociated()) {
  /* Setup the WiFly to connect to a wifi network */
  Serial.println("Joining network");
  wifly.setSSID(mySSID);
        wifly.setPassphrase(myPassword);
  wifly.enableDHCP();

  if (wifly.join()) {
      Serial.println("Joined wifi network");
  } else {
      Serial.println("Failed to join wifi network");
      terminal();
  }
   } else {
       Serial.println("Already joined network");
   }
   
   wifly.setDeviceID("Wifly-WebClient2");
   
   Serial.print("DeviceID: ");
   Serial.println(wifly.getDeviceID(buf, sizeof(buf)));
   
   if (wifly.isConnected()) {
       Serial.println("Old connection active. Closing");
  wifly.close();
   }
}

void initialiseSensor(){
  sensor.begin(9600);
  Serial.begin(9600);
  Serial.println("********************************************************");
}

void MainAlarm(){
  Serial.println("Main Alarm...");
  Serial.println(wifly.moduleCrashed);
  initialiseSensor();
  initialiseSensorFlag = true;
  initialiseSensorTimestamp = millis();
}

void reportToCloud() {
  data = "";
  Serial.println("Reporting to cloud...");
   if (wifly.available() > 0) {
  char ch = wifly.read();
  Serial.write(ch);
  if (ch == '\n') {
      /* add a carriage return */ 
      Serial.write('\r');
  }
   }
   
   if (wifly.open(site, 80)) {
       Serial.print("Connected to ");
  Serial.println(site);
  
    // Set data to send
    static char outstr1[15];
    static char outstr2[15];
    static char outstr3[15];
    static char outstr4[15];

    String dataString1 = dtostrf(uvindex, 8, 2, outstr1);
    String dataString2 = dtostrf(mq2ratio, 8, 2, outstr2);
    String dataString3 = dtostrf(CO2PPM, 8, 2, outstr3);
    String dataString4 = dtostrf(temperature, 8, 2, outstr4);
    data = String("uvindex=" + dataString1 + "&mq2=" + dataString2 + "&age=" + dataString3 + "&name=" + dataString4);    
    Serial.print(data); //name = temp && age = co2
    //Reset all values
    uvindex = 0;
    mq2ratio = 0;
    CO2PPM = 0;
    temperature = 0;
    
        /* Send the request */
  wifly.println("POST /arduino/data_post.php HTTP/1.0");
  wifly.println("Host: www.myserver.com"); // SERVER ADDRESS HERE TOO
        wifly.println("Content-Type: application/x-www-form-urlencoded" );
        wifly.print("Content-Length: ");
        wifly.println(data.length());
  wifly.println();
        wifly.print(data);
        Serial.println("Posted successfully");
   } else {
       Serial.println(">>Failed to connect");
   }

   if (Serial.available() > 0) {
  wifly.write(Serial.read());
   }
   
   //Added Sat 14 Nov @820am
   wifly.close();
}

/* Connect the WiFly serial to the serial monitor. */
void terminal(){
   while (1) {
  if (wifly.available() > 0) {
      Serial.write(wifly.read());
  }
  if (Serial.available() > 0) {
      wifly.write(Serial.read());
  }
   }
}

void SampleUVAlarm (){
  Serial.println("Sampling UV");
  int sensorValue = 0;
  long  sum = 0;
  uvindex = 0;
  for(int i=0;i<1024;i++)
   {  
      sensorValue=analogRead(A0);
      sum=sensorValue+sum;
      delay(2);
   }   
 sum = sum >> 10;
 Serial.print("The UV voltage value:");
 Serial.print(sum*4980.0/1023.0);
 Serial.print(" mV");
 delay(20);
 Serial.print("UVIndex is: ");
 uvindex = (307*(sum*4980.0/1023.0))/200; //mW/m2 -> W/m2
 Serial.println(uvindex);
 Serial.print("\n");
 SampleMQ2Alarm();
}

void SampleMQ2Alarm() {
  Serial.println("Sampling MQ2");
  float sensor_volt = 0;
  float RS_gas = 0; // Get value of RS in a GAS
  float ratio = 0; // Get ratio RS_GAS/RS_air
  mq2ratio = 0;
  int sensorValue = analogRead(A1);
  sensor_volt=(float)sensorValue/1024*5.0;
  RS_gas = (5.0-sensor_volt)/sensor_volt; // omit *RL
 //R0 stabilized at 0.08 in secret room
  /*-Replace the name "R0" with the value of R0 in the demo of First Test -*/
  ratio = RS_gas/0.08;  // ratio = RS/R0 
  /*-----------------------------------------------------------------------*/
 
  Serial.print("MQ2 Sensor_volt = ");
  Serial.println(sensor_volt);
  Serial.print("RS_ratio = ");
  Serial.println(RS_gas);
  Serial.print("Rs/R0 = ");
  Serial.println(ratio);
  mq2ratio = ratio;
  Serial.print("\n\n");
  delay(1000);
  //mq2ratio = 333;
  printCO2(); // sample co2
}
void printCO2() {
  Serial.println("printing co2...");
  if(dataRecieve())
  {
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print("  CO2: ");
    Serial.print(CO2PPM);
    Serial.println("");
  }
  delay(500);
}

bool dataRecieve(void) {
  byte data[9];
  int i = 0;
  CO2PPM = 0;
  temperature = 0;
  
  //transmit command data
  for(i=0; i<sizeof(cmd_get_sensor); i++)
  {
    sensor.write(cmd_get_sensor[i]);
  }
  delay(500);
  //begin reveiceing data
  if(sensor.available())
  {
    while(sensor.available())
    {
      for(int i=0;i<9; i++)
      {
        data[i] = sensor.read();
      }
    }
  }
  
  #if DEBUG
  for(int j=0; j<9; j++)
  {
    Serial.print(data[j]);
    Serial.print(" ");    
  }
  Serial.println("");
  #endif
  
  if((i != 9) || (1 + (0xFF ^ (byte)(data[1] + data[2] + data[3] 
    + data[4] + data[5] + data[6] + data[7]))) != data[8])
  {
    Serial.println("false returned");
    return false;
  }

  CO2PPM = (int)data[2] * 256 + (int)data[3];
  temperature = (int)data[4] - 40;  
  Serial.println(temperature);
  return true;
}
