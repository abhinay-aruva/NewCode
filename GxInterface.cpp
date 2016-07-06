#include "GxInterface.h"
#include <map>
#include <string>
#include <time.h>

GxInterface::GxInterface()
{
    memset(&GxStats,0,sizeof(CCReqStats));
    startTime = 0;
    endTime   = 0;
}

int GxInterface::addPkt(Diameter &pkt)
{
 char timeBuf  [256]; 
 struct timeval tv;
 struct timezone tz;
 struct tm *tm;
 gettimeofday(&tv, &tz);
 tm=localtime(&tv.tv_sec);
 sprintf (timeBuf, "%02d:%02d:%02d:%03ld",tm->tm_hour, tm->tm_min, tm->tm_sec, (tv.tv_usec) );
 std::cout << "ABHINAY: time at start of adding packet:" << timeBuf << std::endl;
    std::map<unsigned int, std::map<uint32_t, unsigned int> >::iterator it;
    std::map<uint32_t, unsigned int>::iterator it1;
    std::map<uint32_t, unsigned int> tmp;

    if(pkt.getCC() != CCRorA)
    {
        return 1;
    }

    switch(pkt.getReqType())
    {
       case INITIAL:
           break;
       case UPDATE:
           break;
       case TERMINATE:
           break;
       default :
           return 1;
    }

    switch(pkt.getRequest())
    {
        case 1:
            /* Handle Request */
            incrementAttempts(pkt.getReqType());
            it = req.find(pkt.getReqType());
            if(it != req.end())
            {
                tmp =it->second;
            }

            tmp[pkt.getHopIdentifier()] = pkt.getTimestamp();
            req[pkt.getReqType()] = tmp;
            break;

        case 0:
            /* Handle Response */
            it =  req.find(pkt.getReqType());
            if(it != req.end())
            {
                tmp =it->second;
            }
            
            it1 = tmp.find(pkt.getHopIdentifier());
            if(it1 == tmp.end())
            {
                incrementUnKnwResCount(pkt.getReqType());
            }
            else
            {
                if(pkt.getResCode() < 3000 || pkt.getResCode() == 70001)
                {
                    incrementSuccCount(pkt.getReqType());
                }
                else
                {
                    incrementFailCount(pkt.getReqType());
                }

                updateLatency((pkt.getTimestamp()-(it1->second)), pkt.getReqType());
            }

            /* Delete the request from map */
            tmp.erase(pkt.getHopIdentifier()); 
            req[pkt.getReqType()] = tmp;
            break;

        default:
            return 1;
    }
 gettimeofday(&tv, &tz);
 tm=localtime(&tv.tv_sec);
 sprintf (timeBuf, "%02d:%02d:%02d:%03ld",tm->tm_hour, tm->tm_min, tm->tm_sec, (tv.tv_usec) );
 std::cout << "ABHINAY: time at end of adding packet:" << timeBuf << std::endl;
    return 0;
}

void GxInterface::incrementAttempts(int reqType)
{
    GxStats.attempts[reqType-1]++;
}

void GxInterface::incrementSuccCount(int reqType)
{
    GxStats.succCount[reqType-1]++;
}

void GxInterface::incrementFailCount(int reqType)
{
    GxStats.failCount[reqType-1]++;
}

void GxInterface::incrementTimeoutCount(int reqType)
{
    GxStats.timeoutCount[reqType-1]++;
}

void GxInterface::incrementUnKnwResCount(int reqType)
{
    GxStats.unKnwRes[reqType-1]++;
}

void GxInterface::updateLatency(double newLatency, int reqType)
{
    GxStats.latency[reqType-1] = ((GxStats.latency[reqType-1])*(GxStats.latencySize[reqType-1]) + newLatency) / (++GxStats.latencySize[reqType-1]);
}

void GxInterface::printStats()
{
    char TimeBuf[300];
    time_t curT = startTime;
    struct tm * curTimeInfo;
    curTimeInfo = localtime(&curT);
    strftime(TimeBuf, 100, "%F  %T", curTimeInfo);
    std::string curTime(TimeBuf);

    for(int i=0; i<3; i++)
    {
        std::string msgType;
        switch(i)
        {
            case 0:
                msgType = "INITIAL";
                break;
            case 1:
                msgType = "UPDATE";
                break;
            case 2:
                msgType = "TERMINATE";
                break;
        }

        std::cout << "splunk " << curTime << " DIAMETER " << "Type="     << msgType                 << " "
                                                          << "Attempts=" << GxStats.attempts[i]     << " "
                                                          << "Success="  << GxStats.succCount[i]    << " " 
                                                          << "Fail="     << GxStats.failCount[i]    << " " 
                                                          << "Timeout="  << GxStats.timeoutCount[i] << " " 
                                                          << "Latency="  << GxStats.latency[i]      << std::endl;
    }

    /*
    std::map<unsigned int, std::map<uint32_t, unsigned int> >::iterator it;
    std::map<uint32_t, unsigned int>::iterator it1;
    std::map<uint32_t, unsigned int> tmp;
    for(it = req.begin(); it != req.end(); it++)
    {
        int reqtype = it->first;
        tmp = it->second;
        switch(reqtype)
        {
            case 1:
                std::cout << "ABHINAY:: TIME OUT INITIAL REQUESTS" << std::endl;
                break;
            case 2:
                std::cout << "ABHINAY:: TIME OUT UPDATE REQUESTS" << std::endl;
                break;
            case 3:
                std::cout << "ABHINAY:: TIME OUT TERMINATE REQUESTS" << std::endl;
                break;
        }
        for(it1 = tmp.begin(); it1 != tmp.end(); it1++)
        {
            std::cout << "ABHINAY::hopId:" << it1->first << " Time:" << it1->second << std::endl; 
        }
    }
    */
}

void GxInterface::clearStats()
{
    memset(&GxStats,0,sizeof(CCReqStats));
    std::map<unsigned int, std::map<uint32_t, unsigned int> >::iterator it;
    std::map<uint32_t, unsigned int>::iterator it1;
    std::map<uint32_t, unsigned int> tmp;
    for(it = req.begin(); it != req.end(); it++)
    {
        int reqtype = it->first;
        tmp = it->second;

        for(it1 = tmp.begin(); it1 != tmp.end(); it1++)
        {
            if(it1->second + DIAMETER_TIMEOUT < endTime)
            {
                incrementTimeoutCount(reqtype);
                tmp.erase(it1->first); 
            }
        }
    }
}
