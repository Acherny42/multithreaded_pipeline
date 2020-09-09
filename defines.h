#ifndef CAN_DEFINES_H
#define CAN_DEFINES_H


#define BYTES 8

typedef unsigned int uint;
typedef unsigned char uchar;


#define LOGCOUT(X)\
    {\
    std::string tmp;\
    tmp.append X.append("\n");\
    std::cout << tmp;\
    }

#define LOGEX(X,Y)\
    {\
    std::string tmp;\
    tmp.append X.append("\n");\
    Y.push(std::move(tmp));\
    }

#define LOG(X)\
    {\
    std::string tmp;\
    tmp.append X.append("\n");\
    m_out_log.push(std::move(tmp));\
    }


#endif