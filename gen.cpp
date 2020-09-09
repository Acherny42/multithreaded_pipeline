
#include <string>
#include <fstream>
#include <iostream>
#include <chrono>
#include <time.h>
#include <cstdlib>
#include <sys/time.h>
#include <limits.h>
#include <string.h>

#include "gen.h"
#include "pack.h"


gen::gen(link<pack>& out, link<std::string>& log): 
    thread_base(log),
    m_out_link(out)    
{
    srand(time(0));
}

uint gen::get_rand() {
    return rand();
}

uint gen::generate_id(){
    return CAN_IDS[get_rand() % NUM_IDS];
}

uint gen::generate_nbytes(){
    return get_rand() % (MAX_DATA_LEN_BYTES + 1);
}

uint gen::generate_interval(){
    return MIN_INTERVAL_MS + get_rand() % (MAX_INTERVAL_MS - MIN_INTERVAL_MS + 1);
}

void gen::generate_data(pack& p, uint nbytes) {

    LOG(("gen::generate_data nbytes"))

    for(uint i = 0; i < nbytes; i++) {
        uint b = get_rand() % (1 << CHAR_BIT );
        if(!p.set_data_byte(b, i)){
            LOG(("Error set_data_bytes"))
        }
    }

}

void gen::work() 
{

    
    uint id             = generate_id();
    uint data_nbytes    = generate_nbytes();
    uint interval_ms    = generate_interval();

    size_t size = GET_WIDTH(SOF) +
        GET_WIDTH(IDENTIFIER) +
        GET_WIDTH(RTR) +        
        GET_WIDTH(IDE) +
        GET_WIDTH(R) +
        GET_WIDTH(DLC) +
        CHAR_BIT * data_nbytes +
        GET_WIDTH(CHECKSUM) +
        GET_WIDTH(DEL) +
        GET_WIDTH(ACK) +
        GET_WIDTH(DEL) +
        GET_WIDTH(EOF);


    uint pnbytes = (size + CHAR_BIT - 1)/CHAR_BIT;
    pack p(pnbytes);

    memset(p.getBuff(), 255, pnbytes);

    struct timeval tp;
    gettimeofday(&tp, NULL);
    long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;

    LOG(("generating ").append("id ").append(std::to_string(id)).append(" bytes: ").append(std::to_string(data_nbytes)).append(" ms:").append(std::to_string(ms)));


    if(!p.set_ident(id)) {
        LOG(("Error setting id"))
    }

    if(!p.set_dlc(data_nbytes)) {
        LOG(("Error setting dlc"))
    }

    generate_data(p, data_nbytes);

    if(!m_out_link.push(std::move(p))) {
        LOG(("Error generator push failed "))
    }


    std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
}

gen::~gen() {
    LOG(("generator tedaching "))
    
    m_out_link.detach();
} 