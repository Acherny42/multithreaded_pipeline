
#include <thread>
#include "pack.h"
#include "link.h"
#include "defines.h"


#ifndef CAN_THREAD_H
#define CAN_THREAD_H

class thread_base {

public:

    thread_base(link<std::string>& log);
    virtual void work()=0;
    void loop();
    ~thread_base();
    void start();
    void stop();
    void log(std::string&& s);

protected:
    std::mutex m_unit_test_mutex;
private:
    bool m_start;
    bool m_terminate;
    std::mutex m_mutex;
    std::condition_variable m_cv;    

protected:
    static std::string uchar_to_hex(uint n);
    link<std::string>& m_out_log; 
private:
    std::thread m_th;    
};

#endif