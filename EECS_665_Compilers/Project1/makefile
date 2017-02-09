program:	main.o
	g++ -g -Wall -std=c++11 main.o -o Program

main.o: main.cpp
	g++ -g -Wall -std=c++11 -c main.cpp

test: program
	./Program < input.txt

d2u:
	dos2unix correct_output.txt

testcheck: program d2u
	./Program < input.txt > output.txt
	diff output.txt correct_output.txt

test_windows: program d2u
	Program.exe < input.txt > output.txt
	diff output.txt correct_output.txt

clean:
	rm *.o Program Program.exe
