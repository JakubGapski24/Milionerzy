//****************************************Załączenie potrzebnych bibliotek**************************************************************
#include <Arduino.h>
#include <U8X8lib.h>
#include <SD.h>
#include <SPI.h>
#include <Milionerzy.cpp>
#include <WebConnection.cpp>
#include <LiquidCrystal_I2C.h>

//*************************************Przypisanie pinów sterownika do zdefiniowanych zmiennych*****************************************
#define confirm PA4          //potwierdzenie wyboru odpowiedzi
#define correct PB0          //dioda zielona - sygnalizuje, że wybrana odpowiedź jest poprawna
#define wrong PC1           //dioda czerwona - sygnalizuje, że wybrana odpowiedź jest błędna
#define marked PC0           //dioda żółta - sygnalizuje zaznaczenie wybranej przez joystick odpowiedzi
#define audienceDiode PA10    //dioda informująca o możliwości wyboru pytania do publiczności
#define fiftyFiftyDiode PB3  //dioda informująca o możliwości wyboru pół na pół
#define phoneDiode PC4      //dioda informująca o możliwości wyboru telefonu do przyjaciela
#define audienceSwitch PB4   //wybór pytania do publiczności
#define fiftyFiftySwitch PB10 //wybór pół na pół
#define phoneSwitch PA8      //wybór telefonu do przyjaciela

LiquidCrystal_I2C lcd(0x27, 20, 4); //inicjalizacja wyświetlacza LCD do obsługi kół ratunkowych - przypisanie adresu
U8X8_SH1106_128X64_NONAME_HW_I2C oled(U8X8_PIN_NONE, 15, 14); //stworzenie obiektu klasy obsługującej wybrany wyświetlacz OLED
Millionaire game = *new Millionaire();                                                                           //stworzenie obiektu klasy Milionerzy (obsługa gry)
byte showBasicSettings(int x, int y, const char *text1, int x2, int y2, const char *text2); //mechanizm wyświetlania tekstu do ustawień początkowych
void choiceMenu(byte position);                                                             //wybór ustawień początkowych gry (aktualizacja bazy pytań i wybór rodzaju telefonu do przyjaciela)
byte getPosition(byte position);                                                            //ustalenie, który wiersz wyświetlacza ma być podświetlony (w tym wypadku odpowiedzi TAK lub NIE), dla ustawień początkowych
void updateDatabaseOnSD(WebConnection web);                                                 //zaktualizowanie danych na karcie SD
int takeNumberOfQuestionsFromSD();
int getPosition(int i); //pobierz pozycję do wyświetlania kwoty pytania

void setup()
{
  //ustawienie przypisanych wyjść w odpowiedni tryb
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

  //dopóki nie jest wciśnięty przycisk od potwierdzania
  /*while (digitalRead(confirm) == HIGH) {
    //wyświetlaj menu
    oled.drawString(3,3,"MILIONERZY");
    oled.setInverseFont(1);
    oled.drawString(5,4,"START");
    oled.setInverseFont(0);
  }*/

  //showBasicSettings(2,2,"Zwykla wersja",4,3,"telefonu?"); //wyświetlaj podaną treść w dwóch linijkach z podanymi współrzędnymi
  byte position = showBasicSettings(2, 2, "Aktualizowac", 2, 3, "baze pytan?");

  oled.clearDisplay();
  oled.setInverseFont(0);

  if (position == 5)
  {
    oled.drawString(2, 3, "Laczenie...");
    delay(1000);
    byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE}; //przypisanie adresu MAC
    WebConnection web = *new WebConnection(mac);
    if (web.isConnected())
    {                          //sprawdzenie adresu MAC i czy sterownik jest podłączony do serwera
      updateDatabaseOnSD(web); //aktualizuj dane na karcie SD
    }
  }
}

void loop()
{
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
    randomSeed(analogRead(PA1));

    for (int i=0; i<=11; i++) { //pobierz pytania z karty SD
      while (!SD.begin(4))
      { //sprawdzenie, czy karta SD jest umieszczona w slocie SD
        oled.clearDisplay();
        oled.setCursor(1, 3);
        oled.print("Wloz karte SD!");
      }
      numberOfQuestion[i] = random(1,(number+1)); //losowanie liczby z określonego wcześniej zakresu
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

      if (SD.exists(path)) {
        String readData[6];
        int index = 0; //zapisywanie danych z pliku w tymczasowej tablic
        file = SD.open(path);
        while (file.available()) {
          readData[index] = file.readStringUntil('\n');
          index++;
          delay(75);
        }
        file.close();
        game.saveDataInStruct(readData[0], readData[1], readData[2], readData[3], readData[4], readData[5], i);
      }
      else { //jeśli nie istnieje taki plik, to zakończ działanie programu
        oled.clearDisplay();
        oled.drawString(5,3,"Blad");
        return;
      }
    }

    oled.clearDisplay();
    int currentWin = 0;
    int guarantedWin = 0;
    int values[12] = {500, 1000, 2000, 5000, 10000, 20000, 40000, 75000, 125000, 250000, 500000, 1000000}; //wartości do wyświetlenia za ile jest aktualne pytanie
    for (int i=0; i<=2; i++) {
      game.setTips(i, true);
    }
    digitalWrite(audienceDiode, HIGH);
    digitalWrite(fiftyFiftyDiode, HIGH);
    digitalWrite(phoneDiode, HIGH);

    //gra właściwa - wyświetlanie pytań
    for (int i=0; i<=11; i++) {
      game.setPosition(1);
      oled.drawString(3,3,"Pytanie za");
      oled.setCursor(getPosition(i),4);
      oled.print(values[i]);
      delay(1500);
      lcd.clear();
      digitalWrite(marked, LOW);
      digitalWrite(wrong, HIGH);
      digitalWrite(correct, HIGH);
      oled.clearDisplay();
      while (digitalRead(confirm) == HIGH || game.getPosition() == 1) {
        game.showQuestionAndAnswers(i, oled);
      }
      digitalWrite(marked, HIGH);
      if (i <= 6) {
        delay(2000);
      }
      else {
        delay(3000);
      }
      if (game.getPosition() == game.getCorrectAnswer(i)) {
        digitalWrite(correct, LOW);
        digitalWrite(wrong, HIGH);
        lcd.clear();
        lcd.setCursor(7,1);
        lcd.print("Dobrze");
        switch (i) {
          case 0:
            currentWin = 500;
            break;
          case 1:
            currentWin = 1000;
            guarantedWin = currentWin;
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
      else {
        digitalWrite(wrong, LOW);
        digitalWrite(correct, HIGH);
        lcd.clear();
        lcd.setCursor(8,1);
        lcd.print("Zle");
        break;
      }
    }
    oled.clearDisplay();
    oled.setCursor(4,3);
    oled.print("Wygrana:");
    oled.setCursor(4,4);
    oled.print(guarantedWin);
  }
}

byte showBasicSettings(int x1, int y1, const char *text1, int x2, int y2, const char *text2)
{
  byte position = 4;
  oled.clearDisplay();
  oled.setInverseFont(0);
  delay(200);

  while (digitalRead(confirm) == HIGH || position == 4)
  {                                 //dopóki nie wciśnięty przycisk potwierdzania lub nie ruszony joystick
    oled.drawString(x1, y1, text1); //wyświetlaj odpowiednie menu
    oled.drawString(x2, y2, text2);
    position = getPosition(position);
    choiceMenu(position); //pobierz pozycję joysticka, aby wiedzieć, który wiersz OLED'a zaznaczyć
  }

  return position;
}

void choiceMenu(byte position)
{
  unsigned long time = 0, rememberedTime = 0;
  time = millis();
  if (time - rememberedTime >= 150UL)
  {
    rememberedTime = time;
    for (int i = 5; i <= 6; i++)
    { //wyświetlanie odpowiednio oznaczonych informacji
      oled.setCursor(6, i);
      if (i == position)
      {
        oled.setInverseFont(1);
      }
      switch (i)
      {
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

byte getPosition(byte position)
{
  if (analogRead(A0) > 800)
  { //joystick w dół
    if (position == 6)
    {
      position = 5;
    }
    else
    {
      position++;
    }
  }
  else if (analogRead(A0) < 700)
  { //joystick w górę
    if (position == 5)
    {
      position = 6;
    }
    else
    {
      position--;
    }
  }
  delay(100);
  return position;
}

void updateDatabaseOnSD(WebConnection web)
{
  File file;                                    //zmienna do przeprowadzania operacji na plikach na karcie
  String id = web.connectAndRead(true, -1, -1); //pobranie numeru ostatniego pytania z bazy
  while (!SD.begin(4))
  { //sprawdzenie, czy karta SD jest umieszczona w slocie SD
    oled.clearDisplay();
    oled.setCursor(1, 3);
    oled.print("Wloz karte SD!");
  }
  if (SD.exists("M/ILOSC.txt"))
  { //jeżeli podany plik istnieje, to usuń go
    SD.remove("M/ILOSC.txt");
  }
  //i następnie zapisz nową wartość ilości zestawów pytań
  file = SD.open("M/ILOSC.txt", FILE_WRITE);
  file.print(id);
  file.close();
  delay(200);

  for (int i = id.toInt(); i >= 1; i--)
  { //sprawdzenie czy na karcie SD, w poszczególnym folderze z pytaniami, znajduje się plik z danym pytaniem
    for (int j = 1; j <= 12; j++)
    {
      while (!SD.begin(4))
      { //sprawdzenie, czy karta SD jest umieszczona w slocie SD
        oled.clearDisplay();
        oled.setCursor(1, 3);
        oled.print("Wloz karte SD!");
      }
      oled.clearDisplay();
      oled.setCursor(1, 3);
      oled.print("Aktualizowanie");
      String path = "M/";
      switch (j)
      {
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
      path.concat(i);      //dodanie nazwy pliku do ścieżki (czyli numeru pytania dla danego folderu)
      path.concat(".txt"); //dodanie rozszerzenia dla zwykłego pliku tekstowego
      if (!SD.exists(path))
      {
        file = SD.open(path, FILE_WRITE);
        file.print(web.connectAndRead(false, j, i)); //nie pobieram id, tylko daję numer tabeli (j) i numer pytania (i)
        file.close();
        delay(200);
      }
    }
  }
  oled.clearDisplay();
  oled.setCursor(1, 3);
  oled.print("Zaktualizowano");
  delay(2000);
}

int takeNumberOfQuestionsFromSD()
{
  File file;
  int number = 0;
  while (!SD.begin(4)) //sprawdź, czy karta SD jest włożona
  {
    oled.clearDisplay();
    oled.setCursor(1, 3);
    oled.print("Wloz karte SD!");
  }

  if (SD.exists("M/ILOSC.txt")) {
    file = SD.open("M/ILOSC.txt"); //otwórz plik ILOSC.txt
    while (file.available())
    {
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