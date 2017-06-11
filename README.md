# ESP32_IchigoLatte_Telnet
ESP32でIchigoLatteをTelnetで接続できるようにします。<br><br>
![画像1](images/image1.png)<br><br>
### 必要な物 ###
* IchigoLatte、またはIchigoLatteファーム書き込み済みのLPC1114FN28
* ESP32-DevKitC<br>
* Arduino IDE (1.8.0で動作確認をしました。)<br>
* [Arduino core for ESP32 WiFi chip](https://github.com/espressif/arduino-esp32 "Title")

### 配線(LPC1114FN28を使った例) ###
![画像2](images/image2.png)<br><br>

### 使い方 ###
 1. ESP32にスケッチを書き込み後、シリアルモニタを開くとIPアドレスとポート番号が表示されるのでメモしておいて下さい。
 2. Tera Termを使う場合の設定<br><br>
![画像3](images/image3.png)<br><br>
![画像4](images/image4.png)<br><br>
 3. Telnetで接続後IchigoLatteのBTNスイッチを押すと、IchigoLatteのシェルのプロンプトが表示されます。

