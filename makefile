all: mainprog pacjent rejestracja

mainprog: mainprog.o sem_utils.o
	gcc -o mainprog $^

mainprog.o: mainprog.c sem_utils.h msg_utils.h dekoratory.h
	gcc -c mainprog.c

sem_utils.o: sem_utils.c sem_utils.h
	gcc -c $^

pacjent: pacjent.o sem_utils.o msg_utils.o dekoratory.o
	gcc -o pacjent $^

pacjent.o: pacjent.c sem_utils.h msg_utils.h dekoratory.h pacjent.h
	gcc -c pacjent.c

msg_utils.o: msg_utils.c msg_utils.h
	gcc -c msg_utils.c

dekoratory.o: dekoratory.c dekoratory.h
	gcc -c dekoratory.c

rejestracja: rejestracja.o sem_utils.o dekoratory.o
	gcc -o rejestracja $^

rejestracja.o: rejestracja.c sem_utils.h msg_utils.h dekoratory.h pacjent.h
	gcc -c $^
