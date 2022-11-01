//****************************************Załączenie potrzebnych bibliotek**************************************************************
#include <Arduino.h>
#include <U8X8lib.h>
#include <SD.h>
#include <SPI.h>
#include <Milionerzy.cpp>
#include <WebConnection.cpp>
#include <LiquidCrystal_I2C.h>

//*************************************Przypisanie pinów sterownika do zdefiniowanych zmiennych*****************************************
#define confirm PA4 //potwierdzenie wyboru odpowiedzi
#define correct PB0 //dioda zielona - sygnalizuje, że wybrana odpowiedź jest poprawna
#define wrong PC1 //dioda czerwona - sygnalizuje, że wybrana odpowiedź jest błędna
#define marked PC0 //dioda żółta - sygnalizuje zaznaczenie wybranej przez joystick odpowiedzi
#define audienceDiode PA10 //dioda informująca o możliwości wyboru pytania do publiczności
#define fiftyFiftyDiode PB3 //dioda informująca o możliwości wyboru pół na pół
#define phoneDiode PC4 //dioda informująca o możliwości wyboru telefonu do przyjaciela
#define audienceSwitch PB4 //wybór pytania do publiczności
#define fiftyFiftySwitch PB10 //wybór pół na pół
#define phoneSwitch PA8 //wybór telefonu do przyjaciela

LiquidCrystal_I2C lcd(0x27, 20, 4); //inicjalizacja wyświetlacza LCD do obsługi kół ratunkowych - przypisanie adresu
U8X8_SH1106_128X64_NONAME_HW_I2C oled(U8X8_PIN_NONE, 15, 14); //stworzenie obiektu klasy obsługującej wybrany wyświetlacz OLED
Millionaire game = *new Millionaire(); //stworzenie obiektu klasy Milionerzy (obsługa gry)
byte showBasicSettings(int x, int y, const char *text1, int x2, int y2, const char *text2); //mechanizm wyświetlania tekstu do ustawień początkowych
void showChoiceMenu(byte position);  //wybór ustawień początkowych gry (aktualizacja bazy pytań i wybór rodzaju telefonu do przyjaciela)
byte getPosition(byte position); //ustalenie, który wiersz wyświetlacza ma być podświetlony (w tym wypadku odpowiedzi TAK lub NIE), dla ustawień początkowych
void updateDatabaseOnSD(WebConnection web); //zaktualizowanie danych na karcie SD
int takeNumberOfQuestionsFromSD(); //pobierz numer określający liczbę pytań z karty SD
int getPosition(int i); //pobierz pozycję do wyświetlania kwoty pytania
void showMainMenu(); //pokaż menu powitalne

void setup() { //ustawienie przypisanych wyjść w odpowiedni tryb
  pinMode(confirm, INPUT_PULLUP);
  pinMode(wrong, OUTPUT);
  pinMode(correct, OUTPUT);
  pinMode(marked, OUTPUT);
  pinMode(audienceDiode, OUTPUT);
  pinMode(fiftyFiftyDiode, OUTPUT);
  pinMode(phoneDiode, OUTPUT);
  pinMode(audienceSwitch, INPUT_PULLUP);
  pinMode(fiftyFiftySwitch, INPUT_PULLUP);
  pinMode(phoneSwitch, INPUT_PULLUP);
  digitalWrite(wrong, HIGH);
  digitalWrite(correct, HIGH);
  digitalWrite(marked, LOW);
  digitalWrite(audienceDiode, LOW);
  digitalWrite(fiftyFiftyDiode, LOW);
  digitalWrite(phoneDiode, LOW);

  //inicjalizacja wyświetlacza OLED i LCD i ustawienie czcionki dla OLED'a
  oled.begin();
  oled.setFont(u8x8_font_pcsenior_r);
  lcd.init(); //inicjalizacja wyświetlacza
  lcd.clear(); //wyczyszczenie ekranu
  lcd.backlight(); //włączenie podświetlenia LCD

  //wyświetl menu powitalne
  showMainMenu();

  //showBasicSettings(2,2,"Zwykla wersja",4,3,"telefonu?"); //wyświetlaj podaną treść w dwóch linijkach z podanymi współrzędnymi
  byte position = showBasicSettings(2, 2, "Aktualizowac", 2, 3, "baze pytan?");

  oled.clearDisplay();
  oled.setInverseFont(0);

  if (position == 5) { //jeśli chcesz aktualizować bazę pytań
    oled.drawString(2, 3, "Laczenie...");
    delay(1000);
    byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE}; //przypisanie adresu MAC
    WebConnection web = *new WebConnection(mac);
    if (web.isConnected()) { //sprawdzenie poprawności i unikalności adresu MAC i czy sterownik jest podłączony do serwera
      updateDatabaseOnSD(web); //aktualizuj dane na karcie SD
    }
  }

  randomSeed(analogRead(PA1)); //przygotowanie do losowania liczb
}

void loop() {
  int number = takeNumberOfQuestionsFromSD(); //pobierz z karty SD informację o ilości pytań (ile jest pytań za każdą kwotę)
  if (number == 0) { //jeśli pobrano wartość 0, to znaczy, że coś jest nie tak i należy zakończyć pracę programu
    oled.clearDisplay();
    oled.drawString(5,3,"Blad");
    return;
  }
  else {
    oled.clearDisplay();
    oled.drawString(2,3,"Ustawianie");
    oled.drawString(5,4,"pytan");
    int numberOfQuestion[12]; //przechowywanie losowych liczb z zakresu 1-pobrana liczba
    File file;

    for (int i=0; i<=11; i++) { //pobierz pytania z karty SD
      while (!SD.begin(4))
      { //sprawdzenie, czy karta SD jest umieszczona w slocie SD
        oled.clearDisplay();
        oled.setCursor(1, 3);
        oled.print("Wloz karte SD!");
      }
      numberOfQuestion[i] = random(1,(number+1)); //losowanie liczby z określonego wcześniej zakresu
      oled.clearLine(5);
      oled.setCursor(3,5);
      oled.print(i+1);
      oled.drawString(5,5," z 12");
      //określenie ścieżki, z której będzie pobrane pytanie (z którego pliku)
      String path = "M/";

      switch (i) {
        case 0:
          path.concat("500/");
          break;
        case 1:
          path.concat("1000/");
          break;
        case 2:
          path.concat("2000/");
          break;
        case 3:
          path.concat("5000/");
          break;
        case 4:
          path.concat("10000/");
          break;
        case 5:
          path.concat("20000/");
          break;
        case 6:
          path.concat("40000/");
          break;
        case 7:
          path.concat("75000/");
          break;
        case 8:
          path.concat("125000/");
          break;
        case 9:
          path.concat("250000/");
          break;
        case 10:
          path.concat("500000/");
          break;
        case 11:
          path.concat("1000000/");
          break;
      }
      path.concat(numberOfQuestion[i]);
      path.concat(".txt");

      String readData[6];

      while (readData[1] == readData[2] || readData[1] == readData[3] || readData[1] == readData[4] || readData[2] == readData[3] || readData[2] == readData[4] || readData[3] == readData[4]) { //w przypadku błędnego pobrania odpowiedzi (dublują się), pobieraj je tak długo, aż będzie ok
        if (SD.exists(path)) { //jeśli podany plik istnieje, to zapisz dane w odpowiednim miejscu
          int index = 0;
          file = SD.open(path);
          while (file.available()) { //dopóki są dane do odczytania
            readData[index] = file.readStringUntil('\n');
            index++;
            delay(75);
          }
          file.close();
        
        }
        else { //jeśli nie istnieje taki plik, to zakończ działanie programu
          oled.clearDisplay();
          oled.drawString(2,3,"Brak pliku");
          return;
        }
      }
      game.saveDataInStruct(readData[0], readData[1], readData[2], readData[3], readData[4], readData[5], i); //zapisz dane do puli pytań
    }

    oled.clearDisplay();
    game.setPosition(1);
    int currentWin = 0;
    int guarantedWin = 0;
    int values[12] = {500, 1000, 2000, 5000, 10000, 20000, 40000, 75000, 125000, 250000, 500000, 1000000}; //wartości do wyświetlenia za ile jest aktualne pytanie
    for (int i=0; i<=2; i++) {
      game.setTips(i, true); //ustaw koła ratunkowe jako do użycia
    }
    //zapal diody przypisane kołom ratunkowym i zgaś te od zaznaczonej odpowiedzi i odpowiedzi błędnej i poprawnej
    digitalWrite(audienceDiode, HIGH);
    digitalWrite(fiftyFiftyDiode, HIGH);
    digitalWrite(phoneDiode, HIGH);
    digitalWrite(marked, LOW);
    digitalWrite(correct, HIGH);
    digitalWrite(wrong, HIGH);

    //gra właściwa - wyświetlanie pytań
    for (int i=0; i<=11; i++) {
      if (game.getPosition() != 7) { //jeśli nie zrezygnowano z gry
        game.setToClear(); //wyczyść dane po użyciu 50:50
        //na OLED pokaż za ile jest dane pytanie
        game.setPosition(1);
        oled.drawString(3,3,"Pytanie za");
        oled.setCursor(getPosition(i),4);
        oled.print(values[i]);
        delay(1500);
        // a na LCD pokazuj aktualną wygraną
        lcd.clear();
        lcd.setCursor(2,1);
        lcd.print("Aktualna wygrana:");
        lcd.setCursor(8,2);
        lcd.print(currentWin);
        oled.clearDisplay();
      }
      digitalWrite(marked, LOW); //gaś diody odpowiedzialne za informację o udzielanych odpowiedziach
      digitalWrite(wrong, HIGH);
      digitalWrite(correct, HIGH);
      while (digitalRead(confirm) == HIGH || game.getPosition() == 1) { //dopóki nie zostanie wybrana któraś odpowiedź
        //to pokazuj pytania i odpowiedzi oraz sprawdzaj, czy nie użyto któregoś z kół ratunkowych
        game.showQuestionAndAnswers(i,oled);
        game.questionToAudience(oled,lcd,i);
        game.phone(oled,lcd,i);
        game.fiftyFifty(oled,i);
      }

      if (game.getPosition() != 7) { //jeśli nie zrezygnowano z gry
        digitalWrite(marked, HIGH); //to zapal żółtą diodę i odczekaj adekwatnie do numeru pytania
        if (i <= 6) {
          delay(2000);
        }
        else {
          delay(3000);
        }
      }
      
      if (game.getPosition() == game.getCorrectAnswer(i)) { //jeśli udzielono prawidłowej odpowiedzi
        digitalWrite(correct, LOW); //zapal zieloną i zgaś czerwoną diodę
        digitalWrite(wrong, HIGH);
        lcd.clear();
        lcd.setCursor(7,1);
        lcd.print("Dobrze"); //pokaż komunikat na LCD
        //przy osiągnięciu kwoty gwarantowanej - pokaż ten fakt na LCD
        switch (i) {
          case 0:
            currentWin = 500;
            break;
          case 1:
            currentWin = 1000;
            guarantedWin = currentWin;
            lcd.clear();
            lcd.setCursor(0,1);
            lcd.print("Kwota gwarantowana:");
            lcd.setCursor(8,2);
            lcd.print(currentWin);
            break;
          case 2:
            currentWin = 2000;
            break;
          case 3:
            currentWin = 5000;
            break;
          case 4:
            currentWin = 10000;
            break;
          case 5:
            currentWin = 20000;
            break;
          case 6:
            currentWin = 40000;
            guarantedWin = currentWin;
            lcd.clear();
            lcd.setCursor(0,1);
            lcd.print("Kwota gwarantowana:");
            lcd.setCursor(8,2);
            lcd.print(currentWin);
            break;
          case 7:
            currentWin = 75000;
            break;
          case 8:
            currentWin = 125000;
            break;
          case 9:
            currentWin = 250000;
            break;
          case 10:
            currentWin = 500000;
            break;
          case 11:
            currentWin = 1000000;
            guarantedWin = currentWin;
            break;
        }
        oled.clearDisplay();
      }
      else if (game.getPosition() == 7) { //jeśli zrezygnowano z gry
        byte position = showBasicSettings(5,2,"Czy na",5,3,"pewno?"); //ustal, czy na pewno
        if (position == 5) { //jeśli tak, to zakończ grę i pokaż aktualną wygraną
          guarantedWin = currentWin;
          break;
        }
        else { //jeśli nie, to wróć do aktualnego pytania 
          i--;
          oled.clearDisplay();
          delay(200);
          continue;
        }
      }
      else { //jeśli udzielono złej odpowiedzi to zapal czerwoną i zgaś zieloną diodę
        digitalWrite(wrong, LOW);
        digitalWrite(correct, HIGH);
        lcd.clear();
        lcd.setCursor(8,1);
        lcd.print("Zle");
        break; //skończ grę i pokaż wygraną kwotę gwarantowaną
      }
    }
    oled.clearDisplay();
    delay(200);
    while (digitalRead(confirm) == HIGH) { //pokaż jaka jest wygrana
      if (guarantedWin == 1000000) {
        oled.setCursor(1,0);
        oled.print("GRATULACJE!!!");
      }
      oled.setCursor(4,3);
      oled.print("Wygrana:");
      oled.setCursor(4,4);
      oled.print(guarantedWin);
      oled.setInverseFont(1);
      oled.drawString(7,7,"OK");
      oled.setInverseFont(0);
    }

    //rozpoczęcie gry od nowa
    oled.setInverseFont(0);
    showMainMenu();
  }
}

byte showBasicSettings(int x1, int y1, const char *text1, int x2, int y2, const char *text2) {
  byte position = 4;
  oled.clearDisplay();
  oled.setInverseFont(0);
  delay(200);

  while (digitalRead(confirm) == HIGH || position == 4) { //dopóki nie wciśnięty przycisk potwierdzania lub nie ruszony joystick
    oled.drawString(x1, y1, text1); //wyświetlaj odpowiednie menu
    oled.drawString(x2, y2, text2);
    position = getPosition(position);
    showChoiceMenu(position); //pobierz pozycję joysticka, aby wiedzieć, który wiersz OLED'a zaznaczyć
  }

  return position;
}

void showChoiceMenu(byte position) {
  unsigned long time = 0, rememberedTime = 0;
  time = millis();
  if (time - rememberedTime >= 150UL) {
    rememberedTime = time;
    for (int i = 5; i <= 6; i++) { //wyświetlanie odpowiednio oznaczonych informacji
      oled.setCursor(6, i);
      if (i == position) {
        oled.setInverseFont(1);
      }
      switch (i) {
      case 5:
        oled.print("TAK");
        break;
      case 6:
        oled.print("NIE");
        break;
      }
      oled.setInverseFont(0);
    }
  }
}

byte getPosition(byte position) {
  if (analogRead(A0) > 800) { //joystick w dół
    if (position == 6) {
      position = 5;
    }
    else {
      position++;
    }
  }
  else if (analogRead(A0) < 700) { //joystick w górę
    if (position == 5) {
      position = 6;
    }
    else {
      position--;
    }
  }
  delay(100);
  return position;
}

void updateDatabaseOnSD(WebConnection web) { //aktualizacja bazy pytań na karcie SD
  File file; //zmienna do przeprowadzania operacji na plikach na karcie
  String id = web.connectAndRead(true, -1, -1); //pobranie numeru ostatniego pytania z bazy
  int existingQuestions = takeNumberOfQuestionsFromSD(); //pobranie liczby pytań z karty SD

  while (!SD.begin(4)) { //sprawdzenie, czy karta SD jest umieszczona w slocie SD
    oled.clearDisplay();
    oled.setCursor(1, 3);
    oled.print("Wloz karte SD!");
  }
  if (SD.exists("M/ILOSC.txt")) { //jeżeli podany plik istnieje, to usuń go
    SD.remove("M/ILOSC.txt");
  }
  //i następnie zapisz nową wartość ilości zestawów pytań
  file = SD.open("M/ILOSC.txt", FILE_WRITE);
  file.print(id);
  file.close();
  delay(200);
  int n = 1;

  for (int i = id.toInt(); i >= existingQuestions; i--) { //sprawdzenie czy na karcie SD, w poszczególnym folderze z pytaniami, znajduje się plik z danym pytaniem
    oled.clearDisplay();
    for (int j = 1; j <= 12; j++) {
      while (!SD.begin(4)) { //sprawdzenie, czy karta SD jest umieszczona w slocie SD
        oled.clearDisplay();
        oled.setCursor(1, 3);
        oled.print("Wloz karte SD!");
      }
      oled.setCursor(1, 3);
      oled.print("Aktualizowanie");
      oled.setCursor(4,4);
      oled.print(n);
      oled.drawString(5,4," z ");
      oled.setCursor(8,4);
      oled.print(id.toInt()-existingQuestions);
      String path = "M/";
      switch (j) {
        case 1:
          path.concat("500/");
          break;
        case 2:
          path.concat("1000/");
          break;
        case 3:
          path.concat("2000/");
          break;
        case 4:
          path.concat("5000/");
          break;
        case 5:
          path.concat("10000/");
          break;
        case 6:
          path.concat("20000/");
          break;
        case 7:
          path.concat("40000/");
          break;
        case 8:
          path.concat("75000/");
          break;
        case 9:
          path.concat("125000/");
          break;
        case 10:
          path.concat("250000/");
          break;
        case 11:
          path.concat("500000/");
          break;
        case 12:
          path.concat("1000000/");
          break;
      }
      path.concat(i); //dodanie nazwy pliku do ścieżki (czyli numeru pytania dla danego folderu)
      path.concat(".txt"); //dodanie rozszerzenia dla zwykłego pliku tekstowego
      if (!SD.exists(path)) {
        file = SD.open(path, FILE_WRITE);
        file.print(web.connectAndRead(false, j, i)); //nie pobieram id, tylko daję numer tabeli (j) i numer pytania (i)
        file.close();
        delay(200);
      }
    }
    n++;
  }
  //wyświetlenie informacji o zakończeniu aktualizacji karty pamięci
  oled.clearDisplay();
  oled.setCursor(1, 3);
  oled.print("Zaktualizowano");
  delay(2000);
}

int takeNumberOfQuestionsFromSD() { //pobierz informację o tym, ile pytań za 500, 1000 itd.
  File file;
  int number = 0;
  while (!SD.begin(4)) { //sprawdź, czy karta SD jest włożona
    oled.clearDisplay();
    oled.setCursor(1, 3);
    oled.print("Wloz karte SD!");
  }

  if (SD.exists("M/ILOSC.txt")) {
    file = SD.open("M/ILOSC.txt"); //otwórz plik ILOSC.txt
    while (file.available()) {
      number = file.readString().toInt(); //i pobierz całą jego zawartość (w tym wypadku jest to tylko liczba)
    }
    file.close();
  }
  
  return number;
}

int getPosition (int i) {
  switch (i) {
    case 0 ... 3: //500, 1000, 2000, 5000
      return 6;
    case 4 ... 10: //10000, 20000, 40000, 75000, 125000, 250000, 500000
      return 5;
    case 11: //1000000
      return 4;
  }
}

void showMainMenu() {
  //dopóki nie jest wciśnięty przycisk od potwierdzania
  while (digitalRead(confirm) == HIGH) {
    //wyświetlaj menu
    oled.drawString(3,3,"MILIONERZY");
    oled.setInverseFont(1);
    oled.drawString(5,4,"START");
    oled.setInverseFont(0);
  }
}