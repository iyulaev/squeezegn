optimization = 3

# Default builds the dictionary builder
dictionarybuilder: DictionaryBuilder.o SequenceWord.o squeezegene.h utility.h
	g++ -o dictionarybuilder.exe DictionaryBuilder.o SequenceWord.o

SequenceWord.o: SequenceWord.cpp squeezegene.h utility.h
	g++ -c -O$(optimization) -msse4.2 -floop-optimize SequenceWord.cpp
	
DictionaryBuilder.o: DictionaryBuilder.cpp squeezegene.h utility.h
	g++ -c -O$(optimization) -msse4.2 -floop-optimize -std=gnu++0x DictionaryBuilder.cpp
	
Dictionary.o: Dictionary.cpp Dictionary.h squeezegene.h utility.h
	g++ -c -O$(optimization) -msse4.2 -floop-optimize -std=gnu++0x Dictionary.cpp
	
Compressor.o: Compressor.cpp Compressor.h squeezegene.h utility.h
	g++ -c -O$(optimization) -msse4.2 -floop-optimize -std=gnu++0x Compressor.cpp
	
# Builds the Dictionary test suite
dictionarytest: Dictionary.o DictionaryTest.cpp SequenceWord.o
	g++ -c -std=gnu++0x DictionaryTest.cpp
	g++ -o dictionarytest.exe DictionaryTest.o Dictionary.o SequenceWord.o

# Builds just sequenceword with debugging enabled, so that main() gets compiled
# in and the test cases get run
sequenceword: SequenceWord.cpp squeezegene.h utility.h
	g++ -o sequenceword.exe -DDEBUG_SEQUENCEWORD -msse4.2 -floop-optimize SequenceWord.cpp
	
#Buildes the compressor tool (not yet working 2012-10-15)
compressor: Compressor.o SequenceWord.o Dictionary.o squeezegene.h
	g++ -o compressor.exe Compressor.o Dictionary.o SequenceWord.o

#Removes all generated / temporary files
clean:
	rm -f *.o
	rm -f *.exe
	rm -f *.stackdump
