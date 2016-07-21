#include <unordered_map>
#include <string>
#include <time.h>
#include "S6bInterface.h"

S6BInterface::S6BInterface(std::string &nodepair)
{
    memset(&s6bStats,0,sizeof(s6bStats));
    startTime = 0;
    endTime   = 0;
    initialiseShf(nodepair);
}

int S6BInterface::addPkt(Diameter &pkt)
{
    static int uid    = 0;
    static double RTT = 0;
    static double TS  = 0; 
    msgType = DEFAULT;  

    switch(pkt.cc)
    {
        case S6BAA:
            msgType = AA;
            break;
        case S6BTERMINATE:
            msgType = TERM;
            break;
        default:
            return 1;
    }

    std::string key  = std::to_string(pkt.hopIdentifier);
    std::string keyV =  std::to_string(pkt.timeStamp);

    switch(pkt.request)
    {
        case 1:
            /* Handle Request */
            shfrql->MakeHash(key.c_str(), key.length());
            req[msgType][pkt.hopIdentifier] = shfrql->PutKeyVal(keyV.c_str(), keyV.length());
            break;

        case 0:
            /* Handle Response */
             uid = 0;
             TS  = 0;
             RTT = 0;
             bzero(shf_val,sizeof(shf_val));

             uid = req[msgType][pkt.hopIdentifier];
             if(uid == 0)
             {
                shfrql->MakeHash(key.c_str(), key.length());
                if(shfrql->GetKeyValCopy())
                {
                    TS=atof(shf_val);
                    shfrql->DelKeyVal();
                }
                else
                {
                    res[msgType][pkt.hopIdentifier] = pkt.timeStamp;
                    return 0;
                }
             }
             else
             {
                if(shfrql->GetUidValCopy(uid))
                {
                    TS=atof(shf_val);
                    shfrql->DelUidVal(uid);
                }
            }

            // Sucess or failure stats 
            if(pkt.resCode < 3000 || pkt.resCode == 70001)
            {
                s6bStats.succCount[msgType]++;
            }
            else
            {
                s6bStats.failCount[msgType]++;
            }

            // Latency stats
            RTT = pkt.timeStamp - TS;
            if(RTT > 40)
            {
                s6bStats.timeoutCount[msgType]++;
                return 0;
            }
            else if(RTT < 0)
            {
                return 0;
            }

            s6bStats.latency[msgType] += RTT; 
            s6bStats.latencySize[msgType]++;
            break;

        default:
            return 1;
    }
    return 0;
}

void S6BInterface::printStats(std::string &node)
{
    curT = startTime;

    /* Print Stats */
    curTimeInfo = localtime(&curT);
    strftime(TimeBuf, 100, "%F  %T", curTimeInfo);
    std::string curTime(TimeBuf);

    for(int i=AA; i<= TERM; i++)
    {
        std::string msgType;
        switch(i)
        {
            case AA:
                msgType = "AA";
                break;
            case TERM:
                msgType = "TERMINATE";
                break;
        }

        std::cout << curTime << " " << node <<   " Ix=" << "S6B"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Att"  
                                                          << " Kpv=" << s6bStats.attempts[i]     << std::endl;

        std::cout << curTime << " " << node <<   " Ix=" << "S6B"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Suc"
                                                          << " Kpv="  << s6bStats.succCount[i]     << " " << std::endl;

       std::cout << curTime << " " << node <<   " Ix=" << "S6B"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Fail"
                                                          << " Kpv="      << s6bStats.failCount[i]    << " " << std::endl;
       std::cout << curTime << " " << node <<   " Ix=" << "S6B"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Tout"
                                                          << " Kpv="   << s6bStats.timeoutCount[i] << " " << std::endl;

       if(s6bStats.latencySize[i] > 0)
       {
          std::cout << curTime << " " << node <<   " Ix=" << "S6B"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Laty"
                                                          << " Kpv=" << (int) ((s6bStats.latency[i]/s6bStats.latencySize[i])*1000) << std::endl; 
       }
    }
}

void S6BInterface::clearStats()
{
    memset(&s6bStats,0,sizeof(s6bStats));
    req.clear();
    res.clear();
}
