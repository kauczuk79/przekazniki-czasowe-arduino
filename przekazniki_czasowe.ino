#include <PCF8574.h>
#include <Adafruit_GFX.h>
#include <gfxfont.h>
#include <Adafruit_PCD8544.h>
#include <DS1302RTC.h>

#define RTC_CE 2
#define RTC_IO 3
#define RTC_SCLK 4

#define LCD_RST 5
#define LCD_CE 6
#define LCD_DC 7
#define LCD_DIN 8
#define LCD_CLK 9

#define PRZEKAZNIK_1_EXPANDER 0
#define PRZEKAZNIK_2_EXPANDER 1
#define PRZEKAZNIK_3_EXPANDER 2
#define PRZEKAZNIK_4_EXPANDER 3
#define PRZYCISK_WYBORU_EXPANDER 4
#define PRZYCISK_PLUS_EXPANDER 5
#define PRZYCISK_MINUS_EXPANDER 6

DS1302RTC RTC(RTC_CE, RTC_IO, RTC_SCLK);
Adafruit_PCD8544 display = Adafruit_PCD8544(LCD_CLK, LCD_DIN, LCD_DC, LCD_CE, LCD_RST);
PCF8574 expander;

enum Tryb { TRYB_A = 0, TRYB_B = 1, TRYB_C = 2, TRYB_D = 3};

struct Przekaznik {
  Tryb tryb;
  int T1;
  int T2;
  byte stan;
  long nastepna_zmiana;
};

Przekaznik przekaznik[4];

void ustaw_rtc() {
  int aktualny_czas = now();
  if(RTC.haltRTC()) {
    Serial.println("Clock stopped!");
  } else {
    Serial.println("Clock working");
  }
  if(RTC.writeEN()) {
    Serial.println("Write allowed");
  } else {
    Serial.println("Write protected");
  }
  delay(2000);
  setSyncProvider(RTC.get);
  if(timeStatus() == timeSet) {
    Serial.println("OK");
    czytajRam();
  } else {
    Serial.println("FAIL!");
    for (int i = 0; i < 4; i++) {
      przekaznik[i].tryb = (Tryb)i;
      przekaznik[i].T1 = 10;
      przekaznik[i].T2 = 2;
      przekaznik[i].stan = 0;
      if(przekaznik[i].tryb == TRYB_B || przekaznik[i].tryb == TRYB_D) {
        przekaznik[i].stan = 1;
      }
      przekaznik[i].nastepna_zmiana = aktualny_czas + przekaznik[i].T1;
    }
  }
}

void czytajRam() {
  for(byte adres = 0; adres < 4; adres++) {
    przekaznik[adres].tryb = (Tryb)RTC.readRTC(adres);
    Serial.println(przekaznik[adres].tryb);
  }
  for(byte adres = 4; adres < 8; adres++) {
    przekaznik[adres - 4].T1 = RTC.readRTC(adres);
  }
  for(byte adres = 8; adres < 12; adres++) {
    przekaznik[adres - 8].T2 = RTC.readRTC(adres);
  }
}

void zapiszRam() {
  for(byte adres = 0; adres < 4; adres++) {
    RTC.writeRTC(adres, (uint8_t) przekaznik[adres].tryb);
  }
  for(byte adres = 4; adres < 8; adres++) {
    RTC.writeRTC(adres, (uint8_t) przekaznik[adres - 4].T1);
  }
  for(byte adres = 8; adres < 12; adres++) {
    RTC.writeRTC(adres, (uint8_t) przekaznik[adres - 8].T2);
  }
}

void setup() {
  Serial.begin(9600);
  expander.begin(0x20);
  expander.pinMode(PRZEKAZNIK_1_EXPANDER, OUTPUT);
  expander.pinMode(PRZEKAZNIK_2_EXPANDER, OUTPUT);
  expander.pinMode(PRZEKAZNIK_3_EXPANDER, OUTPUT);
  expander.pinMode(PRZEKAZNIK_4_EXPANDER, OUTPUT);
  expander.pinMode(PRZYCISK_WYBORU_EXPANDER, INPUT);
  expander.pinMode(PRZYCISK_PLUS_EXPANDER, INPUT);
  expander.pinMode(PRZYCISK_MINUS_EXPANDER, INPUT);
  expander.pullUp(PRZYCISK_WYBORU_EXPANDER);
  expander.pullUp(PRZYCISK_PLUS_EXPANDER);
  expander.pullUp(PRZYCISK_MINUS_EXPANDER);
  ustaw_rtc();
  display.begin();
  display.setContrast(50);
}

int aktualnie_wyswietlany = 0;
int aktualny_parametr = 0;

void czarneTlo(char* tekst) {
  display.setTextColor(WHITE, BLACK);
  display.println(tekst);
  display.setTextColor(BLACK);
}

void print2digits(int number) {
  if (number >= 0 && number < 10) {
    display.print("0");
  }
  display.print(number);
}

void wyswietlMenu() {
  display.clearDisplay();
  if(aktualny_parametr == 0) {
    display.setTextColor(WHITE, BLACK);
    display.print("Przekaznik: ");
    display.println(aktualnie_wyswietlany);
    display.setTextColor(BLACK);
  } else {
    display.print("Przekaznik: ");
    display.println(aktualnie_wyswietlany);
  }
  if(aktualny_parametr == 1) {
    display.setTextColor(WHITE, BLACK);
  }
  display.print("Tryb:       ");
  switch (przekaznik[aktualnie_wyswietlany].tryb) {
    case TRYB_A: 
      display.println("A");
      break;
    case TRYB_B:
      display.println("B");
      break;
    case TRYB_C: 
      display.println("C");
      break;
    case TRYB_D:
      display.println("D");
      break;
  }
  if(aktualny_parametr == 1) {
    display.setTextColor(BLACK);
  }
  if(aktualny_parametr == 2) {
    display.setTextColor(WHITE, BLACK);
    display.print("T1:      ");
    display.print(przekaznik[aktualnie_wyswietlany].T1);
    display.println("s");
    display.setTextColor(BLACK);
  } else {
    display.print("T1:      ");
    display.print(przekaznik[aktualnie_wyswietlany].T1);
    display.println("s");
  }
  if(aktualny_parametr == 3) {
    display.setTextColor(WHITE, BLACK);
    display.print("T2:      ");
    display.print(przekaznik[aktualnie_wyswietlany].T2);
    display.println("s");
    display.setTextColor(BLACK);
  } else {
    display.print("T2:      ");
    display.print(przekaznik[aktualnie_wyswietlany].T2);
    display.println("s");
  }
  print2digits(hour());
  display.print(":");
  print2digits(minute());
  display.print(":");
  print2digits(second());
  display.println();
  //display.println(now());
  for(int i = 0; i < 4; i++) {
    display.print(przekaznik[i].stan);
  }
  display.display();
}

void aktualizujStany() {
  int aktualny_czas = now();
  for(int i = 0; i < 4; i++) {
    if(przekaznik[i].nastepna_zmiana <= aktualny_czas) {
      if(przekaznik[i].tryb == TRYB_A && przekaznik[i].stan == 0) {
        przekaznik[i].stan = 1;
      }
      if(przekaznik[i].tryb == TRYB_B && przekaznik[i].stan == 1) {
        przekaznik[i].stan = 0;
      }
      if(przekaznik[i].tryb == TRYB_C) {
        przekaznik[i].stan = (przekaznik[i].stan + 1) % 2;
        if(przekaznik[i].stan == 0) {
          przekaznik[i].nastepna_zmiana = aktualny_czas + przekaznik[i].T1;
        } else {
          przekaznik[i].nastepna_zmiana = aktualny_czas + przekaznik[i].T2;
        }
      }
      if(przekaznik[i].tryb == TRYB_D) {
        przekaznik[i].stan = (przekaznik[i].stan + 1) % 2;
        if(przekaznik[i].stan == 0) {
          przekaznik[i].nastepna_zmiana = aktualny_czas + przekaznik[i].T2;
        } else {
          przekaznik[i].nastepna_zmiana = aktualny_czas + przekaznik[i].T1;
        }
      }
    }
  }
}

void zmienStanPrzekaznikow() {
  if(przekaznik[0].stan == 1) {
    expander.digitalWrite(PRZEKAZNIK_1_EXPANDER, HIGH);
  } else {
    expander.digitalWrite(PRZEKAZNIK_1_EXPANDER, LOW);
  }
  if(przekaznik[1].stan == 1) {
    expander.digitalWrite(PRZEKAZNIK_2_EXPANDER, HIGH);
  } else {
    expander.digitalWrite(PRZEKAZNIK_2_EXPANDER, LOW);
  }
  if(przekaznik[2].stan == 1) {
    expander.digitalWrite(PRZEKAZNIK_3_EXPANDER, HIGH);
  } else {
    expander.digitalWrite(PRZEKAZNIK_3_EXPANDER, LOW);
  }
  if(przekaznik[3].stan == 1) {
    expander.digitalWrite(PRZEKAZNIK_4_EXPANDER, HIGH);
  } else {
    expander.digitalWrite(PRZEKAZNIK_4_EXPANDER, LOW);
  }
}

void czytajPrzyciski() {
  if(expander.digitalRead(PRZYCISK_WYBORU_EXPANDER) == LOW) {
    aktualny_parametr = (aktualny_parametr + 1) % 4;
  }
  if(expander.digitalRead(PRZYCISK_PLUS_EXPANDER) == LOW) {
    switch (aktualny_parametr) {
      case 0: //przekaznik
        aktualnie_wyswietlany = (aktualnie_wyswietlany + 1) % 4;
        break;
      case 1: //Tryb
        przekaznik[aktualnie_wyswietlany].tryb = (Tryb)((((int)przekaznik[aktualnie_wyswietlany].tryb) + 1) % 4);
        break;
      case 2: //T1
        przekaznik[aktualnie_wyswietlany].T1 = przekaznik[aktualnie_wyswietlany].T1 + 1;
        break;
      case 3: //T2
        przekaznik[aktualnie_wyswietlany].T2 = przekaznik[aktualnie_wyswietlany].T2 + 1;
        break;
    }
  }
  if(expander.digitalRead(PRZYCISK_MINUS_EXPANDER) == LOW) {
    switch (aktualny_parametr) {
      case 0: //przekaznik
        aktualnie_wyswietlany = ((aktualnie_wyswietlany - 1) + 4) % 4;
        break;
      case 1: //Tryb
        przekaznik[aktualnie_wyswietlany].tryb = (Tryb)((((int)przekaznik[aktualnie_wyswietlany].tryb) - 1 + 4) % 4);
        break;
      case 2: //T1
        if(przekaznik[aktualnie_wyswietlany].T1 > 1) {
          przekaznik[aktualnie_wyswietlany].T1 = przekaznik[aktualnie_wyswietlany].T1 - 1;
        }
        break;
      case 3: //T2
        if(przekaznik[aktualnie_wyswietlany].T2 > 1) {
          przekaznik[aktualnie_wyswietlany].T2 = przekaznik[aktualnie_wyswietlany].T2 - 1;
        }
        break;
    }
  }
}

void loop() {
  wyswietlMenu();
  aktualizujStany();
  zmienStanPrzekaznikow();
  czytajPrzyciski();
  zapiszRam();
  delay(125);
  //aktualny_parametr = (aktualny_parametr + 1) % 4;
}
