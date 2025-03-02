// ==================================
// 全体共通のヘッダファイルのinclude
#include <Arduino.h>                         // Arduinoフレームワークを使用する場合は必ず必要
#include <SD.h>                              // SDカードを使うためのライブラリです。
#include <Update.h>                          // 定義しないとエラーが出るため追加。
#include <Ticker.h>                          // 定義しないとエラーが出るため追加。
#include <M5Unified.h>                       // M5Unifiedライブラリ
// ================================== End

// ==================================
// for SystemConfig
#include <Stackchan_system_config.h>          // stack-chanの初期設定ファイルを扱うライブラリ
StackchanSystemConfig system_config;          // (Stackchan_system_config.h) プログラム内で使用するパラメータをYAMLから読み込むクラスを定義
// ================================== End

// ==================================
// for LED
#include <FastLED.h>
#define LED_PIN 27
#define NUM_LEDS 1
static CRGB leds[NUM_LEDS];

void setLed(CRGB color)
{
  // change RGB to GRB
  uint8_t t = color.r;
  color.r = color.g;
  color.g = t;
  leds[0] = color;
  FastLED.show();
}
// ================================== End

// ==================================
// for Servo
#include <ServoEasing.hpp>

/**
 * サーボのPWM値と回転方向の関係（最右列は180サーボの場合の角度）
 * 
 * Tower Pro
 * 時計回り   : 500 - 1500US : 0 - 90度
 * 停止       : 1500US : 90度
 * 反時計周り : 1500 - 2400US : 90 - 180度
 * 
 * Feetech
 * 時計回り   : 700 - 1500US : 0 - 90度
 * 停止       : 1500US : 90度
 * 反時計周り : 1500 - 2300US : 90 - 180度
 * 
 * M5Stack
 * 時計回り   : 500 - 1500US : 0 - 90度
 * 停止       : 1500US : 90度
 * 反時計周り : 1500 - 2500US : 90 - 180度
*/

ServoEasing servo360;  // 360度サーボ

// #define USE_Servo_TowerPro
// #define USE_Servo_Feetech360
#define USE_Servo_M5Stack

// サーボの種類毎のPWM幅や初期角度、回転速度のレンジ設定など
#ifdef USE_Servo_TowerPro
  const int MIN_PWM = 500;
  const int MAX_PWM = 2400;
  // const int START_DEGREE_VALUE_SERVO_360 = 90;     // 360度サーボの停止位置：仕様では90で停止
  const int START_DEGREE_VALUE_SERVO_360 = 95;        // サーボ個体差で、90度指定で停止しなかった場合値を変えてみる（試作に使用したsg90-hvの場合95付近で停止だった)
  const int SERVO_DEG_RANGE_MAX = 12;
  const int SERVO_DEG_RANGE_MIN = -1 * SERVO_DEG_RANGE_MAX;
#endif
#ifdef USE_Servo_Feetech360
  const int MIN_PWM = 700;
  const int MAX_PWM = 2300;
  // const int START_DEGREE_VALUE_SERVO_360 = 90;     // 360度サーボの停止位置：仕様では90で停止
  const int START_DEGREE_VALUE_SERVO_360 = 93;        // サーボ個体差で、90度指定で停止しなかった場合値を変えてみる（試作に使用したsg90-hvの場合95付近で停止だった)
  const int SERVO_DEG_RANGE_MAX = 6;
  const int SERVO_DEG_RANGE_MIN = -1 * SERVO_DEG_RANGE_MAX;
#endif
#ifdef USE_Servo_M5Stack
  const int MIN_PWM = 500;
  const int MAX_PWM = 2500;
  const int START_DEGREE_VALUE_SERVO_360 = 90;         // 360度サーボの停止位置：仕様では90で停止（M5Stack公式は停止のレンジが85～95あたりと広めにとられている様子。手元では個体差なし）
  const int SERVO_DEG_RANGE_MAX = 12;
  const int SERVO_DEG_RANGE_MIN = -1 * SERVO_DEG_RANGE_MAX;
#endif

bool isRandomRunning = false;  // サーボ動作のフラグ
unsigned long prevTime360 = 0;
unsigned long interval360 = 0;

int servo360_speed = 0; // 360サーボの速度用変数

#define NOT_USE_ATOM_BATTERY_BASE    // Atom LiteのGroveポートから直接動作させる場合
// #define USE_ATOM_BATTERY_BASE     // M5ATOM用補助電池基板を使う場合

// M5ATOM用補助電池基板の使用／未使用ごとの接続PIN
#ifdef NOT_USE_ATOM_BATTERY_BASE
  const int SERVO_360_PIN = 26;  // Atom LiteのGroveポート 26
#endif
#ifdef USE_ATOM_BATTERY_BASE
  const int SERVO_360_PIN = 22;  // M5ATOM用補助電池基板のGroveポート 22
#endif

// ================================== End

// ==================================
// for Servo Running Mode

// サーボのタイマーを初期化する関数
void initializeServoTimers() {
  unsigned long now = millis();
  prevTime360 = now;
  interval360 = random(0, 101);
}

// ランダムモード開始時にタイマーをリセット
void startRandomMode() {
  isRandomRunning = true;
  initializeServoTimers();  // タイマー初期化
}

// ランダムモード
void servoRandomRunningMode(unsigned long currentMillis) {

  // === 360°サーボの動作 (7秒〜30秒間隔) ===
  if (currentMillis - prevTime360 >= interval360) {
    prevTime360 = currentMillis;
    interval360 = random(7000, 30001); // 7秒〜30秒のランダム間隔

    // 66 ～ 114 度が示す速度（初期位置は90、2度刻み）
    int rand_speed_offset_360 = random(SERVO_DEG_RANGE_MIN, SERVO_DEG_RANGE_MAX)* 2;
    servo360.startEaseTo(START_DEGREE_VALUE_SERVO_360 + rand_speed_offset_360);
  }

}

// ★テストモード
int count_360 = 0;
void servoTestRunningMode(unsigned long currentMillis) {

  // === 360°サーボの動作 (5秒間隔) ===
  if (currentMillis - prevTime360 >= interval360) {
    prevTime360 = currentMillis;
    interval360 = 5000; // 5秒間隔固定

    // 60 ～ 120 度が示す速度（初期位置は90）
    servo360.startEaseTo(START_DEGREE_VALUE_SERVO_360 + (count_360 % 7) * 10 - 30);
    count_360 = (count_360 + 1) % 99;
  }
}
// ================================== End

// ----------------------------------------------
void setup() {
  // 設定用の情報を抽出
  auto cfg = M5.config();
  // Groveポートの出力をしない（m5atomS3用）
  // cfg.output_power = true;
  // M5Stackをcfgの設定で初期化
  M5.begin(cfg);

  // ログ設定
  M5.Log.setLogLevel(m5::log_target_display, ESP_LOG_NONE);    // M5Unifiedのログ初期化（画面には表示しない。)
  M5.Log.setLogLevel(m5::log_target_serial, ESP_LOG_INFO);     // M5Unifiedのログ初期化（シリアルモニターにESP_LOG_INFOのレベルのみ表示する)
  M5.Log.setEnableColor(m5::log_target_serial, false);         // M5Unifiedのログ初期化（ログをカラー化しない。）
  M5_LOGI("Hello World");                                      // logにHello Worldと表示
  SD.begin(GPIO_NUM_4, SPI, 25000000);                         // SDカードの初期化
  delay(2000);                                                 // SDカードの初期化を少し待ちます。

  // servoの初期化
  M5_LOGI("attach servo");

  ESP32PWM::allocateTimer(0); // ESP32Servoはタイマーを割り当てる必要がある

  servo360.setPeriodHertz(50);  // サーボ用のPWMを50Hzに設定
  servo360.attach(SERVO_360_PIN, MIN_PWM, MAX_PWM);
  servo360.setEasingType(EASE_LINEAR);       // 一定の速度で動かす場合は EASE_LINEAR に変更
  setSpeedForAllServos(60);
  servo360.startEaseTo(START_DEGREE_VALUE_SERVO_360);  // 360°サーボを停止位置にセット

  M5.Power.setExtOutput(!system_config.getUseTakaoBase());       // 設定ファイルのTakaoBaseがtrueの場合は、Groveポートの5V出力をONにする。
  M5_LOGI("ServoType: %d\n", system_config.getServoType());      // サーボのタイプをログに出力

  // ランダム動作用の変数初期化、個体差を出すためMACアドレスを使用する
  uint8_t mac[6];
  esp_efuse_mac_get_default(mac);
  uint32_t seed = mac[0] | (mac[1] << 8) | (mac[2] << 16) | (mac[3] << 24);
  randomSeed(seed);
  interval360 = random(7000, 30001); // 7秒〜30秒のランダム間隔

  // ---------------------------------------------------------------
  // RGB LEDの初期化（状態を把握するために利用）
  // ---------------------------------------------------------------
  FastLED.addLeds<WS2811, LED_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(255 * 15 / 100);

  for (int i = 0; i < 3; i++)
  {
    setLed(CRGB::Blue);
    delay(500);
    setLed(CRGB::Black);
    delay(500);
  }
}

// ----------------------------------------------
void loop() {

  M5.update();

  // === ボタンAが押されたらテスト動作モードの開始/停止を切り替え ===
  if (M5.BtnA.wasPressed()) {
    if (!isRandomRunning) {
      setLed(CRGB::White);
      delay(1000);
      setLed(CRGB::Black);
      startRandomMode();
    } else {
      isRandomRunning = false;
      servo360.startEaseTo(START_DEGREE_VALUE_SERVO_360);  // 360°サーボを停止
      setLed(CRGB::Red);
      delay(1000);
      setLed(CRGB::Black);
      }
  }

  if (!isRandomRunning) return;  // 停止中なら何もしない

  unsigned long currentMillis = millis();
  // ランダム動作モード（デフォルト）
  if (isRandomRunning) servoRandomRunningMode(currentMillis);
  // テストモード（段階的に回転速度を変えるデモ）を動かしたい場合はこちらを使用
  // if (isRandomRunning) servoTestRunningMode(currentMillis);

}
