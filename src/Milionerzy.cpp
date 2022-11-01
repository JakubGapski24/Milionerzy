//załączenie niezbędnych bibliotek
#include <Arduino.h>
#include <U8X8lib.h>
#include <LiquidCrystal_I2C.h>

#define audienceDiode PA10 //dioda informująca o możliwości wyboru pytania do publiczności
#define fiftyFiftyDiode PB3 //dioda informująca o możliwości wyboru pół na pół
#define phoneDiode PC4 //dioda informująca o możliwości wyboru telefonu do przyjaciela
#define audienceSwitch PB4 //wybór pytania do publiczności
#define fiftyFiftySwitch PB10 //wybór pół na pół
#define phoneSwitch PA8 //wybór telefonu do przyjaciela

class Millionaire { //klasa Milionerzy
  private:
    struct poolOfQuestions { //struktura zawiera treści pytań, odpowiedzi i poprawnej odpowiedzi
      String questionAndAnswers[5];
      int correct;
    };
    unsigned long time, rememberedTime; //zmienne służące do odliczania czasu
    uint8_t audience[24] = {0x00, 0x60, 0x16, 0x09, 0x09, 0x16, 0x60, 0x00, 0x00, 0xC0, 0x2C, 0x12, 0x12, 0x2C, 0xC0, 0x00, 0x00, 0x60, 0x16, 0x09, 0x09, 0x16, 0x60, 0x00}; //utworzenie własnego symbolu publiczności;
    bool tips[3]; //tablica, która zawiera informacje o użytych kołach ratunkowych
    byte position; //zmienna do określania pozycji kursora na wyświetlaczu
    poolOfQuestions pool[12]; //deklaracja struktury pytań, ich utworzenie
    int toClear[2]; //tablica przechowująca informacje o wierszach do wyczyszczenia (zastosowanie pół na pół)
    byte setPosition() { //funkcja, która określa położenie kursora (podświetlenie danego wiersza) w zależności od ruszania joystickiem
      if (analogRead(A0) > 800) { //jeśli przesuwam joystick w dół
        switch (this->position) {
          case 1:
            if (this->toClear[0] == 2 && this->toClear[1] == 3) {
              this->position += 3;
            }
            else if (this->toClear[0] == 2) {
              this->position += 2;
            }
            else {
              this->position++;
            }
            break;
          case 2:
            if (this->toClear[0] == 3 && this->toClear[1] == 4) {
              this->position += 3;
            }
            else if (this->toClear[0] == 3) {
              this->position += 2;
            }
            else {
              this->position++;
            }
            break;
          case 3:
            if (this->toClear[0] == 4 && this->toClear[1] == 5) {
              this->position = 7;
            }
            else if (this->toClear[0] == 4 || this->toClear[1] == 4) {
              this->position += 2;
            }
            else {
              this->position++;
            }
            break;
          case 4:
            if (this->toClear[1] == 5) {
              this->position = 7;
            }
            else {
              this->position++;
            }
            break;
          case 5:
            this->position = 7;
            break;
          case 7:
            if (this->toClear[0] == 2 && this->toClear[1] == 3) {
              this->position = 4;
            }
            else if (this->toClear[0] == 2) {
              this->position = 3;
            }
            else {
              this->position = 2;
            }
            break;
        }
      }
      else if (analogRead(A0) < 700) { //jeśli przesuwam joystick w górę
        switch (this->position) {
          case 1: case 2:
            this->position = 7;
            break;
          case 3:
            if (this->toClear[0] == 2) {
              this->position = 7;
            }
            else {
              this->position--;
            }
            break;
          case 4:
            if (this->toClear[0] == 2 && this->toClear[1] == 3) {
              this->position = 7;
            }
            else if (this->toClear[0] == 3 || this->toClear[1] == 3) {
              this->position -= 2;
            }
            else {
              this->position--;
            }
            break;
          case 5:
            if (this->toClear[0] == 3 && this->toClear[1] == 4) {
              this->position -= 3;
            }
            else if (this->toClear[1] == 4) {
              this->position -= 2;
            }
            else {
              this->position--;
            }
            break;
          case 7:
            if (this->toClear[0] == 4 && this->toClear[1] == 5) {
              this->position = 3;
            }
            else if (this->toClear[1] == 5) {
              this->position = 4;
            }
            else {
              this->position = 5;
            }
            break;
        }
      }
      delay(100);
      return this->position;
    }

    void addSpace(int i) { //dodaj spację na początku każdego wiersza tekstu, gdy przekracza on 16 znaków, w celu lepszego wizualnego efektu wyświetlania go na OLED-zie
      String space = " ";
      for (int j = 0; j <= 4; j++) {
        if (this->pool[i].questionAndAnswers[j].length() > 16) { //jeśli liczba znaków większa od 16 (pytanie)
          space.concat(this->pool[i].questionAndAnswers[j]); //to do spacji dodaj cały ciąg
          this->pool[i].questionAndAnswers[j] = space; //całość zapisz jako tekst do wyświetlenia
          space = " "; //i znowu ustaw spację
        }
      }
    }

  public:
    Millionaire() { //konstruktor klasy Milionerzy
      for (int i = 0; i <= 2; i++) {
        this->tips[i] = true; //na początku koła ratunkowe są dostępne
      }
      this->position = 1;
      this->time = 0;
      this->rememberedTime = 0;
      for (int i=0; i<=1; i++) {
        this->toClear[i] = 0;
      }
    }

    void setPosition(byte position) { //reset pozycji po skończonej grze
      this->position = position;
    }

    void setTips(int i, bool tip) { //włączenie/wyłączenie kół ratunkowych
      this->tips[i] = tip;
    }

    void saveDataInStruct(String question, String A, String B, String C, String D, String correct, int number) { //ustaw pytania wzięte losowo do gry z karty pamięci
      //wylosuj kolejność wyświetlania odpowiedzi
      int numbers[4] = {(random(1,5)),0,0,0}; //wylosuj liczbę z zakresu 1-4 i od razu umieść ją na pierwszym miejscu tablicy
      for (int i=1; i<=2; i++) {
        numbers[i] = random(1,5); //wylosuj następną liczbę z tego przedziału
        for (int j=0; j<i; j++) {
          while (numbers[i] == numbers[j]) { //jeśli jest ona taka sama, jak któraś z liczb umieszczonych już w tej tablicy, to losuj ponownie
            numbers[i] = random(1,5);
          }
        }
      }
      numbers[3] = (10 - numbers[0] - numbers[1] - numbers[2]);

      //ustawianie treści poszczególnych pytań i odpowiedzi
      this->pool[number].questionAndAnswers[0] = question;
      this->pool[number].questionAndAnswers[1] = "A.";
      this->pool[number].questionAndAnswers[2] = "B.";
      this->pool[number].questionAndAnswers[3] = "C.";
      this->pool[number].questionAndAnswers[4] = "D.";

      for (int i=1; i<=4; i++) {
        switch (numbers[i-1]) {
          case 1:
            this->pool[number].questionAndAnswers[i].concat(A);
            break;
          case 2:
            this->pool[number].questionAndAnswers[i].concat(B);
            break;
          case 3:
            this->pool[number].questionAndAnswers[i].concat(C);
            break;
          case 4:
            this->pool[number].questionAndAnswers[i].concat(D);
            break;
        }
      }

      //ustalenie prawidłowej odpowiedzi
      if (this->pool[number].questionAndAnswers[1].substring(2,this->pool[number].questionAndAnswers[1].length()).equals(correct)) {
        this->pool[number].correct = 2;
      }
      else if (this->pool[number].questionAndAnswers[2].substring(2,this->pool[number].questionAndAnswers[2].length()).equals(correct)) {
        this->pool[number].correct = 3;
      }
      else if (this->pool[number].questionAndAnswers[3].substring(2,this->pool[number].questionAndAnswers[3].length()).equals(correct)) {
        this->pool[number].correct = 4;
      }
      else if (this->pool[number].questionAndAnswers[4].substring(2,this->pool[number].questionAndAnswers[4].length()).equals(correct)) {
        this->pool[number].correct = 5;
      }

      //dodanie spacji do prawidłowego wyświetlania tekstu
      addSpace(number);
    }

    int getCorrectAnswer(int i) { //pobierz wartość poprawnej odpowiedzi dla danego pytania
      return this->pool[i].correct;
    }

    void setToClear() { //skasuj informację o wierszach do wyczyszczenia (po wykorzystaniu pół na pół lub przy nowej grze)
      toClear[0] = 0;
      toClear[1] = 0;
    }

    byte getPosition() { //pobierz akutalną wartość "pozycji" joysticka
      return this->position;
    }

    //wyświetl odpowiednio ustawione dane
    void showQuestionAndAnswers(int q, U8X8_SH1106_128X64_NONAME_HW_I2C oled) {
      byte position = setPosition(); //pobierz wartość pozycji, której tekst ma być podkreślony (czyli wybrany przez użytkownika)
      if (this->tips[0]) { //jeśli dostępne, to pokaż symbol pytania do publiczności
        oled.drawTile(0, 0, 3, audience);
      }
      if (this->tips[1]){ //jeśli dostępne, to pokaż symbol pół na pół
        oled.setFont(u8x8_font_chroma48medium8_r);
        oled.drawString(6, 0, "50:50");
      }
      if (this->tips[2]) { //jeśli dostępne to pokaż symbol pytania do przyjaciela
        oled.setFont(u8x8_font_open_iconic_thing_1x1);
        oled.drawGlyph(14, 0, 0x0052);
      }
      oled.setFont(u8x8_font_chroma48medium8_r);
      this->time = millis();
      if (this->time - this->rememberedTime >= 150UL) { //co 150ms aktualizuj wyświetlane dane (czyli podkreślenia w odpowiednim miejscu i przesuwaj treści pytań/odpowiedzi)
        this->rememberedTime = this->time;
        for (int i = 0; i <= 4; i++) {
          if (this->pool[q].questionAndAnswers[i].length() > 16) { //jeśli powyżej 16 znaków
            this->pool[q].questionAndAnswers[i].concat(this->pool[q].questionAndAnswers[i].substring(0, 1)); //to do istniejącej treści dodaj pierwszy znak
            this->pool[q].questionAndAnswers[i].remove(0, 1); //i następnie usuń ten pierwszy znak - efekt przesuwania napisu
          }
        }

        for (int i = 1; i <= 7; i++) { //wyświetlanie każdego wiersza OLED'a
          oled.setCursor(0, i); //ustaw kursor na początku kolejnego wiersza
          if (i == position && i != 1) { //jeśli wybrany joystickiem wiersz równy z tym, co akurat ma być wyświetlony
            oled.setInverseFont(1); //to ustaw podświetlenie wiersza
          }
          switch (i) { //wyświetl odpowiednie dane
            case 1:
              oled.print(this->pool[q].questionAndAnswers[0].substring(0, 16)); //wyświetl pierwsze 16 znaków danego tekstu (pytanie)
              break;
            case 2:
              if (this->toClear[0] != 2) { //jeśli nie została usunięta przez wykorzystanie 50:50, to
                oled.print(this->pool[q].questionAndAnswers[1].substring(0, 16)); //wyświetl odpowiedź A
              }
              break;
            case 3:
              if (this->toClear[0] != 3 && this->toClear[1] != 3) { //jeśli nie została usunięta przez wykorzystanie 50:50, to
                oled.print(this->pool[q].questionAndAnswers[2].substring(0, 16)); //wyświetl odpowiedź B
              }
              break;
            case 4:
              if (this->toClear[0] != 4 && this->toClear[1] != 4) { //jeśli nie została usunięta przez wykorzystanie 50:50, to
                oled.print(this->pool[q].questionAndAnswers[3].substring(0, 16)); //wyświetl odpowiedź C
              }
              break;
            case 5:
              if (this->toClear[1] != 5) { //jeśli nie została usunięta przez wykorzystanie 50:50, to
                oled.print(this->pool[q].questionAndAnswers[4].substring(0, 16)); //wyświetl odpowiedź D
              }
              break;
            case 6: //pusty wiersz
              break;
            case 7:
              oled.setCursor(3, i);
              oled.print("REZYGNACJA"); //wyświetl informację o możliwości zrezygnowania z gry na aktualnym etapie
              break;
          }
          oled.setInverseFont(0); //wyłącz podświetlenie
        }
      }
    }

    //użycie pytania do publiczności
    void questionToAudience(U8X8_SH1106_128X64_NONAME_HW_I2C oled, LiquidCrystal_I2C lcd, int i) {
      if (digitalRead(audienceSwitch) == LOW && this->tips[0]) { //jeśli wciśnięty przycisk pytania do publicznośći i nie został on już użyty
        digitalWrite(audienceDiode, LOW);
        setTips(0, false); //zapamiętaj to, że w tej grze użyto już tego koła
        oled.clearLine(0);
        lcd.clear();
        //ustalenie procentów "głosowania" publiczności
        int audienceAnswers[4] = {0,0,0,0};
        if (i <= 5) { //dla pytań do 20000 włącznie, pokaż największy procent dla prawidłowej odpowiedzi
          switch (this->pool[i].correct) {
            case 2:
              audienceAnswers[0] = random(51,61);
              audienceAnswers[1] = random(0,(100-audienceAnswers[0]));
              audienceAnswers[2] = random(0,(100-audienceAnswers[1]-audienceAnswers[0]));
              audienceAnswers[3] = 100 - audienceAnswers[0] - audienceAnswers[1] - audienceAnswers[2];
              break;
            case 3:
              audienceAnswers[1] = random(51,61);
              audienceAnswers[0] = random(0,(100-audienceAnswers[1]));
              audienceAnswers[2] = random(0,(100-audienceAnswers[1]-audienceAnswers[0]));
              audienceAnswers[3] = 100 - audienceAnswers[0] - audienceAnswers[1] - audienceAnswers[2];
              break;
            case 4:
              audienceAnswers[2] = random(51,61);
              audienceAnswers[0] = random(0,(100-audienceAnswers[2]));
              audienceAnswers[1] = random(0,(100-audienceAnswers[2]-audienceAnswers[0]));
              audienceAnswers[3] = 100 - audienceAnswers[0] - audienceAnswers[1] - audienceAnswers[2];
              break;
            case 5:
              audienceAnswers[3] = random(51,61);
              audienceAnswers[0] = random(0, (100-audienceAnswers[3]));
              audienceAnswers[1] = random(0,(100-audienceAnswers[3]-audienceAnswers[0]));
              audienceAnswers[2] = 100 - audienceAnswers[0] - audienceAnswers[1] - audienceAnswers[3];
              break;
          }
        }
        else { //dla pytań powyżej 20000 pokaż losowe procenty, ale jedna z dwóch najwyższych wartości dla prawidłowej odpowiedzi
          switch (this->pool[i].correct) {
            case 2:
              audienceAnswers[0] = random(40,46);
              audienceAnswers[2] = random(40,46);
              audienceAnswers[3] = random(0,(100-audienceAnswers[0]-audienceAnswers[2]));
              audienceAnswers[1] = 100 - audienceAnswers[0] - audienceAnswers[2] - audienceAnswers[3];
              break;
            case 3:
              audienceAnswers[1] = random(40,46);
              audienceAnswers[0] = random(40,46);
              audienceAnswers[2] = random(0,(100-audienceAnswers[0]-audienceAnswers[1]));
              audienceAnswers[3] = 100 - audienceAnswers[0] - audienceAnswers[1] - audienceAnswers[2];
              break;
            case 4:
              audienceAnswers[2] = random(40,46);
              audienceAnswers[3] = random(40,46);
              audienceAnswers[1] = random(0,(100-audienceAnswers[2]-audienceAnswers[3]));
              audienceAnswers[0] = 100 - audienceAnswers[1] - audienceAnswers[2] - audienceAnswers[3];
              break;
            case 5:
              audienceAnswers[3] = random(40,46);
              audienceAnswers[0] = random(40,46);
              audienceAnswers[1] = random(0,(100-audienceAnswers[0]-audienceAnswers[3]));
              audienceAnswers[2] = 100 - audienceAnswers[0] - audienceAnswers[1] - audienceAnswers[3];
              break;
          }
        }
        //wyświetlenie wyników "głosowania"
        lcd.setCursor(0,0);
        lcd.print("A:");
        lcd.print(audienceAnswers[0]);
        lcd.print("%");
        lcd.setCursor(0,1);
        lcd.print("B:");
        lcd.print(audienceAnswers[1]);
        lcd.print("%");
        lcd.setCursor(0,2);
        lcd.print("C:");
        lcd.print(audienceAnswers[2]);
        lcd.print("%");
        lcd.setCursor(0,3);
        lcd.print("D:");
        lcd.print(audienceAnswers[3]);
        lcd.print("%");
      }
    }

    //użycie pół na pół
    void fiftyFifty(U8X8_SH1106_128X64_NONAME_HW_I2C oled, int i) {
      if (digitalRead(fiftyFiftySwitch) == LOW && this->tips[1]) { //jeśli wciśnięty przycisk 50:50 i nie został on wcześniej użyty
        digitalWrite(fiftyFiftyDiode, LOW);
        setTips(1, false);
        oled.clearLine(0);
        //losowe wybranie dwóch błędnych odpowiedzi
        for (int j=0; j<=1; j++) {
          do {
            this->toClear[j] = random(2,6);
          }
          while (this->toClear[j] == this->pool[i].correct || this->toClear[0] == this->toClear[1]); //sprawdzenie, czy wartości te się nie powtarzają i czy nie zostanie usunięta poprawna odpowiedź
        }

        //sortowanie tej tablicy w celu łatwiejszego określania pozycji przy ruszaniu joystickiem
        if (this->toClear[0] > this->toClear[1]) {
          int x = this->toClear[0];
          this->toClear[0] = this->toClear[1];
          this->toClear[1] = x;
        }

        //wymazanie błędnych odpowiedzi z ekranu
        for (int j=0; j<=1; j++) {
          switch (this->toClear[j]) {
            case 2:
              oled.clearLine(2);
              break;
            case 3:
              oled.clearLine(3);
              break;
            case 4:
              oled.clearLine(4);
              break;
            case 5:
              oled.clearLine(5);
              break;
          }
        }
        this->position = 1;
      }
    }

    //użycie telefonu do przyjaciela
    void phone(U8X8_SH1106_128X64_NONAME_HW_I2C oled, LiquidCrystal_I2C lcd, int i) {
      if (digitalRead(phoneSwitch) == LOW && this->tips[2]) { //jeżeli wciśnięto przycisk telefonu i nie został on wcześniej użyty
        digitalWrite(phoneDiode, LOW);
        setTips(2, false); //zapamiętaj fakt, że użyto już telefonu do przyjaciela
        oled.clearLine(0);
        lcd.clear();
        lcd.setCursor(3,1);

        //wyświetlenie poprawnej odpowiedzi
        lcd.print("Poprawna odp.:");
        lcd.setCursor(9,2);
        switch (this->pool[i].correct) {
          case 2:
            lcd.print("A");
            break;
          case 3:
            lcd.print("B");
            break;
          case 4:
            lcd.print("C");
            break;
          case 5:
            lcd.print("D");
            break;
        }
      }
    }
};