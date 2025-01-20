all: mainprog pacjent rejestracja
mainprog: mainprog.o operacje.o
	gcc -o mainprog $^
mainprog.o: mainprog.c operacje.h 
	gcc -c mainprog.c
operacje.o: operacje.c
	gcc -c $^
pacjent: pacjent.o operacje.o
	gcc -o pacjent $^
pacjent.o: pacjent.c
	gcc -c $^
rejestracja: rejestracja.o operacje.o
	gcc -o rejestracja $^
rejestracja.o:	rejestracja.c
	gcc -c $^

