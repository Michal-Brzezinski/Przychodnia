# Dostęp do budynku:

Mechanizm semafora:
- Wartość semafora = liczba dostępnych miejsc w budynku
- Proces pacjenta próbuje obniżyć wartość semafora przed wejściem i zwiększyć po wyjściu
- Gdy semafor osiągnie 0, pacjenci czekają przed wejściem.

Semafor na początku inicjalizuję na N.
Gdy pacjent wchodzi do budynku to zmniejsza N o 1. Gdy pacjent wychodzi z budynku przychodni, to N zwiększa się o 1.

# Rejestrowanie pacjentów:

(jedna kolejka, oraz 1/2 okienka w zależności od ilości pacjentów)

Kolejka będzie przechowywana w pamięci dzielonej.

- Gdy liczba pacjentów w kolejce przekroczy K >= N/2 , uruchamiany drugi proces rejestracji
- Gdy liczba pacjentów w kolejce zmniejszy się do K < N/3 drugi proces kończy pracę 

Do zarządzania dostępem do kolejki można użyć semafory. Można użyć mechanizmu pthread_cond_wait lub odpowiednika procesowego do czekania na zmiany w kolejce. 
Pacjenci *VIP* dodawani będą na początek kolejki.

# Przydzielanie pacjentów do lekarzy:

- dla każdego lekarza specjalisty w pamięci współdzielonej istnieje osobna kolejka
- dla 2 lekarzy POZ kolejka jest wspólna podobnie do mechanizmu rejestracji
- lekarze POZ obsługują +/- 60% pacjentów (każdy po 30%)
- lekarze specjaliści po ok. 10%
- dla każdej kolejki należy użyć mechanizmu blokady np. semafory.
  
# Przekierowanie pacjentów na badania: 

- Lekarze POZ losowo przekierowują ~20% swoich pacjentów do lekarzy specjalistów (do kolejki, pod warunkiem, że jest miejsce)
- Specjaliści przekierowują losowo ~ 10% pacjentów na badania ambulatoryjne
- Pacjent wracający z badań ambulatoryjnych wchodzi do gabinetu bez kolejki (dodawany na początek).

# Obsługa limitów przyjęć:

- Liczniki przyjęć dla każdego lekarza w pamięci współdzielonej
- Jeżeli licznik osiągnie X, lekarz odmawia przyjęcia kolejnych pacjentów
- Pacjenci, którzy nie zostali przyjęci, są zapisywani w raporcie dziennym

# Obsługa sygnałów Dyrektora:
- Sygnał 1: Lekarz kończy pracę przed zamknięciem budynku
  - Proces lekarza przestaje przyjmować nowych pacjentów
  - Nieobsłużeni pacjenci zapisywani w raporcie dziennym
- Sygnał 2: Ewakuacja
  - Wszystkie procesy pacjentów są natychmiastowo kończone
  - Stan kolejek zapisywany w raporcie dziennym

Wykorzystać można np. SIGUSR1 i SIGUSR2
(SIGUSR1 (sygnał numer 10) i SIGUSR2 (sygnał numer 12) są sygnałami, które można dowolnie wykorzystać w programach. Twórcy oprogramowania mogą zdefiniować ich znaczenie i zachowanie w kontekście swojej aplikacji)

# Tworzenie raportu dziennego
- Dla każdego dnia tworzony jest osobny plik o nazwie: Data_i_godzina stworzenia 
- Format danych:

    id pacjenta, skierowanie do ..., kto wystawił.

- Należy użyć stosownego mechanizmu do blokady do zapisu i odczytu pliku

