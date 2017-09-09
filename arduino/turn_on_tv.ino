#include "ESP8266.h"

#define SERIAL_SPEED 115200

#define WLAN_SSID "e-Broad630cg6vi"
#define WLAN_PASS "201717ba4e"

#define HOST_NAME "13.114.74.103"
#define STATUS_PHP_PATH "/statusGet.php"
#define MOISTURE_PHP_PATH "/moistureSet.php"
#define PORT_NUMBR 80

ESP8266 wifi;

const int ledPin  = 13;
int ir_out = 2;
 
// ディスプレイ電源
unsigned int data[] = {254,44,133,46,72,46,130,50,68,49,129,51,67,51,67,52,125,53,65,53,65,53,65,53,65,2565,
249,49,129,51,66,52,125,53,65,53,125,53,65,53,65,53,125,54,65,54,65,54,65,54,65,2563,
249,50,127,52,67,52,126,53,65,53,125,53,65,53,65,53,125,53,65,53,65,54,65,54,65};
int last = 0;
unsigned long us = micros();

String resultString = "";
String pastResultString = "";

void setup() {
  pinMode (ledPin ,OUTPUT);
  pinMode(ir_out, OUTPUT);

  Serial.begin(SERIAL_SPEED);
  Serial5.begin(SERIAL_SPEED);
  Serial.println("PG start");

  setupWiFi();

}

void setupWiFi(){
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  wifi.begin(Serial5);

  if (wifi.joinAP(WLAN_SSID, WLAN_PASS)) {
    Serial.print("Join AP success\r\n");
    Serial.print("IP: ");
    Serial.println(wifi.getLocalIP().c_str());
  } else {
    Serial.println("Join AP failure\r\n");
  }

  if (wifi.disableMUX()) {
    Serial.print("single ok\r\n");
  } else {
    Serial.print("single err\r\n");
  }

  if (wifi.setOprToStation()) {
    Serial.println("to station ok\r\n");
  } else {
    Serial.println("to station err\r\n");
  }
  
  while(!wifi.joinAP(WLAN_SSID,WLAN_PASS)){
    delay(250);
    Serial.print(".");    
  }
  Serial.println("WiFi Connected");
  Serial.print("IP address: ");
  Serial.println(wifi.getLocalIP().c_str());  
}

void loop() {
  
  //TCP接続を確立
  if(wifi.createTCP(HOST_NAME,PORT_NUMBR)){
    //sendMoisture();
    getStatus();
  }else{
    Serial.print("TCP failure");
  }
  Serial.println("");
  Serial.println("");
  Serial.println("");
  pastResultString = resultString;
  delay(1000);
}

void getStatus(){
    Serial.println("********getStatus-start**********");
    Serial.print("Requesting URL: ");   
    Serial.println(STATUS_PHP_PATH);
    
    //サーバへ送信するデータ
    char sendData[] = "";
    sprintf(sendData,"GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",STATUS_PHP_PATH,HOST_NAME);
    Serial.println("Send Data: ");
    Serial.println(sendData);
    //サーバへデータを送信
    wifi.send((const uint8_t*)sendData,strlen(sendData));
    
    //サーバから取得したデータ
    uint8_t buffer[512] = {0};
    
    //取得した文字列の長さ
    uint8_t len = wifi.recv(buffer,sizeof(buffer),10000);
    
    Serial.print("len: ");
    Serial.println(len);
    //一文字以上取得できていれば
    if(len > 0){
      //サーバから取得した文字列を解析
      resultString = parse(len,buffer);
      if(resultString == "LED_ON"){
        digitalWrite(ledPin,HIGH);
      }else if(resultString == "LED_OFF"){
        digitalWrite(ledPin,LOW);  
      }else if(resultString == "TV_ON"){
        if(resultString != pastResultString){
          sendSignal();
        }
      }else if(resultString == "TV_OFF"){
        if(resultString != pastResultString){
          sendSignal();
        }
      }else{
        Serial.print("予期せぬ文字列；");            
        Serial.println(resultString);      
      }
    }
    Serial.println("********getStatus-end**********");
}


String parse(int len, uint8_t buffer[512]){
  Serial.println("--------------parse-start----------------");
  //サーバから取得した文字列
  String resultCode ="";
  for (uint32_t i = 0 ; i < len ; i++){
    resultCode += (char)buffer[i];
  }
  Serial.println(resultCode);
  //文字列(status:)を検索
  String searchWord = "status:";
  int searchWordLength = searchWord.length();
  int lastLF = resultCode.indexOf(searchWord);
  //検索にHitしたら
  if(lastLF > 0){
    //全体の文字列の長さを取得する
    int resultCodeLength = resultCode.length();

    //検索がHitした位置＋検索した文字列数から最後までを取得
    String resultString = resultCode.substring(lastLF + searchWordLength,resultCodeLength);

    //前後のスペースを取り除く
    resultString.trim();

    Serial.println(resultString);
    Serial.println("--------------parse-end-succsess-----------");
    return resultString;
  }else{
    Serial.println("--------------parse-end-error--------------");
    return "ERROR";
  }
}

// dataからリモコン信号を送信する
void sendSignal() {
  Serial.println("----------sendSignal----------");
  int dataSize = sizeof(data) / sizeof(data[0]);
  for (int cnt = 0; cnt < dataSize; cnt++) {
    unsigned long len = data[cnt]*10;  // dataは10us単位でON/OFF時間を記録している
    unsigned long us = micros();
    do {
      digitalWrite(ir_out, 1 - (cnt&1)); // cntが偶数なら赤外線ON、奇数ならOFFのまま
      delayMicroseconds(8);  // キャリア周波数38kHzでON/OFFするよう時間調整
      digitalWrite(ir_out, 0);
      delayMicroseconds(7);
    } while (long(us + len - micros()) > 0); // 送信時間に達するまでループ
  }
}

//湿度センサーの値を送信する
void sendMoisture() {
  Serial.println("********sendMoisture-start**********");
  int moistureValue=analogRead(0); 

  //サーバへ送信するデータ
  char sendData[] = "";
  sprintf(sendData,"GET %s?value=%d HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",MOISTURE_PHP_PATH, moistureValue, HOST_NAME);
  Serial.println("Send Data: ");
  Serial.println(sendData);
  //サーバへデータを送信
  wifi.send((const uint8_t*)sendData,strlen(sendData));
  
  //サーバから取得したデータ
  uint8_t buffer[512] = {0};
  
  //取得した文字列の長さ
  uint8_t len = wifi.recv(buffer,sizeof(buffer),10000);
  Serial.print("moistureValue:");
  Serial.println(moistureValue);
  Serial.print("buffer:");
  Serial.println(len);
  Serial.println("********sendMoisture-end**********");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  
}


