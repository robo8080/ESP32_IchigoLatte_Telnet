# ESP32_IchigoLatte_Telnet
ESP32でIchigoLatteをTelnetで接続できるようにします。<br><br>
### 特徴 ###
* IchigoLatteにモニタやキーボードを付けなくてもWiFi経由でIchigoLatteを操作できます。<br>
* MixJuiceの"MJ GET"、"MJ POST"コマンドが使えます。
* MixJuiceの"MJ GET"コマンドが使えるのでMixJuice対応のサイトから直接プログラムをダウンロードできます。<br>
* エスケープシーケンス対応のターミナルソフトを使えばカラー表示等ができます。<br>
![画像1](images/image1.png)<br><br>
### 必要な物 ###
* IchigoLatte、またはIchigoLatteファーム書き込み済みのLPC1114FN28<br>
* ESP32-DevKitC<br>
* Arduino IDE (1.8.2で動作確認をしました。)<br>
* [Arduino core for ESP32 WiFi chip](https://github.com/espressif/arduino-esp32 "Title")
* [QueueArray Library For Arduino](http://playground.arduino.cc/Code/QueueArray "Title")<br>

### 配線(LPC1114FN28を使った例) ###
![画像2](images/image2.png)<br><br>

### 使い方 ###
 1. スケッチのssidとpasswordを自分の環境に合わせて書き換えて下さい。<br>
 2. ESP32にスケッチを書き込み後、シリアルモニタを開くとIPアドレスとポート番号が表示されるのでメモしておいて下さい。
 3. Tera Termを使う場合の設定<br><br>
![画像3](images/image3.png)<br><br>
![画像4](images/image4.png)<br><br>
 4. Telnetで接続後IchigoLatteのBTNスイッチを押すと、IchigoLatteのシェルのプロンプトが表示されます。


### 制限事項 ###
* IchigoLatteのviエディタは使えません。<br>
* 画面制御コマンドには対応していません。<br>


---
IchigoLatte and MixJuice by na-s.jp (Natural Style Co. Ltd.) / PCN<br><br>
Michio OnoさんのMicJackのソースの一部を使わせて頂いております。<br>
MicJack CC BY Michio Ono (Micono Utilities) <http://ijutilities.micutil.com><br>
<br>
