#include <Arduino.h>
#include <U8X8lib.h>

class Millionaire
{ //klasa Milionerzy
private:
  struct poolOfQuestions
  { //struktura zawiera treści pytań, odpowiedzi i poprawnej odpowiedzi
    String questionAndAnswers[5];
    int correct;
  };
  unsigned long time, rememberedTime; //zmienne służące do odliczania czasu
  uint8_t audience[24] = {0x00, 0x60, 0x16, 0x09, 0x09, 0x16, 0x60, 0x00, 0x00, 0xC0, 0x2C, 0x12, 0x12, 0x2C, 0xC0, 0x00, 0x00, 0x60, 0x16, 0x09, 0x09, 0x16, 0x60, 0x00}; //utworzenie własnego symbolu publiczności;
  bool tips[3]; //tablica, która zawiera informacje o użytych kołach ratunkowych
  byte position; //zmienna do określania pozycji kursora na wyświetlaczu
  poolOfQuestions pool[12]; //deklaracja struktury pytań, ich utworzenie
  byte setPosition()
  { //funkcja, która określa położenie kursora (w pionie) na wyświetlaczu (który wiersz)
    if (analogRead(A0) > 800)
    { //jeśli przesuwam joystick w dół
      if (this->position == 5)
      {
        this->position = 7;
      }
      else if (this->position == 7)
      {
        this->position = 2;
      }
      else {
        this->position++;
      }
    }
    else if (analogRead(A0) < 700)
    { //jeśli przesuwam joystick w górę
      if (this->position == 2)
      {
        this->position = 7;
      }
      else if (this->position == 7) {
        this->position = 5;
      }
      else
      {
        this->position--;
      }
    }
    delay(100);
    return this->position;
  }

  void addSpace(int i)
  { //dodaj spację na początku każdego wiersza tekstu, gdy przekracza on 16 znaków, w celu lepszego wizualnego efektu wyświetlania go na OLED-zie
    String space = " ";
    for (int j = 0; j <= 4; j++)
    {
      if (this->pool[i].questionAndAnswers[j].length() > 16)
      {                                                    //jeśli liczba znaków większa od 16 (pytanie)
        space.concat(this->pool[i].questionAndAnswers[j]); //to do spacji dodaj cały ciąg
        this->pool[i].questionAndAnswers[j] = space;       //całość zapisz jako tekst do wyświetlenia
        space = " ";                                       //i znowu ustaw spację
      }
    }
  }

public:
  Millionaire()
  { //konstruktor klasy Milionerzy
    for (int i = 0; i <= 2; i++)
    {
      this->tips[i] = true; //na początku koła ratunkowe są dostępne
    }
    this->position = 1;
    this->time = 0;
    this->rememberedTime = 0;
    //oled = *new U8X8_SH1106_128X64_NONAME_HW_I2C(U8X8_PIN_NONE, 15, 14); //deklaracja obiektu dla klasy obsługującej wybrany wyświetlacz OLED
  }

  void setPosition(byte position)
  { //reset pozycji po skończonej grze
    this->position = position;
  }

  void setTips(int i, bool tip)
  { //włączenie/wyłączenie kół ratunkowych
    this->tips[i] = tip;
  }

  void saveDataInStruct(String question, String A, String B, String C, String D, String correct, int number)
  { //ustaw pytania wzięte losowo do gry z karty pamięci

    //wylosuj kolejność wyświetlania odpowiedzi
    randomSeed(analogRead(PA1));
    int numbers[4] = {(random(1,5)),0,0,0}; //wylosuj liczbę z zakresu 1-4 i od razu umieść ją na pierwszym miejscu tabeli
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

  int getCorrectAnswer(int i)
  { //pobierz wartość poprawnej odpowiedzi dla danego pytania
    return this->pool[i].correct;
  }

  byte getPosition() {
    return this->position;
  }

  //wyświetl odpowiednio ustawione dane
  void showQuestionAndAnswers(int q, U8X8_SH1106_128X64_NONAME_HW_I2C oled)
  {
    byte position = setPosition(); //pobierz wartość pozycji, której tekst ma być podkreślony (czyli wybrany przez użytkownika)
    if (this->tips[0])
    { //jeśli dostępne, to pokaż symbol pytania do publiczności
      oled.drawTile(0, 0, 3, audience);
    }
    if (this->tips[1])
    { //jeśli dostępne, to pokaż symbol pół na pół
      oled.setFont(u8x8_font_chroma48medium8_r);
      oled.drawString(6, 0, "50:50");
    }
    if (this->tips[2])
    { //jeśli dostępne to pokaż symbol pytania do przyjaciela
      oled.setFont(u8x8_font_open_iconic_thing_1x1);
      oled.drawGlyph(14, 0, 0x0052);
    }
    oled.setFont(u8x8_font_chroma48medium8_r);
    this->time = millis();
    if (this->time - this->rememberedTime >= 150UL)
    { //co 150ms aktualizuj wyświetlane dane (czyli podkreślenia w odpowiednim miejscu i przesuwaj treści pytań/odpowiedzi)
      this->rememberedTime = this->time;
      for (int i = 0; i <= 4; i++)
      {
        if (this->pool[q].questionAndAnswers[i].length() > 16)
        {                                                                                                  //jeśli powyżej 16 znaków
          this->pool[q].questionAndAnswers[i].concat(this->pool[q].questionAndAnswers[i].substring(0, 1)); //to do istniejącej treści dodaj pierwszy znak
          this->pool[q].questionAndAnswers[i].remove(0, 1);                                                //i następnie usuń ten pierwszy znak - efekt przesuwania napisu
        }
      }

      for (int i = 1; i <= 7; i++)
      {                             //tylko
        oled.setCursor(0, i); //ustaw kursor na początku kolejnego wiersza
        if (i == position && i != 1)
        {                               //jeśli wybrany analogiem wiersz równy z tym, co akurat ma być zapisany
          oled.setInverseFont(1); //to ustaw podświetlenie wiersza
        }
        switch (i) //wyświetl odpowiednie dane
        {
        case 1:
          oled.print(this->pool[q].questionAndAnswers[0].substring(0, 16)); //wyświetl pierwsze 16 znaków danego tekstu
          break;
        case 2:
          oled.print(this->pool[q].questionAndAnswers[1].substring(0, 16));
          break;
        case 3:
          oled.print(this->pool[q].questionAndAnswers[2].substring(0, 16));
          break;
        case 4:
          oled.print(this->pool[q].questionAndAnswers[3].substring(0, 16));
          break;
        case 5:
          oled.print(this->pool[q].questionAndAnswers[4].substring(0, 16));
          break;
        case 6:
          break;
        case 7:
          oled.setCursor(3, i);
          oled.print("REZYGNACJA");
          break;
        }
        oled.setInverseFont(0); //wyłącz podświetlenie
      }
    }
  }
};