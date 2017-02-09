program:	main.o caseList.o avPairs.o lem2Calc.o
	g++ -g -Wall -std=c++11 main.o caseList.o avPairs.o lem2Calc.o -o program

main.o: main.cpp caseList.h avPairs.h lem2Calc.h
	g++ -g -Wall -std=c++11 -c main.cpp

caseList.o: caseList.cpp caseList.h
	g++ -g -Wall -std=c++11 -c caseList.cpp

avPairs.o: avPairs.cpp avPairs.h caseList.h
	g++ -g -Wall -std=c++11 -c avPairs.cpp

lem2Calc.o: lem2Calc.cpp lem2Calc.h avPairs.h caseList.h
	g++ -g -Wall -std=c++11 -c lem2Calc.cpp


test: program
	./Program < input.txt

d2u:
	dos2unix correct_output.txt

testcheck: program d2u
	./program < input.txt > output.txt
	diff output.txt correct_output.txt

test_windows: program d2u
	program.exe < input.txt > output.txt
	diff output.txt correct_output.txt

clean:
	rm *.o program program.exe
