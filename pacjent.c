#include <stdio.h>
#include <stdlib.h>


typedef struct {
    int id_pacjent; // numer pacjenta - pid
    int vip; // 1 jeśli VIP, 0 jeśli nie
    int wiek;    // Wiek pacjenta
    int id_lekarz;  // numer lekarza, do którego pacjent chce się udać
} Pacjent;

int main(){

    srand(time(0)); // Inicjalizacja generatora liczb losowych

    // ----------- inicjalizacja wartości struktury pacjenta
    Pacjent pacjent;
    pacjent.id_pacjent = getpid();
    
    int pomocnicza_vip = rand() % 100;
    if (pomocnicza_vip < 10)  pacjent.vip = 1; // 10% szans  
    else pacjent.vip  = 0; // 90% szans 
    
    pacjent.wiek = rand() % 100 + 1; // Wiek od 1 do 100 lat

    pacjent.id_lekarz = rand() % 5;
    // -----------------------------------------------------

    

    return 0;
}