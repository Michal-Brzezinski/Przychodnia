# Przychodnia

W przychodni przyjmują pacjentów dwaj lekarze POZ (pierwszego kontaktu) oraz lekarze o specjalizacjach: kardiolog, okulista, pediatra i lekarz medycyny pracy. Każdy z lekarzy w danym dniu może przyjąć określoną liczbę pacjentów: lekarze POZ każdy X1 osób, kardiolog X2 osób, okulista X3 osób, pediatra X4 osób, lekarz medycyny pracy X5 osób. Lekarze POZ przyjmują ok. 60% wszystkich pacjentów (wartość losowa), lekarze specjaliści każdy po ok. 10% wszystkich pacjentów (wartość losowa). Do obu lekarzy POZ jest jedna wspólna kolejka pacjentów, do każdego lekarza specjalisty kolejki są indywidualne.
Lekarze POZ część pacjentów (ok. 20% z X1) kierują na dodatkowe badania do wskazanych lekarzy specjalistów – pacjent zostaje zarejestrowany do specjalisty przez lekarza POZ, jeżeli dany lekarz nie ma już wolnych terminów dane pacjenta (id - skierowanie do …. - wystawił) powinny zostać zapisane w raporcie dziennym. Lekarze specjaliści dodatkowo mogą skierować pacjenta (ok. 10%) na badanie ambulatoryjne po wykonaniu którego pacjent wraca do danego specjalisty i wchodzi do gabinetu bez kolejki (po uprzednim wyjściu pacjenta). Jeśli w chwili zamknięcia przychodni w kolejce do lekarza czekali pacjenci to te osoby zostaną przyjęte w tym dniu ale nie mogą zostać skierowane na dodatkowe badania.

Zasady działania przychodni ustalone przez Dyrektora są następujące:

- Przychodnia jest czynna w godzinach od Tp do Tk;
- W budynku przychodni w danej chwili może się znajdować co najwyżej N pacjentów (pozostali, jeżeli są czekają przed wejściem);
- Dzieci w wieku poniżej 18 lat do przychodni przychodzą pod opieką osoby dorosłej;
- Każdy pacjent przed wizytą u lekarza musi się udać do rejestracji;
- W przychodni są 2 okienka rejestracji, zawsze działa min. 1 stanowisko;
- Jeżeli w kolejce do rejestracji stoi więcej niż K pacjentów (K>=N/2) otwiera się drugie okienko rejestracji. Drugie okienko zamyka się jeżeli liczba pacjentów w kolejce do rejestracji jest mniejsza niż N/3;
- Osoby uprawnione VIP (np. honorowy dawca krwi) do gabinetu lekarskiego wchodzą bez kolejki;
- Jeżeli zostaną wyczerpane limity przyjęć do danego lekarza, pacjenci ci nie są przyjmowani (rejestrowani);
- Jeżeli zostaną wyczerpane limity przyjęć w danym dniu do wszystkich lekarzy, pacjenci nie są wpuszczani do budynku;
- Jeśli w chwili zamknięcia przychodni w kolejce do rejestracji czekali pacjenci to te osoby nie zostaną przyjęte w tym dniu przez lekarza. 
Dane tych pacjentów (id - skierowanie do …. - wystawił) powinny zostać zapisane w raporcie dziennym.

Na polecenie Dyrektora (sygnał 1) dany lekarz bada bieżącego pacjenta i kończy pracę przed zamknięciem przychodni. Dane pacjentów (id - skierowanie do …. - wystawił), którzy nie zostali przyjęci powinny zostać zapisane w raporcie dziennym.
Na polecenie Dyrektora (sygnał 2) wszyscy pacjenci natychmiast opuszczają budynek.
Napisz procedury Dyrektor, Rejestracja, Lekarz i Pacjent symulujące działanie przychodni.

[Github-przychodnia] (https://github.com/Michal-Brzezinski/Przychodnia)
