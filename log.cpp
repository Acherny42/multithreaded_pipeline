#include <string>
#include <fstream>
#include <iostream>
#include <chrono>

#include "log.h"

log::log(link<std::string>& in, std::string file, link<std::string>& log): 
    thread_base(log),
    m_in_link(in),
    m_log(file)
{
    
}

void log::work() 
{

    
    std::string s;
    if(!m_in_link.pop(s)){
        
        LOG(("log pop failed terminating "))

        thread_base::stop();//empty and detached producer.
        return;
    }
    
    LOG(("logging: ").append(s))

    m_log << s;
    
}


log::~log() {
  LOG(("logger detaching "))
  m_log.close();
} 