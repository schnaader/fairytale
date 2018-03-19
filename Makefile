PROGNAME = fairytale
CFLAGS = -std=c++11 -m64 -O2 -Wall -s
LFLAGS = -lz
TRANSFORMSCPP = transforms/zlibtransform.cpp
PARSERSCPP = parsers/ddsparser.cpp parsers/modparser.cpp parsers/textparser.cpp
MAINCPP = analyser.cpp block.cpp deduper.cpp filestream.cpp hybridstream.cpp storagemanager.cpp

.PHONY: all
all: $(PROGNAME)

.PHONY: clean
clean:
	rm -f *.o

$(PROGNAME):
	c++ $(CFLAGS) $(TRANSFORMSCPP) $(PARSERSCPP) $(MAINCPP) $(LFLAGS) fairytale.cpp -ofairytale
