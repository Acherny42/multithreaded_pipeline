#include "pack.h"
#include "link.h"
#include "thread_base.h"

#ifndef CAN_LOG_H
#define CAN_LOG_H

class log : public thread_base {

public:

    log(link<std::string>& in, std::string file, link<std::string>& log);
    void work() override;
    ~log();
private:
    link<std::string>& m_in_link;
    std::ofstream m_log;
};

#endif