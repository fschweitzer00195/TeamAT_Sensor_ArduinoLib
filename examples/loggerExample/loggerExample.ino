#include <TatLogger.h>
#define led 2

int count = 0;
TatLogger logger(2);
// Declare TatSensor object directly inside an array TatSensor(device_id)
// Device id can be found on the manage device page on website
TatSensor sensorArray[2] = {TatSensor(36), TatSensor(37)};
const char* SSID = "YOUR_WIFI_SSID";
const char* PASSWORD =  "YOUR_WIFI_PWD";


void toggleLed(int p_tics);

void setup()
{
    // Serial com
    Serial.begin(115200);
    while (!Serial) {
        delay(1);
    }
    Serial.println("Serial ON");
    
    // begin commusication with server
    logger.begin(SSID, PASSWORD);
    logger.login("your_account_username", "your_account_password");
    Serial.println(logger.getHTTPResponse());
	
	// dev board LED indicator
    pinMode(led, OUTPUT);
    digitalWrite(led, HIGH);
}


void loop()
{
    count ++;
    int tics = 1000;
	
    delay(tics/2);
    sensorArray[0].saveData(random(100), logger.getDatetime());
	
    delay(tics/2);
    sensorArray[1].saveData(random(10), logger.getDatetime());
	
    logger.smartLog(sensorArray);
	
    toggleLed(tics);
}


void toggleLed(int p_tics)
{
    if (p_tics > 1000)
    {
      if (count % (2) == 0)
      {
          digitalWrite(led, LOW);
  
      }
      else if (count % (1) == 0)
      {
          digitalWrite(led, HIGH);
      }
    }
    else 
    {
      if (count % ((int)(1000/p_tics)) == 0)
      {
          digitalWrite(led, HIGH);
  
      }
      if (count % ((int)(2000/p_tics)) == 0)
      {
          digitalWrite(led, LOW);
      }
    }
    
}
