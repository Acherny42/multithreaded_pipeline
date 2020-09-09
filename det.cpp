#include <string>
#include <fstream>
#include <iostream>
#include <chrono>
#include <sys/time.h>
#include <cstring>


#include "det.h"



#define REASON_UNKNOWN     "unk"
#define REASON_DLC         "dlc"
#define REASON_ID          "id"
#define INVALID_VALUE -1


using namespace std::chrono;


det::det(link<pack>& in, link<std::string>& out, link<std::string>& log): 
    thread_base(log),
    m_in_link(in),
    m_out_link(out)
{

    for(uint i = 0 ; i < NUM_IDS; i++ ) {
        m_frames_inf[i].m_id = CAN_IDS[i];
        m_frames_inf[i].m_dlc = INVALID_VALUE;
        m_frames_inf[i].m_ms = INVALID_VALUE;
    }

}

std::string det::data_to_hex(uchar* data, uint len, uint max_alighn) {
    std::string r;
    uint i = 0;
    for(i = 0 ; i < len ; i++) {
        r.append("0X").append(uchar_to_hex((uint)data[i])).append(" ");
    }

    for(; i < max_alighn ; i++) {
        r.append("  ").append("   ");
    }

    return r;
}

void det::work() 
{
    
    
    pack p;
    if(!m_in_link.pop(p)){
        LOG(("detector pop failed terminating "))

        thread_base::stop();//empty, and detached producer.
        return;
    }

    std::string status_id = REASON_VALID;
    std::string status_dlc = REASON_VALID;
    std::string reason_rate = REASON_VALID;
    std::string reason_length = REASON_VALID;
    std::string reason_data = REASON_VALID;
    std::string status = REASON_VALID;

    //validate id
    int tmp_id = 0;
    if(!p.get_ident(tmp_id)) {
        LOG(("Error get_ident "))
    }
    {
        int i = 0;
        for(i = 0; i < NUM_IDS; i++) {
            if(tmp_id == CAN_IDS[i]){
                break;
            }
        }
        if(NUM_IDS == i) {
            status_id = REASON_INVALID;
            LOG(("Error id is invalid").append(std::to_string(tmp_id)));
        }
    }

    uchar tmp_data[MAX_DATA_LEN_BYTES];

    int tmp_dlc = 0;
    if(!p.get_dlc(tmp_dlc)) {
        LOG(("Error get.dlc"))
    }

    //validate dlc
    if( tmp_dlc > 0 && MAX_DATA_LEN_BYTES < tmp_dlc) {
        LOG(("Error MAX_DATA_LEN_BYTES <= dlc"));
        status_dlc = REASON_INVALID;
    } else {     
        //get data   
        for(int i = 0; i < tmp_dlc; i++) {
            if(!p.get_data_byte(tmp_data[i], i)) {
                LOG(("Error get_data_byte"))
            }
        }
    }


    struct timeval tp;
    gettimeofday(&tp, NULL);
    long int tmp_ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
    long int tmp_delta = INVALID_VALUE;

    if(status_id != REASON_INVALID && status_dlc != REASON_INVALID) {
        for(uint i = 0 ; i < NUM_IDS ; i ++) {
            if(tmp_id == m_frames_inf[i].m_id) {

                //check rate
                if(INVALID_VALUE != m_frames_inf[i].m_ms) {
                    tmp_delta = tmp_ms - m_frames_inf[i].m_ms;
                    if(tmp_delta < MIN_VALID_TIME_INTERVAL_MS) {
                        status = REASON_INVALID;
                        reason_rate = REASON_INVALID;
                    }
                }

                //check dlc
                if(INVALID_VALUE != m_frames_inf[i].m_dlc) {
                    if(m_frames_inf[i].m_dlc == tmp_dlc) {
                        status = REASON_INVALID;
                        reason_length = REASON_INVALID;
                    }
                }

                //check data
                if(INVALID_VALUE != m_frames_inf[i].m_dlc) {
                    for(int k = 0; k < tmp_dlc - 1 ; k++) {
                        for(int j = 0 ; j < m_frames_inf[i].m_dlc; j++ ) {
                            if(m_frames_inf[i].m_data[j] == tmp_data[k]) {
                                status = REASON_INVALID;
                                reason_data = REASON_INVALID;
                                break;
                            }
                        }
                    }
                }

                break;//id found
            }
        }
    }


    std::string debug_log;
    debug_log.append("detecting ").
    append("id:").append(std::to_string(tmp_id).
    append(" dlc:").append(std::to_string(tmp_dlc))).
    append(" ms:").append(std::to_string(tmp_ms)).
    append(" data:").append((status_dlc != REASON_INVALID ) ? data_to_hex(tmp_data, tmp_dlc, MAX_DATA_LEN_BYTES) : data_to_hex(tmp_data, 0, MAX_DATA_LEN_BYTES)).
    append(" status:").append(status).
    append(" rate:").append(reason_rate).
    append(" length:").append(reason_length).
    append(" data:").append(reason_data).
    append(" delta:").append(std::to_string((status_id != REASON_INVALID ) ? (tmp_delta):(INVALID_VALUE))).
    append("\n");

    LOG((debug_log))

    std::string to_log;
    to_log.append(std::to_string(tmp_ms)).
    append(",").append(data_to_hex(p.getBuff(), p.getSize() , MAX_PACKET_LEN_BYTES)).
    append(",").append(status);
    if(REASON_VALID != status) {
        if(REASON_VALID != reason_rate) {
            to_log.append(",RATE");        
        }
        if(REASON_VALID != reason_length) {
            to_log.append(",LENGTH");        
        }
        if(REASON_VALID != reason_data) {
            to_log.append(",DATA");        
        }
    }
    to_log.append("\n");


    if(!m_out_link.push(std::move(to_log))) {
        std::cout << "push logging failed " << std::endl;
    }


    if(status_dlc != REASON_INVALID && status_id != REASON_INVALID /*&& status == REASON_VALID*/) {

        for(uint i = 0 ; i < NUM_IDS ; i ++) {
            if(tmp_id == m_frames_inf[i].m_id) {

                m_frames_inf[i].m_dlc = tmp_dlc;
                m_frames_inf[i].m_id = tmp_id;
                m_frames_inf[i].m_ms = tmp_ms;

                for(int j = 0 ; j < tmp_dlc; j++ ) {
                    m_frames_inf[i].m_data[j] = tmp_data[j];
                }

                break; //id found
            }
        }
    }
}


det::~det() {

    LOG(("detector tedaching "))

    m_out_link.detach();
} 
