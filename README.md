# m5stack-atom-servo-tester-360

## 概要

M5StackのAtomシリーズから360度サーボモーターを1台稼働させるテストプログラムです。

180度と360度を併用するサンプルプログラムをもとに作成したものとなります[stack-chan-tester-180and360](https://github.com/u-tanick/stack-chan-tester-180and360)。

## 使用ライブラリ

詳細は `platformio.ini` を参照してください。

- mongonta0716/stackchan-arduino
- fastled/FastLED@^3.7.5
- h2zero/NimBLE-Arduino@^1.4.2

- その他
  - サーボモーターの制御は `ServoEasing` を使用しています。

## 想定サーボモーター

- 360度回転サーボモーター
  - [TowerPro SG90-HV](https://akizukidenshi.com/catalog/g/g114382/)
  - [FEETECH FS90R](https://akizukidenshi.com/catalog/g/g113206/)

使用するサーボモーター毎にパラメータを調整が必要ですが、`main.cpp の59行目付近` のコメント／コメントアウトで、切り替えることができます。

``` cpp
// #define USE_Servo_TowerPro
#define USE_Servo_Feetech360
// #define USE_Servo_M5Stack
```

サーボモーターは、[HY4pServo](https://www.switch-science.com/products/6922) を使用してGroveケーブル経由でAtom製品に接続している想定です。

## 想定M5Stack製品

詳細は `platformio.ini` を参照してください。

- m5stack-atom
  - atom系の機種だとおおむね動くはずです
  - ただし汎用に実装しているため、S3などのLCDディスプレイ付きの機種の場合でも特に画面には何も表示されません。

## 提供機能

- Aボタン
  - ランダム駆動モード ON/OFF
  - サーボの回転角度や回転速度をランダムに変化させるモードです。
