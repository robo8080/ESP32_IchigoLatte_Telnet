//#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <QueueArray.h>

HardwareSerial Serial1(2);
//rxPin = 16
//txPin = 17
// create a queue of characters.
QueueArray <char> queue;
const char* ssid = "******************";
const char* password = "******************";
WiFiServer server(10001);
WiFiClient serverClient;
//#define LED   23
#define LED   25
uint8_t cmd[6] = {255,251,3,255,253,3}; // suppress go-ahead

void serverSetup() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW

  WiFi.begin(ssid, password);
  Serial.print("\nConnecting to "); Serial.println(ssid);
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 20) {
    Serial.print(".");
    delay(500);
  }
  if (i == 21) {
    Serial.print("Could not connect to"); Serial.println(ssid);
    while (1) delay(500);
  } else {
    digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
  }
  server.begin();
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(115200);
  serverSetup();
  delay(2000);
}

void serial2client(int select) {
  // read from port 1, send to Client:
  while(Serial1.available()){
    size_t len = Serial1.available();
    uint8_t sbuf[len];
    Serial1.readBytes(sbuf, len);
//    serverClient.write(sbuf, len);
    for (int i = 0 ; i < len;i++) {
      if(sbuf[i] == 0x0a) {
        serverClient.print("\r\n");
      } else {
        serverClient.write(sbuf[i]);
      }
    }
    if(select != 0) { 
      for (int i= 0 ; i< len ;i++) {
          MJ_Command((char)sbuf[i]);
       }
    } else {
      for (int i= 0 ; i< len ;i++) {
          queue.enqueue ((char)sbuf[i]);
       }        
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  while (1) {
    Serial.print("\nReady! Use 'telnet ");
    Serial.print(WiFi.localIP());
    Serial.println(" 10001' to connect");

    digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
    // wait for a new client:
    while (1) {
      serverClient = server.available();
      if (serverClient) break;
      digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(500);                       // wait for a second
      digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
      delay(500);                       // wait for a second
    }
    Serial.println("Client Connected");
    digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(10);
    serverClient.write(&cmd[0],6);  // suppress go-ahead
    delay(500);
//    serverClient.print("\r\n");
    while (serverClient.connected()) {
      while(serverClient.available())
      {
        size_t len = serverClient.available();
        uint8_t sbuf[len];
        serverClient.read(sbuf, len);
//        serverClient.write(sbuf, len);  //Local echo
        for (int i = 0 ; i < len;i++) {
          if(sbuf[i] == 0x0d) {
            serverClient.print("\r\n");
          } else if(sbuf[i] == 0x08) {
            serverClient.print("\b \b");
          } else if(sbuf[i] != 0x0a)
            serverClient.write(sbuf[i]);
        }
        for (int i = 0 ; i < len;i++) {
          if(sbuf[i] == 0x0d)
            Serial1.write(0x0a);
          else if(sbuf[i] != 0x0a)
            Serial1.write(sbuf[i]);
        }
      }      
      serial2client(1);
      while (!queue.isEmpty ()) {
        MJ_Command(queue.dequeue ());
      }
    } //ループの末尾
    if (serverClient) serverClient.stop();
    Serial.println("\nClient Disconnected");
  } //無限ループの末尾

}

//------------------------------------------------------------------------------
/*
 *  MicJack by Michio Ono.
 *  IoT interface module for IchigoJam with ESP-WROOM-02
 *  CC BY
 */
// modified by robo8080

const String MicJackVer="MicJack-0.8.0";
const String MJVer="MixJuice-1.2.0";
String inStr = ""; // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

int httpPort = 80;

int posttype;
boolean postmode=false;
enum { HTML_GET, HTML_GETS, 
       HTML_POST, HTML_POSTS };
String postaddr="";
String postdata="";
String myPostContentType="";

#define kspw  20
#define kspn  30
int spw=kspw;
int spn=kspn;

/*** Use access Status LED ***/
//#define useMJLED
#ifdef useMJLED
  const int connLED=12; //Green
  const int postLED=4;  //Yellow
  const int getLED=5;   //Red
#endif

String homepage="mj.micutil.com";
String lastGET=homepage;

/***************************************
 *  Reset POST parameters
 *  POST用パラメータをリセット
 *  
***************************************/
void ResetPostParam() {
  postmode=false;
  postaddr="";
  postdata="";
}

void SetPCT(String s) {
  int n=s.length();
  if(n<=0) {
    myPostContentType="";
  } else {
    //Content-Type: application/x-www-form-urlencoded;\r\n
    myPostContentType="Content-Type: "+s+"\r\n";
  }
}

void MJ_Command(char inChar) {

//  while (Serial11.available()) {
//    // get the new byte:
//    char inChar = (char)Serial1.read();
    Serial.write(inChar);
    //Check end of line 
    if(inChar=='\n' || inChar=='\r') {
      stringComplete = true;
      inStr.replace("lash>","");
//      if(!postmode) Serial1.flush(); //読み飛ばし "OK"など
//      break;
    } else {
      inStr += inChar;
    }
    
//  }

  if(stringComplete) {  //データあり
    stringComplete=false;
    String cs=inStr;//文字をコピー
    cs.toUpperCase();//大文字に

    if(postmode) {
      /**** POSTモード 処理 ****/
      if(cs.startsWith("MJ POST END")) {
        /* POST通信開始 */
        MJ_HTML(posttype,postaddr);
        ResetPostParam();
      } else if(cs.startsWith("MJ POSTS END")) {
        /* POST通信開始 */
        MJ_HTMLS(posttype,postaddr);
        ResetPostParam();
      } else if(cs.startsWith("MJ POST CANCEL") || cs.startsWith("MJ POSTS CANCEL") ||
                cs.startsWith("MJ POST STOP") || cs.startsWith("MJ POSTS STOP") ||
                cs.startsWith("MJ POST ESC") || cs.startsWith("MJ POSTS ESC")) {
        /*POST/POSTSデータリセット*/
        ResetPostParam();
        
      } else {
        /* POSTコンテンツを記録 */
        if(inStr.endsWith("\n")) inStr=inStr.substring(0,inStr.length()-1);//最後の"\n"を外す
        postdata+=inStr;
        postdata+="\n";

      }
      
   } else if(cs.startsWith("MJ GET ")) {
      /*** GET通信 ****/
      MJ_HTML(HTML_GET,inStr.substring(7));
     
    } else if(cs.startsWith("MJ GETS ")) {
      /**** HTMLS GET通信 ****/
      MJ_HTMLS(HTML_GETS,inStr.substring(8));
     
 //   } else if(cs.startsWith("MJ FUJI")) {
 //     /*** FUJI ***/ MJ_HTMLS(HTML_GETS,"raw.githubusercontent.com/bokunimowakaru/petit15term/master/fuji.txt");
    
    } else if(cs.startsWith("MJ POST START ")) {
      /*** POST START通信 ***/
      MJ_POST_START(HTML_POST,inStr.substring(14));
      
    } else if(cs.startsWith("MJ POSTS START ")) {
      /*** HTMLS POST通信 ***/
      MJ_POST_START(HTML_POSTS,inStr.substring(15));
      
    } else if(cs.startsWith("MJ PCT ")) {
      /*** PCT ***/ SetPCT(inStr.substring(7));
    
    } else if(cs.startsWith("MJ MJVER")) {
      /*** バージョン ***/ Serial1.println("'"+MicJackVer);
      serverClient.println(MicJackVer);
//      IJCodeSendString(WS_num, "'"+MicJackVer);
//      IJCodeSend(WS_num, 0x0a);
      // Verで、エラー表示で、MixJuiceのバージョンを表示
      // MJVerで、MicJackのバージョンを表示
      
    } else if(cs.startsWith("MJ ")) {
      /*** NG ***/ Serial1.println("'NG: "+MJVer);
      serverClient.println("NG: "+MJVer);
//      IJCodeSendString(WS_num, "'NG: "+MJVer);
//      IJCodeSend(WS_num, 0x0a);
    }

    inStr="";
  }

  
}


/***************************************
 *  MJ GET / POST
 *  HTTP GET/POST 通信
 *  
***************************************/

void MJ_HTML(int type, String addr) {
  int sc=addr.length();
  if(sc<=0) return;

  String host="";
  String url="/";
  String prm="";
  
  int ps;
  ps=addr.indexOf("/");
  if(ps<0) {
    host=addr;
  } else {
    host=addr.substring(0,ps);  // /より前
    url=addr.substring(ps);     // /を含んで後ろ
  }
  //Serial1.println("'"+host);
  //Serial1.println("'"+url);

  //------プロキシ-----
  String httpServer=host;
//  if(useProxy) httpServer=httpProxy;

  //------ポート-----
  int port=httpPort;
  ps=host.indexOf(":");
  if(ps>0) {
    host=host.substring(0,ps);  // :より前
    port=host.substring(ps+1).toInt();     // :より後ろ
  }
  
  //------せつぞく-----
  WiFiClient client;
  if(!client.connect(httpServer.c_str(), port)) {
    Serial1.write(0x04);
      return;
  }
    
  //------リクエスト-----
  switch(type) {
    case HTML_GET:
    case HTML_GETS:
      #ifdef useMJLED
        digitalWrite(getLED, HIGH);
      #endif
      client.print(String("GET ") + url + " HTTP/1.0\r\n" + 
                   "Host: " + host + "\r\n" + 
                   "User-Agent: " + MJVer + "\r\n" + 
                   //"Connection: close\r\n" +
                   "\r\n");
                   //"Accept: */*\r\n" + 
      break;

    case HTML_POST:
    case HTML_POSTS:
      #ifdef useMJLED
        digitalWrite(postLED, HIGH);
      #endif
      prm=postdata;//postdata.replace("\n","\r\n");//改行を置き換え

      client.print(String("POST ") + url + " HTTP/1.0\r\n" + 
                  "Accept: */*\r\n" + 
                  "Host: " + host + "\r\n" + 
                  "User-Agent: " + MJVer + "\r\n" + 
                  //"Connection: close\r\n" +
                  myPostContentType +
                  "Content-Length: " + String(prm.length()) + "\r\n" +
                  "\r\n" + 
                  prm);
                  //"Content-Type: application/x-www-form-urlencoded;\r\n" +
     
      break;

    default:
      return;
  }

  //------待ち-----
  int64_t timeout = millis() + 5000;
  while (client.available() == 0) {
    if (timeout - millis() < 0) {
      Serial1.println("'Client Timeout !");
      serverClient.println("Client Timeout !");
      client.stop();
      Serial1.write(0x04);
        return;
    }
  }

  //------レスポンス-----
  switch(type) {
    case HTML_GET:
    case HTML_GETS:
      /* ヘッダー 読み飛ばし */
      while(client.available()){
        String line = client.readStringUntil('\n');
        if(line.length()<2) break;
      }
       
      /*
       * プログラム入力時は、改行後、50msくらい,
       * 文字は、20msはあけないと、
       * IchigoJamは処理できない
      */
      if(spw<=1) {
        while(client.available()){
          String line = client.readStringUntil('\n');
          Serial1.print(line);
          Serial1.print('\n');
          serverClient.print(line);
          serverClient.print("\r\n");
          delay(spn);
        }
      } else {
        while(client.available()){
          char c = client.read();
          switch(c){
          case 0:
            break;
          case '\r':
            break;
          case '\n':
            Serial1.print('\n');
//            IJCodeSend(WS_num, (uint8_t)'\n');
            serverClient.print("\r\n");
            delay(spn);
            break;
          default:
            Serial1.print(c);
//            IJCodeSend(WS_num, (uint8_t)c);
            serverClient.print(c);
            delay(spw);
            break;
          }
          serial2client(0);
        }
      }
      if(!addr.equals(homepage)) lastGET=addr;
      #ifdef useMJLED
        digitalWrite(getLED, LOW);
      #endif
      Serial1.write(0x04);
        break;

    case HTML_POST:
    case HTML_POSTS:
      Serial1.println("'POST OK!");
      serverClient.println("POST OK!");
//      IJCodeSendString(WS_num, "'POST OK!");
//      IJCodeSend(WS_num, 0x0a);
      #ifdef useMJLED
        digitalWrite(postLED, LOW);
      #endif
      Serial1.write(0x04);
        break;
      
    default:
      /* 読み飛ばし */
      while(client.available()){
        String line = client.readStringUntil('\r');
      }
      break;
  }

}

/***************************************
 *  MJ GETS / POSTS
 *  HTTP GETS/POSTS 通信
 *  
***************************************/

void MJ_HTMLS(int type, String addr) {
  int sc=addr.length();
  if(sc<=0) return;

  String host="";
  String url="/";
  String prm="";
  
  int ps;
  ps=addr.indexOf("/");
  if(ps<0) {
    host=addr;
  } else {
    host=addr.substring(0,ps);  // /より前
    url=addr.substring(ps);     // /を含んで後ろ
  }
  //Serial1.println("'"+host);
  //Serial1.println("'"+url);

  //------プロキシ-----
  String httpServer=host;
//  if(useProxy) httpServer=httpProxy;

  //------ポート-----
  int port=httpPort;
  ps=host.indexOf(":");
  if(ps>0) {
    host=host.substring(0,ps);  // :より前
    port=host.substring(ps+1).toInt();     // :より後ろ
  }
  
  switch(type) {
    case HTML_GET:
    case HTML_POST:
      break;
    case HTML_POSTS:
    case HTML_GETS:
      if(port==80) port=443;
      break;
  }
  
  //------セキュアーなせつぞく-----
  WiFiClientSecure client;
  if(!client.connect(httpServer.c_str(), port)) {
    Serial1.write(0x04);
      return;
  }

  //------リクエスト-----
  switch(type) {
    case HTML_GET:
    case HTML_GETS:
      #ifdef useMJLED
        digitalWrite(getLED, HIGH);
      #endif
      client.print(String("GET ") + url + " HTTP/1.0\r\n" + 
                   "Host: " + host + "\r\n" + 
                   "User-Agent: " + MJVer + "\r\n" + 
//                   "Connection: close\r\n" + 
                   "\r\n");
                   //"Accept: */*\r\n" + 
      break;

    case HTML_POST:
    case HTML_POSTS:
      #ifdef useMJLED
        digitalWrite(postLED, HIGH);
      #endif
      prm=postdata;//postdata.replace("\n","\r\n");//改行を置き換え

      client.print(String("POST ") + url + " HTTP/1.0\r\n" + 
                  "Accept: */*\r\n" + 
                  "Host: " + host + "\r\n" + 
                  "User-Agent: " + MJVer + "\r\n" + 
                  //"Connection: close\r\n" +
                  myPostContentType +
                  "Content-Length: " + String(prm.length()) + "\r\n" +
                  "\r\n" + 
                  prm);
                  //"Content-Type: application/x-www-form-urlencoded;\r\n" +
     
      break;

    default:
      return;
  }

  //------待ち-----
  int64_t timeout = millis() + 5000;
  while (client.available() == 0) {
    if (timeout - millis() < 0) {
      Serial1.println("'Client Timeout !");
      serverClient.println("Client Timeout !");
      client.stop();
      Serial1.write(0x04);
      return;
    }
  }

  //------レスポンス-----
  switch(type) {
    case HTML_GET:
    case HTML_GETS:
      /* ヘッダー 読み飛ばし */
      while(client.available()){
        String line = client.readStringUntil('\n');
        if(line.length()<2) break;
      }
       
      /*
       * プログラム入力時は、改行後、50msくらい,
       * 文字は、20msはあけないと、
       * IchigoJamは処理できない
      */
      if(spw<=1) {
        while(client.available()){
          String line = client.readStringUntil('\n');
          Serial1.print(line);
          Serial1.print('\n');
          serverClient.print(line);
          serverClient.print("\r\n");
          delay(spn);
        }
      } else {
        while(client.available()){
          char c = client.read();
          switch(c){
          case 0:
            break;
          case '\r':
            break;
          case '\n':
            Serial1.print('\n');
            serverClient.print("\r\n");
//            IJCodeSend(WS_num, (uint8_t)'\n');
            delay(spn);
            break;
          default:
            Serial1.print(c);
            serverClient.print(c);
//            IJCodeSend(WS_num, (uint8_t)c);
            delay(spw);
            break;
          }
          serial2client(0);
        }
      }
      if(!addr.equals(homepage)) lastGET=addr;
      #ifdef useMJLED
        digitalWrite(getLED, LOW);
      #endif
      Serial1.write(0x04);
      break;

    case HTML_POST:
    case HTML_POSTS:
      Serial1.println("'POST OK!");
      serverClient.println("POST OK!");
//      IJCodeSendString(WS_num,"'POST OK!");
//      IJCodeSend(WS_num, 0x0a);
      #ifdef useMJLED
        digitalWrite(postLED, LOW);
      #endif
      Serial1.write(0x04);
      break;
      
    default:
      /* 読み飛ばし */
      while(client.available()){
        String line = client.readStringUntil('\r');
      }
      break;
  }

}

/***************************************
 *  MJ POST START addr
 *  POST/POSTS 開始
 *  
***************************************/

void MJ_POST_START(int type, String addr) {
  int n=addr.length();
  if(n>0) {
    posttype=type;
    postaddr=addr;
    postdata="";
    postmode=true;
  }
}


