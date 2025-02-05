# Cel domyślny
all: mainprog pacjent rejestracja lekarz

# Kompilowanie programu mainprog
mainprog: mainprog.o MyLib/sem_utils.o MyLib/msg_utils.o MyLib/dekoratory.o MyLib/shm_utils.o
	@gcc -o mainprog $^ -lm  

# Kompilowanie pliku mainprog.o
mainprog.o: mainprog.c MyLib/sem_utils.h MyLib/msg_utils.h MyLib/dekoratory.h MyLib/shm_utils.h
	@gcc -c mainprog.c

# Kompilowanie MyLib/sem_utils.o
MyLib/sem_utils.o: MyLib/sem_utils.c MyLib/sem_utils.h MyLib/dekoratory.h
	@gcc -c -o MyLib/sem_utils.o MyLib/sem_utils.c

# Kompilowanie programu pacjent
pacjent: pacjent.o MyLib/sem_utils.o MyLib/msg_utils.o MyLib/dekoratory.o
	@gcc -o pacjent $^ -lpthread -lm  

# Kompilowanie pliku pacjent.o
pacjent.o: pacjent.c MyLib/sem_utils.h MyLib/msg_utils.h MyLib/dekoratory.h pacjent.h
	@gcc -c pacjent.c

# Kompilowanie MyLib/msg_utils.o
MyLib/msg_utils.o: MyLib/msg_utils.c MyLib/msg_utils.h MyLib/dekoratory.h
	@gcc -c -o MyLib/msg_utils.o MyLib/msg_utils.c

# Kompilowanie MyLib/dekoratory.o
MyLib/dekoratory.o: MyLib/dekoratory.c MyLib/dekoratory.h
	@gcc -c -o MyLib/dekoratory.o MyLib/dekoratory.c -lm  

# Kompilowanie MyLib/shm_utils.o
MyLib/shm_utils.o: MyLib/shm_utils.c MyLib/shm_utils.h MyLib/dekoratory.h
	@gcc -c -o MyLib/shm_utils.o MyLib/shm_utils.c

# Kompilowanie programu rejestracja
rejestracja: rejestracja.o MyLib/sem_utils.o MyLib/msg_utils.o MyLib/dekoratory.o MyLib/shm_utils.o
	@gcc -o rejestracja $^ -lm 

# Kompilowanie pliku rejestracja.o
rejestracja.o: rejestracja.c MyLib/sem_utils.h MyLib/msg_utils.h MyLib/shm_utils.h MyLib/dekoratory.h pacjent.h rejestracja.h
	@gcc -c rejestracja.c

# Kompilowanie programu lekarz
lekarz: lekarz.o MyLib/dekoratory.o MyLib/sem_utils.o MyLib/msg_utils.o MyLib/shm_utils.o
	@gcc -o lekarz $^ -lpthread -lm  

# Kompilowanie pliku lekarz.o
lekarz.o: lekarz.c MyLib/dekoratory.h MyLib/sem_utils.h MyLib/msg_utils.h lekarz.h MyLib/shm_utils.h
	@gcc -c lekarz.c

# Cel do czyszczenia obiektów i plików wykonywalnych
clean:
	@rm -f *.o mainprog pacjent rejestracja lekarz MyLib/*.o
