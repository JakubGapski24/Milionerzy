#include <Arduino.h>
#include <U8x8lib.h>
#include <Ethernet.h>
#include <SD.h>

class WebConnection
{
private:
    EthernetClient connect; //utworzenie połączenia przez Ethernet
    char inString[2048];    //tablica przechowująca "odebrane" dane z odpowiedniego skryptu PHP (przy odbieraniu danych z serwera)
    int stringPos = 0;      //zmienna określająca pozycje, odbieranych ze skryptu PHP, znaków w tablicy inString
    bool startRead = false; //zmienna określająca czy nadal odbywa się odczytywanie danych ze skryptu PHP
    byte mac[6];

public:
    WebConnection(byte mac[6])
    {
        for (int i = 0; i <= 5; i++)
        {
            this->mac[i] = mac[i];
        }
    }

    bool isConnected()
    {
        if (Ethernet.begin(this->mac) != 0)
        {
            return true;
        }
        return false;
    }

    String connectAndRead(bool checkId, int table, int question)
    { //funkcja określająca, z którego pliku PHP będą odczytywane dane
        if (this->connect.connect("46.242.241.62", 80))
        {                                                                //jeżeli jest nawiązane połączenie z serwerem na porcie nr 80
            this->connect.print("GET /Milionerzy/pobierz_pytania.php?"); //wywołaj skrypt PHP, zgodnie z otrzymaną nazwą
            this->connect.print("checkId=");
            this->connect.print(checkId);
            this->connect.print("&&table=");
            this->connect.print(table);
            this->connect.print("&&question=");
            this->connect.print(question);
            this->connect.println(" HTTP/1.1");
            this->connect.println("Host: www.serwer2064773.home.pl");
            this->connect.println("Connection: close");
            this->connect.println();
            return odczytPHP(); //zwróć odczytane dane
        }
        else
        { //jeżeli nie ma połączenia z serwerem, to znaczy, że sterownik działa w trybie OFFLINE
            return "Connection failed";
        }
    }

    String odczytPHP()
    {
        //odczytaj i zwróć wszystkie dane zawarte między znakami '<' i '>'
        this->stringPos = 0;
        memset(&this->inString, 0, 2048); //wyczyszczenie tablicy przechowującej odebrane znaki, dane

        while (true)
        {
            if (this->connect.available())
            {                                  //jeżeli nawiązanie połączenia z serwerem (skryptem PHP) jest możliwe, to
                char c = this->connect.read(); //odczytaj pierwszy znak
                if (c == '<')
                { //jeżeli pierwszy odczytany znak to '<', to rozpocznij odczytywanie danych, w innym przypadku odczytanie danych nie będzie możliwe
                    this->startRead = true;
                }
                else if (this->startRead)
                { //jeżeli pierwszym znakiem było '<', to kontynuuj odczytywanie danych do momentu odczytania znaku '>'
                    if (c != '>')
                    {
                        this->inString[this->stringPos] = c; //zapisywanie kolejnego odczytanego znaku w tablicy
                        this->stringPos++;                   //przejście do kolejnego, pustego elementu tablicy, aby w następnej iteracji można było zapisać kolejny, odebrany znak
                    }
                    else
                    {                            //jeżeli aktualnie odebrany znak to '>', to odczytane zostały wszystkie dane
                        this->startRead = false; //zakończ odczyt
                        this->connect.stop();    //zakończ połączenie z serwerem
                        this->connect.flush();
                        return inString; //zwróć, przekaż odczytane dane w miejsce wywołania tej funkcji
                    }
                }
            }
        }
    }
};