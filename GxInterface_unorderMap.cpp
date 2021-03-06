#include "GxInterface.h"
#include <unordered_map>
#include <string>

GxInterface::GxInterface()
{
    memset(&GxStats,0,sizeof(CCGxStats));
    startTime = 0;
    endTime   = 0;
}

int GxInterface::addPkt(Diameter &pkt)
{
    //std::cout << "ABHINAY:: Got a new packet with time stamp:" << pkt.timeStamp << std::endl;
    //printStats();
    std::unordered_map<unsigned int, std::unordered_map<uint32_t, unsigned int> >::iterator it;
    std::unordered_map<uint32_t, unsigned int>::iterator it1;
    std::unordered_map<uint32_t, unsigned int> tmp;

    if(pkt.cc != CCRorA)
    {
        return 1;
    }

    unsigned int reqtype = pkt.reqType;
    switch(reqtype)
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

    switch(pkt.request)
    {
        case 1:
            /* Handle Request */
            GxStats.attempts[reqtype-1]++;
            req[reqtype][pkt.hopIdentifier] = pkt.timeStamp;
            break;

        case 0:
            /* Handle Response */
            
            if(pkt.resCode < 3000 || pkt.resCode == 70001)
            {
                GxStats.succCount[reqtype-1]++;
            }
            else
            {
                GxStats.failCount[reqtype-1]++;
            }
            res[reqtype][pkt.hopIdentifier] = pkt.timeStamp;

            break;

        default:
            return 1;
    }
    return 0;
}

void GxInterface::printStats()
{
    std::unordered_map<unsigned int, std::unordered_map<uint32_t, unsigned int> >::iterator it;
    std::unordered_map<uint32_t, unsigned int>::iterator it1;
    std::unordered_map<uint32_t, unsigned int> *tmp;

    std::unordered_map<unsigned int, std::unordered_map<uint32_t, unsigned int> >::iterator reqIt;
    std::unordered_map<uint32_t, unsigned int>::iterator reqIt1;
    std::unordered_map<uint32_t, unsigned int> *reqTmp;


    it=res.begin();
    while(it != res.end())
    {
        tmp=&(it->second);
        it1=tmp->begin();
        while(it1 != tmp->end())
        {
           reqIt = req.find(it->first);
           //std::cout << "ABHINAY:: After reqIt" << std::endl;
           if (reqIt != req.end())
           {
               reqTmp = &(reqIt->second);
               reqIt1 = reqTmp->find(it1->first);
               if(reqIt1 !=  reqTmp->end())
               {
                   GxStats.latency[it->first] += (it1->second)-(reqIt1->second);
                   reqTmp->erase(reqIt1);
               } 
           }

           //if(req[it->first][it1->first] !=0)
           //{
           //    GxStats.latency[it->first] += (it1->second)-(req[it->first][it1->first]);
              //std::cout << "Diff:" << (it1->second)-(req[it->first][it1->first]) << std::endl;
           //}
           it1++;
        }

        //std::cout << "ABHINAY LATENCY is :" << GxStats.latency[it->first] << std::endl;
        it++;
    }

    char TimeBuf[300];
    time_t curT = startTime;
    struct tm * curTimeInfo;
    curTimeInfo = localtime(&curT);
    strftime(TimeBuf, 100, "%F  %T", curTimeInfo);
    std::string curTime(TimeBuf);

    for(int i=0; i< EVENT; i++)
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
        std::cout << curTime << " Ip=" << node <<   " Ix=" << "Gx"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Att"  
                                                          << " Kpv=" << GxStats.attempts[i]     << std::endl;

        std::cout << curTime << " Ip=" << node <<   " Ix=" << "Gx"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Suc"
                                                          << " Kpv="  << GxStats.succCount[i]     << " " << std::endl;

       std::cout << curTime << " Ip=" << node <<   " Ix=" << "Gx"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Fail"
                                                          << " Kpv="      << GxStats.failCount[i]    << " " << std::endl;
       std::cout << curTime << " Ip=" << node <<   " Ix=" << "Gx"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Tout"
                                                          << " Kpv="      << GxStats.timeoutCount[i]    << " " << std::endl;


       std::cout << curTime << " Ip=" << node <<   " Ix=" << "Gx"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Laty"
                                                          << " Kpv=" <<  (float)GxStats.latency[i] << std::endl; 
    }
}

void GxInterface::clearStats()
{
    memset(&GxStats,0,sizeof(CCGxStats));
    req.clear();
    res.clear();
    /*
    std::unordered_map<unsigned int, std::unordered_map<uint32_t, unsigned int> >::iterator it;
    std::unordered_map<uint32_t, unsigned int>::iterator it1;
    std::unordered_map<uint32_t, unsigned int> tmp;
    for(it = req.begin(); it != req.end(); it++)
    {
        int reqtype = it->first;
        tmp = it->second;

        for(it1 = tmp.begin(); it1 != tmp.end(); it1++)
        {
            if(it1->second + DIAMETER_TIMEOUT < endTime)
            {
                GxStats.timeoutCount[reqtype-1]++;
                it1=tmp.erase(it1->first); 
            }
        }
        req[reqtype] = tmp;
    }
    */
}
