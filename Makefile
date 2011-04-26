all: yvideofetch

CXX = g++
CPPFLAGS = -DCURL_STATICLIB
INCLUDE = -I/usr/local/include
LDFLAGS = -L/usr/local/lib
LIBS = -lcurl -lhtmlcxx -lPocoNet -lPocoFoundation

yvideofetch: main.o htmlParser.o videoFetch.o
	$(CXX) main.o videoFetch.o htmlParser.o $(CPPFLAGS) $(INCLUDE) $(LDFLAGS) $(LIBS) -o yvideofetch

main.o:
	$(CXX) $(CPPFLAGS) $(INCLUDE) -c main.cpp

videoFetch.o:
	$(CXX) $(CPPFLAGS) $(INCLUDE) -c videoFetch.cpp

htmlParser.o: 
	$(CXX) $(INCLUDE) -c htmlParser.cpp



clean:
	rm -f main.o videoFetch.o htmlParser.o yvideofetch
