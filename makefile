# Cel domyślny
all: mainprog pacjent rejestracja

# Kompilowanie programu mainprog
mainprog: mainprog.o sem_utils.o msg_utils.o dekoratory.o
	gcc -o mainprog $^

# Kompilowanie pliku mainprog.o
mainprog.o: mainprog.c sem_utils.h msg_utils.h dekoratory.h
	gcc -c mainprog.c

# Kompilowanie sem_utils.o
sem_utils.o: sem_utils.c sem_utils.h
	gcc -c $^

# Kompilowanie programu pacjent
pacjent: pacjent.o sem_utils.o msg_utils.o dekoratory.o
	gcc -o pacjent $^

# Kompilowanie pliku pacjent.o
pacjent.o: pacjent.c sem_utils.h msg_utils.h dekoratory.h pacjent.h
	gcc -c pacjent.c

# Kompilowanie msg_utils.o
msg_utils.o: msg_utils.c msg_utils.h
	gcc -c msg_utils.c

# Kompilowanie dekoratory.o
dekoratory.o: dekoratory.c dekoratory.h
	gcc -c dekoratory.c

# Kompilowanie programu rejestracja
rejestracja: rejestracja.o sem_utils.o msg_utils.o dekoratory.o
	gcc -o rejestracja $^

# Kompilowanie pliku rejestracja.o
rejestracja.o: rejestracja.c sem_utils.h msg_utils.h dekoratory.h pacjent.h
	gcc -c rejestracja.c

# Cel do czyszczenia obiektów i plików wykonywalnych
clean:
	rm -f *.o mainprog pacjent rejestracja
