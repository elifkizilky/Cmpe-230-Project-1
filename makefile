mylang2ir:	main.o postfix.o
			g++ main.o postfix.o -o mylang2ir
			chmod 777 mylang2ir
main.o:		main.cpp 
			g++ -c main.cpp 
postfix.o:	postfix.cpp
			g++ -c postfix.cpp