
APPNAME = PCAP_ANALY
APPTCPTRACE = PCAP_ANALY_TCPT 


APP_OBJS = tcpProto.o\
	   udpProto.o\
	   lteGtp.o\
	   displayStats.o\
	   Diameter.o \
           GxInterface.o \
           GyInterface.o \
           S6bInterface.o \
           murmurhash3.o \
           SharedHashFile.o \
           shf.o \
           Interface.o \
	   sip.o 
           

#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/

#LIBPATH = /usr/local/lib/

CPPFLAGS += "-std=c++0x" 
CFLAGS   += -Wimplicit -Wmissing-prototypes -Wnested-externs -Wstrict-prototypes -std=gnu99
#LIBPATH  = -L /usr/local/lib/
#NDPIINC = -I /usr/local/include/libndpi-1.7.1/libndpi/
LIBFLAGS =  -lpthread  -lrt -ltrace -DUSER_TCP_UDP
#LIBFLAGS =  -lpthread  -lrt -ltrace
NDPILIB = -lndpi
#LIBFLAGS = -DUSER_TCP_UDP

INCLUDEPATH  =  -I SHF/ -I libshmht_dist_0.1_x86/include/ 
LIBPATH += -L libshmht_dist_0.1_x86/lib/libshmht_0.1.so
VPATH=./:SHF/
.SUFFIXES:
	 .cpp

%.o:%.cpp
	$(CXX) $(CPPFLAGS) $(LIBPATH)  $(LIBFLAGS) $(INCLUDEPATH) -g -c $^  

%.o:%.c
	$(CC) $(CFLAGS) $(LIBPATH)  $(LIBFLAGS) $(INCLUDEPATH) -g -c $^


.PHONY:
	all clean


all: $(APPTCPTRACE)
	
 
#$(APPNAME): $(APP_OBJS) 
#	$(CXX) -g main.cpp  -o $@ $(CPPFLAGS) $(INCLUDEPATH)   $(LIBPATH) $(LIBFLAGS)  $(APP_OBJS) $(NDPILIB)

$(APPTCPTRACE): $(APP_OBJS)
	$(CXX) -g main_trace.cpp -o $@ $(CPPFLAGS)  $(LIBPATH) $(LIBFLAGS)  $(APP_OBJS)
	
clean:
	-rm -f *.o  $(APPNAME) $(APPTCPTRACE)

#	g++ -ltrace -lpthread -o analyse.exe main.cpp	
#	g++ -ltrace  -c tcpProto.cpp
#	g++ -ltrace -c displayStats.cpp 





