#include <BluetoothSerial.h>

String deviceName = "ESP32";                    // Nama Bluetooth
BluetoothSerial SerialBT;

int pin[4] = {4, 17, 18, 21};                   // daftar pin output
int jumlahPin = sizeof(pin) / sizeof(pin[0]);   // jumlah isi array, bukan ukuran byte doang
int speed, kecerahan = 0;                       // speed = delay mode, kecerahan = level PWM buat fade
bool state;                                     // state buat blink on/off sekaligus arah fade naik/turun
String cmd, lastCmd;                            // cmd = input sekarang, lastCmd = mode terakhir

void setup() {
  SerialBT.begin(deviceName);                                   // Starting Bluetooth
  SerialBT.printf("\n============[STARTING]============\n");

  for (int i = 0; i < jumlahPin; i++) {          // inisialisasi semua pin sebagai output
    pinMode(pin[i], OUTPUT);
    digitalWrite(pin[i], HIGH);                  // indikator nyala bentar pas boot
    SerialBT.print("Pin Mode ");
    SerialBT.print(pin[i]);
    SerialBT.println(" OUTPUT");
    delay(100);
    digitalWrite(pin[i], LOW);                   // matiin lagi biar ga norak nyala terus
  }

  speed = 500;                                   // default speed awal
  SerialBT.print("Mode : ");
}

void loop() {
  if (SerialBT.available()) {                      // kalau ada input SerialBT
    cmd = SerialBT.readString();                   // baca semua input
    cmd.trim();                                  // buang enter/spasi sampah
    cmd.toLowerCase();                           // biar ga ribet case-sensitive

    SerialBT.printf("\nMode : ");
    SerialBT.println(cmd);
  }

  if (cmd == "1" || cmd == "on") {              // mode 1 = semua nyala
    resetpin();                                  // reset dulu kalau sebelumnya fade
    for (int i = 0; i < jumlahPin; i++) {
      digitalWrite(pin[i], HIGH);
    }
    lastCmd = cmd;                               // simpan mode terakhir
    cmd = "";                                    // kosongin biar ga dieksekusi terus

  } else if (cmd == "2" || cmd == "off") {      // mode 2 = semua mati
    resetpin();                                  // reset dulu kalau sebelumnya fade
    for (int i = 0; i < jumlahPin; i++) {
      digitalWrite(pin[i], LOW);
    }
    lastCmd = cmd;                               // simpan mode terakhir
    cmd = "";

  } else if (cmd == "3") {                      // mode 3 = blink
    resetpin();                                  // reset dulu kalau sebelumnya fade
    blink(speed);                                // jalanin blink sesuai speed
    lastCmd = cmd;                               // simpan mode terakhir

  } else if (cmd == "4") {                      // mode 4 = fade
    fade(speed * 3);                             // fade butuh timing lebih halus
    lastCmd = cmd;

  } else if (cmd == "0") {                      // mode 0 = ganti speed
    if (speed == 500) {
      speed = 300;
    } else if (speed == 300) {
      speed = 150;
    } else if (speed == 150) {
      speed = 1000;
    } else {
      speed = 500;
    }

    SerialBT.printf("\nSpeed : \"%d\"", speed);
    cmd = lastCmd;                               // balik ke mode sebelumnya, ga diem bego
  }
}

void resetpin() {
  if (lastCmd == "4" && cmd != "4") {           // kalau sebelumnya fade terus pindah mode
    kecerahan = 0;                               // reset brightness
    state = HIGH;                                // reset arah fade ke naik

    for (int i = 0; i < jumlahPin; i++) {
      ledcDetach(pin[i]);                        // lepas PWM dari pin
      pinMode(pin[i], OUTPUT);                   // balikin jadi digital biasa
      digitalWrite(pin[i], LOW);                 // pastiin mati bersih
    }
  }
}

void blink(int waktu) {
  static unsigned long waktuAwal;
  unsigned long waktuSekarang = millis();

  if (waktuSekarang - waktuAwal >= waktu) {      // non-blocking delay
    state = !state;                              // toggle nyala/mati

    for (int i = 0; i < jumlahPin; i++) {
      digitalWrite(pin[i], state);
    }

    waktuAwal = waktuSekarang;
  }
}

void fade(int waktu) {
  static unsigned long waktuAwal;
  unsigned long waktuSekarang = millis();

  if (waktuSekarang - waktuAwal >= (waktu / 255)) {   // timing fade per step
    for (int i = 0; i < jumlahPin; i++) {
      analogWrite(pin[i], kecerahan);                 // set PWM semua pin
    }

    if (kecerahan >= 255) {
      state = LOW;                                    // mentok atas, turun
    } else if (kecerahan <= 0) {
      state = HIGH;                                   // mentok bawah, naik
    }

    if (state) {
      kecerahan++;
    } else {
      kecerahan--;
    }

    waktuAwal = waktuSekarang;
  }
}
