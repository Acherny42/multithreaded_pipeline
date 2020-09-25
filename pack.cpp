#include <thread>
#include <string>
#include <fstream>
#include <iostream>
#include <limits.h>


#include "pack.h"

pack::pack(): 
    m_data(nullptr),
    m_size(0)
{

}

pack::pack(size_t size): 
    m_data( (size > 0) ? std::make_unique<uchar[]>(size) : nullptr),
    m_size(size)
{

}

void pack::move_comm(pack& p) {

    m_data = std::move(p.m_data);
    p.m_data = nullptr;
    m_size = p.m_size;
}

pack& pack::operator=(pack&& p) noexcept
{
    if(this == &p){
        return *this;
    }

    move_comm(p);

    return *this;
}

uchar* pack::getBuff() const{
    return m_data.get();
}

pack::pack(pack&& p) noexcept
{    
    move_comm(p);
}

bool pack::isNull() const
{
    return nullptr == m_data;
}

size_t pack::getSize() const{
    return m_size;
}

bool pack::set_ident(int id ) {
    if(isNull()){
        return false;
    }

    set_bits_int(id, getBuff(), GET_INDEX(IDENTIFIER), GET_WIDTH(IDENTIFIER));
    return true;
}

bool pack::set_dlc(int dlc ) {
    if(isNull()){
        return false;
    }

    set_bits_int(dlc, getBuff(), GET_INDEX(DLC), GET_WIDTH(DLC));
    return true;
}

bool pack::set_data_byte(uchar byte, uint nbyte ) {
    if(isNull() || nbyte > MAX_DATA_LEN_BYTES ){
        return false;
    }

    set_bits_int(byte, getBuff(), GET_INDEX(DATA) + GET_WIDTH(DATA) * nbyte, GET_WIDTH(DATA));
    return true;
}


///------------ gets -----------
bool pack::get_ident(int& id ) {
    if(isNull()){
        return false;
    }

    id = get_bits_int( getBuff(), GET_INDEX(IDENTIFIER), GET_WIDTH(IDENTIFIER));
    return true;
}
 
bool pack::get_dlc(int& dlc ) {
    if(isNull()){
        return false;
    }

    dlc = get_bits_int( getBuff(), GET_INDEX(DLC), GET_WIDTH(DLC));
    return true;
}

uint pack::bit_byte(uint i) {
    return ((i+CHAR_BIT)/CHAR_BIT) - 1;
}

uint pack::h_shift_l(uint i) {
    return (bit_byte(i) + 1) * CHAR_BIT - i - 1;
}

bool pack::get_data_byte(uchar& byte, uint nbyte ) {
    if(isNull() || nbyte > MAX_DATA_LEN_BYTES){
        return false;
    }

    byte = get_bits_int( getBuff(), GET_INDEX(DATA) + GET_WIDTH(DATA) * nbyte, GET_WIDTH(DATA));
    return true;
}


//assumes len <= CHAR_BIT
void pack::set_byte(uchar b, uint i, uchar* arr, uint len) {
    uint hb = bit_byte(i);
    uchar hs = h_shift_l(i);

    if(CHAR_BIT < len) {
        printf("%s len > 8", __FUNCTION__);
    }

    uchar ls = CHAR_BIT - hs;

    b = b & (255 >> (CHAR_BIT - len));

    uchar m = 255 >> ls;
    if(ls > len)
        m = m | ~(255 >> (ls - len));

    arr[hb] = arr[hb] & m;

    uchar slb = b << hs;
    arr[hb] = arr[hb] | slb;

    if(0 == hb || ls >= len )
        return;

    m = 255 << (len - ls);
    arr[hb-1] = arr[hb-1] & m;
    uchar srb = b >> ls;
    arr[hb-1] = arr[hb-1] | srb;

}

//assumes len <= CHAR_BIT
uchar pack::get_byte( uint i, const uchar* arr, uint len) {
    uint hb = bit_byte(i);
    uint hs = h_shift_l(i);

    if(CHAR_BIT < len) {
        printf("%s len > 8", __FUNCTION__);
    }


    uchar cm = ~(255 << len);

    uchar hm = arr[hb];
    hm = hm >> hs;

    if(0 == hb || CHAR_BIT - hs >= len) {
        return hm & cm;
    }

    uchar lm = arr[hb-1];    
    lm = lm << (CHAR_BIT -hs);

    return (lm | hm) & cm ;
    
}


void pack::set_bits_int(int u, uchar* out, uint out_i, uint len) {

    uchar * uc = (uchar*)&u;

    uchar wb = get_byte(CHAR_BIT-1, uc, len < CHAR_BIT ? len : CHAR_BIT);
    set_byte(wb, out_i, out, len < CHAR_BIT ? len : CHAR_BIT );

    if( len <= CHAR_BIT )
        return;

    uint i = 0;
    uint offset_out = 0;
    uint offset_in = 0;
    for(i = 0 ; i < (len - CHAR_BIT) / CHAR_BIT ; i++) {
        offset_out = CHAR_BIT + i * CHAR_BIT;
        offset_in = CHAR_BIT * 2 + CHAR_BIT * i - 1;
        wb = get_byte(offset_in, uc, CHAR_BIT );
        set_byte(wb, out_i - offset_out, out, CHAR_BIT );
    }

    if(0 == len % CHAR_BIT)
        return;

    offset_out = CHAR_BIT + i * CHAR_BIT;
    offset_in = CHAR_BIT * 2 + CHAR_BIT * i - 1;
    wb = get_byte(offset_in, uc, len % CHAR_BIT );
    set_byte(wb, out_i - offset_out, out, len % CHAR_BIT );

}

int pack::get_bits_int(const uchar* in, uint in_i, uint len) {

    uint r =  0;

    uchar wb = get_byte(in_i, in, len < CHAR_BIT ? len : CHAR_BIT);
    set_byte(wb, CHAR_BIT-1, (uchar*)&r, len < CHAR_BIT ? len : CHAR_BIT);

    if( len <= CHAR_BIT )
        return r;

    uint i = 0;
    uint offset_out = 0;
    uint offset_in = 0;
    for(i = 0 ; i < (len - CHAR_BIT) / CHAR_BIT ; i++) {
        offset_in = CHAR_BIT + i * CHAR_BIT;
        offset_out = CHAR_BIT * 2 + CHAR_BIT * i - 1;
        wb = get_byte(in_i - offset_in, in, CHAR_BIT );
        set_byte(wb, offset_out, (uchar*)&r, CHAR_BIT );
    }

    if(0 == len % CHAR_BIT)
        return r;

    offset_in = CHAR_BIT + i * CHAR_BIT;
    offset_out = CHAR_BIT * 2 + CHAR_BIT * i - 1;
    wb = get_byte(in_i - offset_in, in, len % CHAR_BIT );
    set_byte(wb, offset_out, (uchar*)&r, len % CHAR_BIT );

    return r;
}

void pack::set_bits(const uchar* in, uint in_i,  uchar* out, uint out_i, uint len) {

    uchar wb = get_byte(in_i, in, len < CHAR_BIT ? len : CHAR_BIT);
    set_byte(wb, out_i, out, len < CHAR_BIT ? len : CHAR_BIT );

    if( len <= CHAR_BIT )
        return;

    uint i = 0;
    uint offset_out = 0;
    uint offset_in = 0;
    for(i = 0 ; i < (len - CHAR_BIT) / CHAR_BIT ; i++) {
        offset_out = CHAR_BIT + i * CHAR_BIT;
        offset_in = CHAR_BIT +  i * CHAR_BIT ;
        wb = get_byte(in_i - offset_in, in, CHAR_BIT );
        set_byte(wb, out_i - offset_out, out, CHAR_BIT );
    }

    if(0 == len % CHAR_BIT)
        return;

    offset_out = CHAR_BIT + i * CHAR_BIT;
    offset_in = CHAR_BIT +  i * CHAR_BIT ;
    wb = get_byte(in_i - offset_in, in, len % CHAR_BIT );
    set_byte(wb, out_i - offset_out, out, len % CHAR_BIT );

}

pack::~pack()
{    

}

