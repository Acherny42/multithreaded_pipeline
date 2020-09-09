#include "pack.h"
#include "link.h"
#include "thread_base.h"

#ifndef CAN_GEN_H
#define CAN_GEN_H

#define MAX_INTERVAL_MS 100
#define MIN_INTERVAL_MS 50


class gen : public thread_base {

public:

    gen(link<pack>& out, link<std::string>& log);
    void work() override;
    ~gen();

    virtual uint get_rand();
    virtual uint generate_id();
    virtual uint generate_nbytes();
    virtual uint generate_interval();
    virtual void generate_data(pack& p, uint nbytes);

private:
    link<pack>& m_out_link;
    
};

#endif

