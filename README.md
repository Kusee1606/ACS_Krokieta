Repozetorium zawiera odpowiedź na zadanie rekrutycyjne z działu ACS
![image](https://github.com/user-attachments/assets/7df72401-673d-477a-aeac-386d5f6a7523)

Zgodnie z ideą zadania jest to rozwiązanie bardzo uproszczonę, a poniżej znajduję się lista rzeczy które w pełnej implementacji powinny się zmienić:
- Użycie lepszego IMU (https://bps-space.github.io/electronics/imu-selection.html) i dodanie innych czujników np. gps lub barometru
- inplementacja algerytmów do syntezy danych z różnych czujników oraz algolitmów filtrowania danych np. filtru Kalemana
- implementacja regulatorów pid do sterowania serw oraz innych bardziej zawansowanych obliczeń do sterowania ich wychyleniem
- Zmiany miejsca zapisu logów na np. pamięć flash (Przeciążenia mogą nie działać dobrze z krtą sd)
- Dodanie możliwości konfiguracji przez np. bluetooth
- Rozdzielenie kodu na dwa rdzenie mikrokontrolela tak aby jeden był odpowiedzialny za sterowanie rakietą logowanie danych itp a drugi za odczyt danych i wyliczenie aktualnej pozycji
- zapewnie wiele innych rzeczy które teraz nie przychodzą mi do głowy
- dodanie działającej kalibraci 
