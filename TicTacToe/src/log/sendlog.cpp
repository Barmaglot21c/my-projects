#include "sendlog.h"

void sendlog(std::string mes, bool close){
    static std::ofstream Logf("build/log.txt", std::ios::out | std::ios::trunc);
    if (close == true) Logf.close();
    else Logf<<mes<<std::endl;
}