# Cel domyślny
all: mainprog pacjent rejestracja

# Kompilowanie programu mainprog
mainprog: mainprog.o MyLib/sem_utils.o MyLib/msg_utils.o MyLib/dekoratory.o
	@gcc -o mainprog $^

# Kompilowanie pliku mainprog.o
mainprog.o: mainprog.c MyLib/sem_utils.h MyLib/msg_utils.h MyLib/dekoratory.h
	@gcc -c mainprog.c

# Kompilowanie MyLib/sem_utils.o
MyLib/sem_utils.o: MyLib/sem_utils.c MyLib/sem_utils.h
	@gcc -c -o MyLib/sem_utils.o MyLib/sem_utils.c

# Kompilowanie programu pacjent
pacjent: pacjent.o MyLib/sem_utils.o MyLib/msg_utils.o MyLib/dekoratory.o
	@gcc -o pacjent $^

# Kompilowanie pliku pacjent.o
pacjent.o: pacjent.c MyLib/sem_utils.h MyLib/msg_utils.h MyLib/dekoratory.h pacjent.h
	@gcc -c pacjent.c

# Kompilowanie MyLib/msg_utils.o
MyLib/msg_utils.o: MyLib/msg_utils.c MyLib/msg_utils.h
	@gcc -c -o MyLib/msg_utils.o MyLib/msg_utils.c

# Kompilowanie MyLib/dekoratory.o
MyLib/dekoratory.o: MyLib/dekoratory.c MyLib/dekoratory.h
	@gcc -c -o MyLib/dekoratory.o MyLib/dekoratory.c

# Kompilowanie programu rejestracja
rejestracja: rejestracja.o MyLib/sem_utils.o MyLib/msg_utils.o MyLib/dekoratory.o
	@gcc -o rejestracja $^

# Kompilowanie pliku rejestracja.o
rejestracja.o: rejestracja.c MyLib/sem_utils.h MyLib/msg_utils.h MyLib/dekoratory.h pacjent.h
	@gcc -c rejestracja.c

# Cel do czyszczenia obiektów i plików wykonywalnych
clean:
	@rm -f *.o mainprog pacjent rejestracja MyLib/*.o
