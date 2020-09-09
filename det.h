
#include "pack.h"
#include "link.h"
#include "thread_base.h"
#include "gen.h"

#ifndef CAN_DET_H
#define CAN_DET_H

#define MIN_VALID_TIME_INTERVAL_MS 100

#define REASON_INVALID     "INVALID"
#define REASON_VALID       "VALID"

class det : public thread_base {

public:

    det(link<pack>& in, link<std::string>& out, link<std::string>& log);
    void work() override;
    void log(std::string& s);
    ~det();

private:

    std::string data_to_hex(uchar* data, uint len, uint max_alighn);

    typedef struct frame_info {
        uchar m_data[MAX_DATA_LEN_BYTES]; 
        int m_id;
        int m_dlc;
        long int m_ms;
    } frame_info;


    link<pack>& m_in_link;
    link<std::string>& m_out_link;

    frame_info m_frames_inf[NUM_IDS];
};

#endif