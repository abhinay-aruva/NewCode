#include "GxInterface.h"
#include <unordered_map>
#include <string>

GxInterface::GxInterface(std::string &nodepair)
{
    memset(&GxStats,0,sizeof(CCGxStats));
    startTime = 0;
    endTime   = 0;
    reqtype   = 0;
    initialiseShf(nodepair);
}

int GxInterface::addPkt(Diameter &pkt)
{
    std::cout << "ABHINAY:: Got a Gx packet" << std::endl;
    static int uid = 0;
    static double TS = 0; 

    if(pkt.cc != CCRorA)
    {
        return 1;
    }

    reqtype = pkt.reqType;
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

            char buf[250];
            std::cout <<  std::fixed << "ABHINAY:: Pkt Hop id is :" << pkt.hopIdentifier << std::endl;
            sprintf(buf,"%d",pkt.hopIdentifier);
            std::cout <<  std::fixed << "ABHINAY:: Buf Hop id is :" << buf << std::endl;
            std::string key(buf);
            sprintf(buf,"%f",pkt.timeStamp);
            std::string keyV(buf);
            std::cout <<  std::fixed << "ABHINAY:: Pkt time stamp is:" << pkt.timeStamp << std::endl;
            std::cout << "ABHINAY:: Key time stamp is:" << keyV << std::endl;
            static int count = 0;
    switch(pkt.request)
    {
        case 1:
            /* Handle Request */
            shfrql->MakeHash(key.c_str(), key.length());
            std::cout << "ABHINAY:: Got a request: hopid:" << pkt.hopIdentifier << " timestamp:" << keyV.c_str() << std::endl;
            req[reqtype][pkt.hopIdentifier] = shfrql->PutKeyVal(keyV.c_str(), keyV.length());
            std::cout << "ABHINAY:: Uid inserted is: " << req[reqtype][pkt.hopIdentifier] << std::endl;
             
            break;

        case 0:
            /* Handle Response */
            std::cout << "ABHINAY:: Got a response packet" << std::endl;
            uid = 0;
            TS  = 0; 
            bzero(shf_val,sizeof(shf_val));
            uid = req[reqtype][pkt.hopIdentifier];
            if(uid == 0)
            {
                shfrql->MakeHash(key.c_str(), key.length());
                if(!shfrql->GetKeyValCopy())
                {
                    TS=atof(shf_val);
                    shfrql->DelKeyVal();
                    std::cout << "TIme stamp from no UID of request is:" << TS << std::endl;
                }
                else
                {
                    GxStats.timeoutCount[reqtype-1]++;
                    return 0;
                }
            }
            else
            {
                if(!shfrql->GetUidValCopy(uid))
                {
                    TS=atof(shf_val);
                    shfrql->DelUidVal(uid);
                    std::cout << "TIme stamp from UID of request is:" << TS << std::endl;
                }
            }
           
            if(pkt.resCode < 3000 || pkt.resCode == 70001)
            {
                GxStats.succCount[reqtype-1]++;
            }
            else
            {
                GxStats.failCount[reqtype-1]++;
            }
            GxStats.attempts[reqtype-1]++;

            break;

        default:
            return 1;
    }
    return 0;
}

void GxInterface::printStats(std::string &node)
{
    /*
    curT = startTime;
    static int RTTCount;
    // Calculate latency 
    it=res.begin();
    while(it != res.end())
    {
        RTTCount = 0;
        tmp=&(it->second);
        it1=tmp->begin();
        while(it1 != tmp->end())
        {
           reqIt = req.find(it->first);
           if (reqIt != req.end())
           {
               reqTmp = &(reqIt->second);
               reqIt1 = reqTmp->find(it1->first);
               if(reqIt1 !=  reqTmp->end())
               {
                   GxStats.latency[(it->first)-1] = ((RTTCount * GxStats.latency[(it->first)-1]) + (it1->second)-(reqIt1->second))/(++RTTCount);
                   reqTmp->erase(reqIt1);
               }
               else
               {
                   GxStats.timeoutCount[(it->first)-1]++;
               }
           }
           else
           {
               GxStats.timeoutCount[(it->first)-1]++;
           }
           it1++;
        }
        it++;
    }

    reqIt = req.begin();
    while(reqIt != req.end())
    {
        reqTmp =&(reqIt->second);
        reqIt1 = reqTmp->begin();
        while(reqIt1 != reqTmp->end())
        {
            if(reqIt1->second + DIAMETER_TIMEOUT < endTime)
            {
                GxStats.timeoutCount[(reqIt->first)-1]++;
                reqTmp->erase(reqIt1++);
            }
            else
            {
                reqIt1++;
            }
        }
        reqIt++;
    }

    // Print Stats
    curTimeInfo = localtime(&curT);
    strftime(TimeBuf, 100, "%F  %T", curTimeInfo);
    std::string curTime(TimeBuf);

    for(int i=INITIAL; i<= TERMINATE; i++)
    {
        std::string msgType;
        switch(i)
        {
            case INITIAL:
                msgType = "INITIAL";
                break;
            case UPDATE:
                msgType = "UPDATE";
                break;
            case TERMINATE:
                msgType = "TERMINATE";
                break;
        }
        std::cout << curTime << " " << node <<   " Ix=" << "Gx"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Att"  
                                                          << " Kpv=" << GxStats.attempts[i-1]     << std::endl;

        std::cout << curTime << " " << node <<   " Ix=" << "Gx"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Suc"
                                                          << " Kpv="  << GxStats.succCount[i-1]     << " " << std::endl;

       std::cout << curTime << " " << node <<   " Ix=" << "Gx"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Fail"
                                                          << " Kpv="      << GxStats.failCount[i-1]    << " " << std::endl;
       std::cout << curTime << " " << node <<   " Ix=" << "Gx"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Tout"
                                                          << " Kpv="      << GxStats.timeoutCount[i-1]    << " " << std::endl;


       std::cout << curTime << " " << node <<   " Ix=" << "Gx"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Laty"
                                                          << " Kpv=" <<  (int)(GxStats.latency[i-1] * 1000000)<< std::endl; 
    }*/
}

void GxInterface::clearStats()
{
    memset(&GxStats,0,sizeof(CCGxStats));
    //req.clear();
    res.clear();
}
