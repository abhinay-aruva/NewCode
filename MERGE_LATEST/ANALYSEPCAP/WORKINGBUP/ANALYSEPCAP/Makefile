
APPNAME = PCAP_ANALY 


APP_OBJS = main.o\
	   tcpProto.o\
	   displayStats.o

#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/

CPPFLAGS =  -lpthread -lrt -ltrace

#INCLUDEPATH  =  -I ./readerwriterqueue/

.SUFFIXES:
	.h .cpp

%.o:%.cpp %.h
	$(CXX) $(CPPFLAGS) -g -c $^
%.o:%.cpp
	$(CXX) $(CPPFLAGS) -g -c $^


.PHONY:
	all clean


all: $(APPNAME)
	
 
$(APPNAME): $(APP_OBJS) 
	$(CXX) $(CPPFLAGS) $(APP_OBJS) -g  -o $@  
	
clean:
	-rm -f *.o $(APPNAME)

#	g++ -ltrace -lpthread -o analyse.exe main.cpp	
#	g++ -ltrace  -c tcpProto.cpp
#	g++ -ltrace -c displayStats.cpp 





