#include <Arduino.h>
#include <SoftwareSerial.h>

// source aeq-web

String apn = "safaricom";                    //APN
String apn_u = "saf";                     //APN-Username
String apn_p = "data";                     //APN-Password
String url = "http://play.veno.co.ke/add_try.php";  //URL of Server
String number = "254724487933";
String smsMessage;
bool smsMode = false;

#define pumpPin 8
#define valvePin 9

SoftwareSerial SWserial(3, 2); // RX, TX

void gsm_send_serial(String command) {
  Serial.println("Send ->: " + command);
  SWserial.println(command);
  long wtimer = millis();
  while (wtimer + 3000 > millis()) {
    while (SWserial.available()) {
      Serial.write(SWserial.read());
    }
  }
  Serial.println();
}

void receiveSMS() {
  gsm_send_serial("AT+CMGF=1");

  // Decides how newly arrived SMS messages should be handled
  gsm_send_serial("AT+CNMI=1,2,0,0,0");  
}

void gsm_config_gprs() {
  Serial.println(" --- CONFIG GPRS --- ");
  gsm_send_serial("AT+SAPBR=3,1,Contype,GPRS");
  gsm_send_serial("AT+SAPBR=3,1,APN," + apn);
  if (apn_u != "") {
    gsm_send_serial("AT+SAPBR=3,1,USER," + apn_u);
  }
  if (apn_p != "") {
    gsm_send_serial("AT+SAPBR=3,1,PWD," + apn_p);
  }
}

void gsm_http_post( String postdata) {
  Serial.println(" --- Start GPRS & HTTP --- ");
  gsm_send_serial("AT+SAPBR=1,1");
  gsm_send_serial("AT+SAPBR=2,1");
  gsm_send_serial("AT+HTTPINIT");
  gsm_send_serial("AT+HTTPPARA=CID,1");
  gsm_send_serial("AT+HTTPPARA=URL," + url);
  gsm_send_serial("AT+HTTPPARA=CONTENT,application/x-www-form-urlencoded");
  gsm_send_serial("AT+HTTPDATA=192,5000");
  gsm_send_serial(postdata);
  gsm_send_serial("AT+HTTPACTION=1");
  gsm_send_serial("AT+HTTPREAD");
  gsm_send_serial("AT+HTTPTERM");
  gsm_send_serial("AT+SAPBR=0,1");
  receiveSMS();
}

void sendSMS(String number, char* text) {
  // Configuring module in TEXT mode
  gsm_send_serial("AT+CMGF=1");
  gsm_send_serial("AT+CMGS=\"+" + number + "\"");
  gsm_send_serial(text);
  SWserial.write(26);
  delay(5000);
}

void smsControl() {
  receiveSMS();
  while (smsMode) {
    if (SWserial.available() > 0) {
      smsMessage = SWserial.readString();
      Serial.print(smsMessage);
      delay(10);
    }
    if (smsMessage.indexOf("PUMPON") >= 0) {
      int pos = smsMessage.lastIndexOf(' ', smsMessage.length());
      String numb = smsMessage.substring(pos, smsMessage.length()-1);
      int durtn = numb.toInt();
      digitalWrite(pumpPin, HIGH);
      delay(durtn * 1000);
      digitalWrite(pumpPin, LOW);
      smsMessage = "";
      sendSMS("254724487933", "PUMP ON");
      // deleteAllSMS();
      // upload info
      gsm_config_gprs();
      gsm_http_post("temp=23&hum=90");
      Serial.print("DOOOONE!");
    }    
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(pumpPin, OUTPUT);
  pinMode(valvePin, OUTPUT);
  Serial.println("SIM800 AT SMS & GPRS Test");
  SWserial.begin(9600);
  delay(15000);
  while (SWserial.available()) {
    Serial.write(SWserial.read());
  }
  receiveSMS();
  delay(2000);
}

void loop() {
  // sms mode
  // gsm mode
  if (SWserial.available() > 0) {    
    smsMessage = SWserial.readString();    
    delay(10);
    
    if (smsMessage.indexOf("PUMPON") >= 0) { 
      digitalWrite(valvePin, HIGH);
      delay(200);
      digitalWrite(pumpPin, HIGH);
      delay(5000);
      digitalWrite(valvePin, LOW);
      digitalWrite(pumpPin, LOW);
      smsMessage = "";
      sendSMS(number, "PUMP ON");
      // deleteAllSMS();
      // upload info
      gsm_config_gprs();
      gsm_http_post("temp=23&hum=90");
      Serial.print("DOOOONE!");
    }
  }  
}
