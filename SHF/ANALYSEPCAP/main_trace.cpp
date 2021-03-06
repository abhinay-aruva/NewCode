#include <iostream>
#include <fstream>
#include <inttypes.h>
#include <err.h>
#include <cerrno>
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <dirent.h>
#include <pthread.h>
#include <list>
#include <string>
#include <string.h>
#include <unistd.h>
#include "displayStats.h"
#include "libtrace_parallel.h"
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <syscall.h>
#include <sstream>
using namespace std;

#define DIRPATH "/apps/opt/LIBTRACE/test/"
#define MAX_THREAD 20
string pcapFileEndStr = "ready.pcap";
string fileNo;
string logFileName="";


std::ofstream logger("pcapanal.log");
pthread_t threads[MAX_THREAD];
std::string protcolname[20];

//uint64_t count = 0;
//Dpi
extern string _protoFilePath;

bool SetThreadAttributes(int m_readeraffinity)
{
    if ( -1 == m_readeraffinity)
       return false; // Default thread behaviour

    int readtid = syscall(__NR_gettid);//syscall.h to get the pid associated with thread
    pthread_t thread = pthread_self();
    cpu_set_t csmask;
    CPU_ZERO(&csmask);
    CPU_SET(m_readeraffinity, &csmask);
    
    if ( pthread_setaffinity_np(thread, sizeof(cpu_set_t), &csmask) != 0 )
    {
        std::cerr << "Reader error on thread set cpu \n";
        return false;
    }
}

static void per_packet(libtrace_packet_t *packet)
{
	assert(packet);
	/* This function turns out to be really simple, because we are just
	 * counting the number of packets in the trace */
//	count += 1;
        displayStats::getdashB()->ParsePkt(packet);
}

/* Due to the amount of error checking required in our main function, it
 * is a lot simpler and tidier to place all the calls to various libtrace
 * destroy functions into a separate function.
 */
static void libtrace_cleanup(libtrace_t *trace, libtrace_packet_t *packet) {
	
	/* It's very important to ensure that we aren't trying to destroy
	 * a NULL structure, so each of the destroy calls will only occur
	 * if the structure exists */
	if (trace)
		trace_destroy(trace);
	
	if (packet)
		trace_destroy_packet(packet);

}

void readPcapFile(void* fileName) 
{
	libtrace_t *trace = NULL;
	libtrace_packet_t *packet = NULL;
        string filePath = DIRPATH;
        filePath += (char *)fileName;
        time_t startTime, endTime;
        time(&startTime);
        uint64_t count = 0;
        logger << "Got file:" << (char *)fileName  << std::endl;
        delete[] (char *)fileName;
        logger << "reading Pcap file:" << filePath << std::endl;
	
	/* Creating and initialising a packet structure to store the packets
	 * that we're going to read from the trace */
	packet = trace_create_packet();


	/* Opening and starting the input trace, as per createdemo.c */
	trace = trace_create(filePath.c_str());

	if (trace_is_err(trace))
        {
		trace_perror(trace,"Opening trace file");
		libtrace_cleanup(trace, packet);
                 return;
	}

	if (trace_start(trace) == -1) {
		trace_perror(trace,"Starting trace");
		libtrace_cleanup(trace, packet);
                return; 
	}

	if (packet == NULL)
        {
		/* Unfortunately, trace_create_packet doesn't use the libtrace
		 * error system. This is because libtrace errors are associated
		 * with the trace structure, not the packet. In our case, we
		 * haven't even created a trace at this point so we can't 
		 * really expect libtrace to set an error on it for us, can
		 * we?
		 */
		perror("Creating libtrace packet");
		libtrace_cleanup(trace, packet);
                return;
	}
	/* This loop will read packets from the trace until either EOF is
	 * reached or an error occurs (hopefully the former!)
	 *
	 * Remember, EOF will return 0 so we only want to continue looping
	 * as long as the return value is greater than zero
	 */
        
 char timeBuf  [256]; 
 struct timeval tv;
 struct timezone tz;
 struct tm *tm;
 gettimeofday(&tv, &tz);
 tm=localtime(&tv.tv_sec);
 sprintf (timeBuf, "%02d:%02d:%02d:%03ld",tm->tm_hour, tm->tm_min, tm->tm_sec, (tv.tv_usec) );
 //std::cout << "ABHINAY: time at start of analyzing packet:" << timeBuf << std::endl;
	while (trace_read_packet(trace,packet)>0) {
		/* Call our per_packet function for every packet */
		per_packet(packet);
                count += 1;
	}
        
        if(!displayStats::getdashB()->StatsAvailable)
          {
           libtrace_stat_t *stat =  trace_create_statistics();
            trace_get_statistics( trace, stat);
           displayStats::getdashB()->setStats(*stat);
          }
          

	/* If the trace is in an error state, then we know that we fell out of
	 * the above loop because an error occurred rather than EOF being
	 * reached. Therefore, we should probably tell the user that something
	 * went wrong
	 */
	if (trace_is_err(trace)) {
		trace_perror(trace,"Reading packets");
	}
		libtrace_cleanup(trace, packet);

	/* We've reached the end of our trace without an error so we can
	 * print our final count. Note the use of the PRIu64 format which is
	 * portable across 64 and 32 bit machines */
	//printf("Packet Count = %" PRIu64 "\n", count);
        logger << "Count:" << count << std::endl;
        count = 0; 
        time(&endTime);
        double seconds = difftime(endTime, startTime);
        //std::logger << clr <<  topLeft ;
        logger << "Completed file:" << filePath << " in:" << seconds << " seconds." << std::endl;
        //displayStats::getdashB()->printstats();

        /* Rename the file to indicate that it is completed*/
        string newFilePath = filePath + ".completed";
        if(rename(filePath.c_str(), newFilePath.c_str()))
            logger << "Error renaming the file:" << filePath << " to:" << newFilePath << std::endl;
/*
//Pass the Pcap tp tcptrace
        char Inttime[15] ;
        sprintf(Inttime, "%d",displayStats::getdashB()->getTimeStat());  
//        = (std::string)displayStats::getdashB()->getTimeStat();
        std::string tcptraceCMD = "./tcptrace " ;
        tcptraceCMD = tcptraceCMD + " -I"+ Inttime +" -lr " + newFilePath; 
        if(system (tcptraceCMD.c_str()))
           logger << "Error executing the tcpbinary cmd:" << tcptraceCMD << std::endl;
*/

/*
        int ret = pthread_self();
        for(int c =0 ; c < MAX_THREAD ; c++)
        {  
          if(ret == threads[c]) 
            threads[c] = 0 ;         
        }
        pthread_exit(&ret);
*/
}

bool isPcapfileReady(string fileName)
{
     if(fileName.length() > pcapFileEndStr.length())
     {
         string endStr = fileName.substr(fileName.length()- pcapFileEndStr.length(), fileName.length());
         if(!endStr.compare(pcapFileEndStr))
             return true;
     }

     return false;
}

void usage()
{
  cerr << "PCAP_ANALY_TCPT \n" 
       << "	-l: log name\n" 
       << "	-c: core number\n" ; 
}
int parse_args(int argc, char **argv)
{

    int option;
    int core = -1;
     while ((option = getopt(argc, argv,"l:c:i:")) != -1)
    {
      switch (option) {
              case 'l' : 
                   if(logger)
                    logger.close(); 

                   logFileName = optarg;
                   logger.open(optarg, std::ofstream::out | std::ofstream::app);
                   if (logger.fail()) {
                   cerr << "open failure as expected: " << strerror(errno) << '\n';
                   return -1;
                   }

              break;

              case 'c' :
                 core = atoi(optarg);
                 break;
 
              case 'i':
                 fileNo = optarg;
                 pcapFileEndStr = "_" + fileNo  + "ready" + ".pcap";
                 if(logFileName.size() == 0)
                 {
                     if(logger)
                         logger.close();

                     logFileName = "pcapanal_" + fileNo + ".log";
                     logger.open(logFileName.c_str(), std::ofstream::out | std::ofstream::app);
                     if (logger.fail())
                     {
                         cerr << "open failure as expected: " << strerror(errno) << '\n';
                         return -1;
                     }
                 }
                 break;
 
              default: 
                  usage(); 
                  return -1; 
             }    
    }

    if(core == -1)
    {
        std::cout << "ABHINAY: core is not set" << std::endl;
        usage();
    }
    else
    {
        SetThreadAttributes(core);
    }
}

int main(int argc , char * argv []) 
{
    int fileCount = 0, rc = 0, i = 0;
    protcolname[TRACE_IPPROTO_TCP] = "TCP";
    protcolname[TRACE_IPPROTO_UDP] = "UDP";
    std::list<string> filesList;
    std::list<string>::iterator it;
       for(int c =0 ; c < MAX_THREAD ; c++)
             threads[c] = 0 ;
    parse_args(argc,argv); 
  while(1) 
 {
    DIR *dirp = NULL;
    dirp = opendir(DIRPATH);
    struct dirent *dr;
    while ((dr = readdir(dirp)) != NULL)
    {
        if(dr)
        {
            string fileName = dr->d_name;
            if(isPcapfileReady(fileName))
              {
                        /* Rename the file to indicate that it has been taken for processing */
                 string OldFilepath = DIRPATH + fileName;
                 fileName = fileName + ".taken";
                 string newFilePath = DIRPATH + fileName;
                 if(rename(OldFilepath.c_str(), newFilePath.c_str()))
                 logger << "Error renaming the file:" << fileName << " to:" << newFilePath << std::endl;
                 else 
                 filesList.push_back(fileName);
              }
        }
    }
    
    /*
    //Count threads 
    DIR *dirp1 = NULL;
    char procPath[50];
    sprintf(procPath, "%s%d%s", "/proc/", getpid(), "/task/");
    dirp1 = opendir(procPath);
    struct dirent *dr1;
    int thredCount = 0;
    while ((dr1 = readdir(dirp1)) != NULL)
    {
      if(dr1 && dr1->d_type == DT_DIR)
        {
           thredCount++;
        }
    }
     (void)closedir(dirp1);
      if(thredCount > 3)// limiting the thread count .+..+mainthread 
      { 
      //logger << "\n Checking for faulty thread\n" << endl;
        // Mechanism to detect Faulty thread
        struct timespec curtime;
        int threadexit_status;


        for (int Ti = 0; Ti < MAX_THREAD; Ti++) // Faulty thread timeouts 
        {
            int threadId = threads[Ti];
            if(threadId != 0 )
            {  
               if (clock_gettime(CLOCK_REALTIME, &curtime) == -1) {
                logger << "Error while reading the real time from clock_gettime" << endl; 
               }
               curtime.tv_sec += 60;// Give a sec time 
               //threadexit_status = pthread_timedjoin_np(threadId, NULL, &curtime);
                  //pthread_join(threadId,NULL);
                 sleep(3);//20 * 3 = 60 secs window size 
              if (threadexit_status != 0) {
              //Don't cancel a thread . if it has locked a resource the resource remains locked for ever Dead lock :(( 
                //pthread_cancel(threadId);
              logger << "Cancel request sent to thread after  60 secs of waiting" << threads[Ti] << endl; 
                  threads[Ti] = 0; 
            }
          }
        }


        // for (int Ti = 0; Ti < filesList.size(); Ti++)// Wait for any 3 threads and start creating 
         //pthread_join(threads[Ti],NULL);
      }else
      {
        sleep (1);
      }
   
   */ 

    for(it = filesList.begin(); it != filesList.end(); it++)
    {
        char *pcapFile = new char[it->length()];
        strcpy(pcapFile, it->c_str());
           
        logger << "Starting Thread for file:" << pcapFile << std::endl;
        readPcapFile(pcapFile);
    }

    i = 0;

    filesList.clear();  
    (void)closedir(dirp);
 }

}
