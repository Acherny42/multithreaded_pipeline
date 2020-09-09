#include <thread>
#include <string>
#include <fstream>
#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>

#include "gen.h"


thread_base::thread_base(link<std::string>& log):     
    m_start(false),
    m_terminate(false),
    m_out_log(log),
    m_th(&thread_base::loop, this)
{
    
}

void thread_base::loop() 
{
    while(!m_terminate) {
        if(m_start){
            work();
        } else {
            std::unique_lock<std::mutex> lock(m_mutex);
            while( !m_start && !m_terminate) {
                m_cv.wait(lock);
            }
        }
    }
}

void thread_base::start() {

    std::unique_lock<std::mutex> lock(m_mutex);

    m_start = true;
    lock.unlock();
    m_cv.notify_one();
}

void thread_base::stop() {

    std::unique_lock<std::mutex> lock(m_mutex);

    m_start = false;
    lock.unlock();
}

void thread_base::log(std::string&& s) {
    if(!m_out_log.push(std::move(s))){
         std::cout << " error loging:" << s << std::endl;
    }    
}

std::string thread_base::uchar_to_hex(uint n) {

    std::stringstream stream;
    stream << std::setfill ('0') << std::setw(2) << std::hex << std::uppercase << n;

    return stream.str();
}

thread_base::~thread_base(){
    
    std::unique_lock<std::mutex> lock(m_mutex);

    m_terminate = true;
    lock.unlock();
    m_cv.notify_all();

    LOG(("Joining"))
    m_th.join();

}
