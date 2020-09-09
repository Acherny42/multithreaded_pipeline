
#include <memory>
#include "defines.h"
#include <limits.h>

#ifndef CAN_PAK_H
#define CAN_PAK_H

#define MAX_DATA_LEN_BYTES 8
#define MAX_PACKET_LEN_BYTES \
        (( CAN_BITS_WIDTH_SOF +\
        CAN_BITS_WIDTH_IDENTIFIER +\
        CAN_BITS_WIDTH_RTR +\
        CAN_BITS_WIDTH_IDE +\
        CAN_BITS_WIDTH_R +\
        CAN_BITS_WIDTH_DLC +\
        MAX_DATA_LEN_BYTES * CHAR_BIT +\
        CAN_BITS_WIDTH_CHECKSUM +\
        CAN_BITS_WIDTH_DEL +\
        CAN_BITS_WIDTH_ACK +\
        CAN_BITS_WIDTH_DEL +\
        CAN_BITS_WIDTH_EOF + CHAR_BIT - 1 )/CHAR_BIT)

#define NUM_IDS 3
const int CAN_IDS[NUM_IDS] = {0x100, 0x200, 0x300};
#define MAX_BYTE_VAL 255

#define GET_WIDTH(X)\
CAN_BITS_WIDTH_##X

#define GET_OFFSET(X)\
CAN_BITS_OFFSET_##X

#define GET_INDEX(X)\
GET_OFFSET(X) + GET_WIDTH(X) - 1


#define CAN_BITS_OFFSET_IDENTIFIER          1
#define CAN_BITS_WIDTH_IDENTIFIER           11

#define CAN_BITS_OFFSET_DLC                 15
#define CAN_BITS_WIDTH_DLC                  4

#define CAN_BITS_OFFSET_DATA                19
#define CAN_BITS_WIDTH_DATA                 8
  
#define CAN_BITS_WIDTH_SOF                  1
#define CAN_BITS_WIDTH_RTR                  1
#define CAN_BITS_WIDTH_IDE                  1
#define CAN_BITS_WIDTH_R                    1
#define CAN_BITS_WIDTH_CHECKSUM             15
#define CAN_BITS_WIDTH_DEL                  1
#define CAN_BITS_WIDTH_ACK                  1
#define CAN_BITS_WIDTH_EOF                  7



class pack {

public:    
    pack();
    explicit pack(size_t size) ;

    pack(const pack& p)             =delete;
    pack(pack& p)                   =delete;
    pack& operator=(const pack& p)  =delete;
    pack& operator=(pack& p)        =delete;

    pack(pack&& p)              noexcept;
    pack& operator=(pack&& p)   noexcept;

    bool isNull() const;

    size_t getSize() const;
    uchar* getBuff() const;

    bool set_ident(int id );
    bool set_dlc(int dlc );
    bool set_data_byte(uchar data, uint nbyte );

    bool get_ident(int& id );
    bool get_dlc(int& dlc );
    bool get_data_byte(uchar& data, uint nbyte );


    ~pack();

private:

    uint bit_byte(uint i);
    uint h_shift_l(uint i);
    uchar get_byte( uint i, const uchar* arr, uint len);
    void  set_byte(uchar b, uint i, uchar* arr, uint len);
    void  set_bits_int(int u, uchar* out, uint out_i, uint len);
    int   get_bits_int(const uchar* in, uint in_i, uint len);


    void move_comm(pack& p);
    
    std::unique_ptr<uchar[]> m_data;
    size_t m_size;
};


#endif

