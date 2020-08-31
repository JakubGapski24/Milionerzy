# Milionerzy
Gra "Milionerzy" na STM32 (projekt w trakcie realizacji).
Pytania i odpowiedzi są pobierane z bazy danych umieszczonej na zewnętrznym serwerze i zapisywane w pamięci karty SD podłączonej do sterownika.
Bazę pytań na karcie można na bieżąco aktualizować - sterownik zrobi to na "życzenie" użytkownika (oczywiście jeśli konkretnych danych nie ma jeszcze na karcie).
Następnie następuje losowy odczyt pytań z karty, dla każdej kwoty (500, 1000 itd), i ustawienie tych danych w sterowniku na potrzeby przeprowadzenia gry.
Po ustawieniu tych danych, wyświetlane jest pytanie z 4 możliwymi odpowiedziami (na wyświetlaczu oled).
Użytkownik może za pomocą joysticka wybrać odpowiedź i za pomocą przycisku wybrać zaznaczoną odpowiedź.
Wybranie podświetlonej odpowiedzi powoduje zapalenie żółtej diody, a po paru sekundach zapala się również dioda zielona/czerwona (w zależności od tego czy jest to odpowiedź poprawna czy nie).
KOŁA RATUNKOWE JESZCZE NIE SĄ ZROBIONE
