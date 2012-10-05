optimization = 3

default: DictionaryBuilder.o SequenceWord.o squeezegene.h
	g++ -o dictionarybuilder.exe DictionaryBuilder.o SequenceWord.o

SequenceWord.o: SequenceWord.cpp squeezegene.h
	g++ -c -O$(optimization) -msse4.2 -floop-optimize SequenceWord.cpp
	
DictionaryBuilder.o: DictionaryBuilder.cpp squeezegene.h
	g++ -c -O$(optimization) -msse4.2 -floop-optimize -std=gnu++0x DictionaryBuilder.cpp

sequenceword: SequenceWord.cpp squeezegene.h
	g++ -o sequenceword.exe -msse4.2 -floop-optimize sequenceword.cpp
	
Compressor.o: Compressor.cpp Compressor.h squeezegene.h
	g++ -c -O$(optimization) -msse4.2 -floop-optimize Compressor.cpp
	
compressor: Compressor.o SequenceWord.o squeezegene.h
	g++ -o compressor.exe Compressor.o SequenceWord.o

clean:
	rm -f *.o
	rm -f *.exe
	rm -f *.stackdump
