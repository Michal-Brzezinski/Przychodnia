# Cel domyslny
all: mainprog pacjent rejestracja lekarz dyrektor

# Kompilowanie programu mainprog
mainprog: mainprog.o MyLib/sem_utils.o MyLib/msg_utils.o MyLib/utils.o MyLib/shm_utils.o
	@gcc -o mainprog $^ -lm  

# Kompilowanie pliku mainprog.o
mainprog.o: mainprog.c MyLib/sem_utils.h MyLib/msg_utils.h MyLib/utils.h MyLib/shm_utils.h
	@gcc -c mainprog.c

# Kompilowanie MyLib/sem_utils.o
MyLib/sem_utils.o: MyLib/sem_utils.c MyLib/sem_utils.h MyLib/utils.h
	@gcc -c -o MyLib/sem_utils.o MyLib/sem_utils.c

# Kompilowanie programu pacjent
pacjent: pacjent.o MyLib/sem_utils.o MyLib/msg_utils.o MyLib/utils.o
	@gcc -o pacjent $^ -lpthread -lm  

# Kompilowanie pliku pacjent.o
pacjent.o: pacjent.c MyLib/sem_utils.h MyLib/msg_utils.h MyLib/utils.h pacjent.h
	@gcc -c pacjent.c

# Kompilowanie MyLib/msg_utils.o
MyLib/msg_utils.o: MyLib/msg_utils.c MyLib/msg_utils.h MyLib/utils.h
	@gcc -c -o MyLib/msg_utils.o MyLib/msg_utils.c

# Kompilowanie MyLib/utils.o
MyLib/utils.o: MyLib/utils.c MyLib/utils.h
	@gcc -c -o MyLib/utils.o MyLib/utils.c -lm  

# Kompilowanie MyLib/shm_utils.o
MyLib/shm_utils.o: MyLib/shm_utils.c MyLib/shm_utils.h MyLib/utils.h
	@gcc -c -o MyLib/shm_utils.o MyLib/shm_utils.c

# Kompilowanie programu rejestracja
rejestracja: rejestracja.o MyLib/sem_utils.o MyLib/msg_utils.o MyLib/utils.o MyLib/shm_utils.o
	@gcc -o rejestracja $^ -lm 

# Kompilowanie pliku rejestracja.o
rejestracja.o: rejestracja.c MyLib/sem_utils.h MyLib/msg_utils.h MyLib/shm_utils.h MyLib/utils.h pacjent.h rejestracja.h
	@gcc -c rejestracja.c

# Kompilowanie programu lekarz
lekarz: lekarz.o MyLib/utils.o MyLib/sem_utils.o MyLib/msg_utils.o MyLib/shm_utils.o
	@gcc -o lekarz $^ -lpthread -lm  

# Kompilowanie pliku lekarz.o
lekarz.o: lekarz.c MyLib/utils.h MyLib/sem_utils.h MyLib/msg_utils.h lekarz.h MyLib/shm_utils.h
	@gcc -c lekarz.c

# Kompilowanie programu dyrektor
dyrektor: dyrektor.o MyLib/utils.o	MyLib/sem_utils.o
	@gcc -o dyrektor dyrektor.o MyLib/utils.o MyLib/sem_utils.o -lm  

# Kompilowanie pliku dyrektor.o
dyrektor.o: dyrektor.c dyrektor.h MyLib/utils.h MyLib/sem_utils.h
	@gcc -c dyrektor.c

# Cel do czyszczenia obiektow i plikow wykonywalnych
clean:
	@rm -f *.o mainprog pacjent rejestracja lekarz dyrektor MyLib/*.o
