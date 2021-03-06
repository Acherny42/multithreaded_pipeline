#include <thread>
#include <string>
#include <fstream>
#include <iostream>
#include <limits.h>

#include "defines.h"
#include "link.h"
#include "pack.h"
#include "gen.h"
#include "det.h"
#include "log.h"

#include <chrono> 
#include <iostream> 
#include <string.h> 
#include <errno.h> 



uint GENERATION_DURATION_SEC    = 10;
uint UNIT_TEST_DURATION_SEC     = 5;
#define FINISH_DURATION_MS         300     //time for thread to empty the queue when producer stoped.


using namespace std; 
using namespace std::chrono; 

/*

    build:          make app
    clear:          make clean 
    normal:          ./app 0
    unit test 1:    ./app 1
    unit test 2:    ./app 2

*/
void normal_operation() {


    link<std::string> null(1);
    link<std::string> any_to_debug_log(50);
    auto debug_logger = std::make_unique<log>(any_to_debug_log, "debug_log_file.txt", null);



    link<pack> gen_to_det(10);
    link<std::string> get_to_log(10);



    auto logger = std::make_unique<log>(get_to_log, "log_file.txt", any_to_debug_log);
    auto detector = std::make_unique<det>(gen_to_det, get_to_log, any_to_debug_log);
    auto generator = std::make_unique<gen>(gen_to_det, any_to_debug_log);
    

    debug_logger->start();
    logger->start();
    detector->start();
    generator->start();

    std::this_thread::sleep_for(std::chrono::milliseconds(GENERATION_DURATION_SEC * 1000));    


    generator->stop();

    std::this_thread::sleep_for(std::chrono::milliseconds(FINISH_DURATION_MS));    
    detector->stop();

    std::this_thread::sleep_for(std::chrono::milliseconds(FINISH_DURATION_MS));    
    logger->stop();
    
    
    


    generator.reset();
    LOGEX(("----generator ended ----"),any_to_debug_log)
    detector.reset();
    LOGEX(("----detector ended ----"),any_to_debug_log)
    logger.reset();
    

    std::this_thread::sleep_for(std::chrono::microseconds(FINISH_DURATION_MS));
    any_to_debug_log.detach();
    debug_logger->stop();
    debug_logger.reset();



}


//test should be always INVALID.
class gen_unit_test_2 : public gen {

public:

    gen_unit_test_2(link<pack>& out, link<std::string>& log) : 
    gen(out,log), 
    m_counter(0){}

    void update_test() {
        std::unique_lock<std::mutex> lock(m_unit_test_mutex);
        m_counter++;
    }

    uint generate_id() override {
        return CAN_IDS[m_counter % NUM_IDS];
    }

    //disvalidate dlc
    uint generate_nbytes() override {
        return MAX_DATA_LEN_BYTES;
    }

    //disvalidate interval
    uint generate_interval() override {
        return ( MIN_VALID_TIME_INTERVAL_MS / NUM_IDS ) / 2;
    }

    void generate_data(pack& p, uint nbytes) override {
        LOG(("generate_data unittest 2"))
        gen::generate_data(p, nbytes);

        //overwrite with same value.
        if(nbytes) {
            if(!p.set_data_byte( 1, 0)){
                LOG(("Error set_data_bytes"))
            }
        }
    }

    void work() override {        
        gen::work();
        update_test();
    }

    bool validate_output(string s) {
        //just to check counter in thread safe way
        uint c = 0;
        {
            std::unique_lock<std::mutex> lock(m_unit_test_mutex);
            c = m_counter;
        }

        //no referance for first 3 packets.
        if(c > 2 && string::npos == s.find(",INVALID,RATE,LENGTH,DATA") ) 
        {
            return false;
        }
        return true;
    }

private:
    long unsigned long m_counter;

};


//test should be always VALID
//time intervals are valid bacause id is repeated every 3 packets.
class gen_unit_test_1 : public gen {

public:

    gen_unit_test_1(link<pack>& out, link<std::string>& log) : gen(out,log), m_counter(0){}

    void update_test() {
        m_counter++;
    }

    uint get_rand() override {
        return m_counter;
    }

    void work() override {
        update_test();
        gen::work();
    }

    bool validate_output(string s) {
        if(string::npos != s.find(",INVALID")) {
            return false;
        }
        return true;
    }

private:
    long unsigned long m_counter;
};

template <typename T>
void run_unit_test(std::string filename){

    link<std::string> null(1);
    link<std::string> any_to_debug_log(50);
    auto debug_logger = std::make_unique<log>(any_to_debug_log, filename, null);


    link<pack> gen_to_det(10);
    link<std::string> det_to_log_proxy(10);
    link<std::string> get_to_log(10);


    auto logger = std::make_unique<log>(get_to_log, "log_file.txt", any_to_debug_log);
    auto detector = std::make_unique<det>(gen_to_det, det_to_log_proxy, any_to_debug_log);
    auto generator = std::make_unique<T>(gen_to_det, any_to_debug_log);
    

    debug_logger->start();
    logger->start();
    detector->start();
    generator->start();

    auto start = high_resolution_clock::now(); 
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<chrono::milliseconds>(stop - start);
    do {

        std::string to_log;
        if(!det_to_log_proxy.pop(to_log)){
            LOGEX(("unit test 1 pop failed terminating "), any_to_debug_log)
            break;
        }

        if(false == generator->validate_output(to_log)) {
            LOGEX(("test failed"), any_to_debug_log)
            std::cout << "test failed !!! " << to_log <<std::endl;
            break;
        }

        if(!get_to_log.push(std::move(to_log))) {
            std::cout << "push logging failed " << std::endl;
        }

        stop = high_resolution_clock::now();
        duration = duration_cast<chrono::milliseconds>(stop - start);

    } while(duration.count() < UNIT_TEST_DURATION_SEC * 1000);
    get_to_log.detach();

    generator->stop();

    std::this_thread::sleep_for(std::chrono::milliseconds(FINISH_DURATION_MS));    
    detector->stop();

    std::this_thread::sleep_for(std::chrono::milliseconds(FINISH_DURATION_MS));    
    logger->stop();
    
    generator.reset();
    LOGEX(("----generator ended ----"),any_to_debug_log)
    detector.reset();
    LOGEX(("----detector ended ----"),any_to_debug_log)
    logger.reset();
    

    std::this_thread::sleep_for(std::chrono::microseconds(FINISH_DURATION_MS));
    any_to_debug_log.detach();
    debug_logger->stop();
    debug_logger.reset();

}

void print_usage() {
        std::cout << "usage: app [1|2|3] [time ms | bytes to compress]" << std::endl;
        std::cout << "1 - normal 10 " << std::endl;
        std::cout << "2 - unit test 1 10 " << std::endl;
        std::cout << "3 - unit test 2 10 " << std::endl;
        std::cout << "4 - unit test compress 500 " << std::endl;
}


void validate_compressed_bytes(const int array_compressed[], const int array_copy[], int size, bool print = false) {
    printf("compressed array:\n");
    uint invalid = 0;
    const uchar* p = (uchar*)array_compressed;
    for(uint i = 0 ; i < size ; i++ ) {
        uint tmp = pack::get_byte( (i + 1) * CHAR_BIT - 1, p, CHAR_BIT);
        if(print) {
            printf("%X ", tmp);
        }
        if(tmp != array_copy[i]) {
            invalid++;
        }
    }
    printf("\n");

    if(invalid) {
        printf("\nCOMPRESSION INVALID %d\n", invalid);
    } else {
        printf("\nCOMPRESSION CORRECT %d\n", invalid);
    }
}

#define PRINT_DATA
void compress_byte(int arr[], int size){
    printf("COMPRESSING\n");
    for(uint i = 0 ; i < size ; i ++) {
        uint tmp = arr[i];
        arr[i] = 0;
        const uchar* p =  (uchar*)&tmp;
        pack::set_bits(p, CHAR_BIT - 1,  (uchar*)arr, ((i + 1) * CHAR_BIT) - 1, CHAR_BIT);
        
        //for debug
#ifdef PRINT_DATA        
        uint out = pack::get_byte( ((i + 1) * CHAR_BIT) - 1, (uchar*)arr, CHAR_BIT);
        printf("%X ", out);
#endif        
    }

    printf("\n");
}

#define MIN_NUM 1 
#define MAX_NUM 100
#define PRINT true
int test_compression(uint NUM_ELEMS) {

    int* array = new int[NUM_ELEMS * 2];
    int* array_copy = new int[NUM_ELEMS * 2];


    srand (time(NULL));


    printf("original array:\n");
    for(int i = 0 ; i <  NUM_ELEMS * 2 ; i++) { 
        array[i] = MIN_NUM + rand() % MAX_NUM;
        array_copy[i] = array[i];
#ifdef PRINT_DATA        
        printf("%X ", array[i]);
#endif        
    }
    printf("\n");


    //unite. two threads. may be recursive.
    std::thread thread0(compress_byte, array, NUM_ELEMS);
    std::thread thread1(compress_byte, &(array[NUM_ELEMS]), NUM_ELEMS);
    thread0.join();    
    thread1.join();   

    // single thread
    // compress_byte(array, NUM_ELEMS);
    // compress_byte(&(array[NUM_ELEMS]), NUM_ELEMS);



    uint half_width = CHAR_BIT * NUM_ELEMS;
    pack::set_bits((uchar*)(&(array[NUM_ELEMS])),  half_width - 1, (uchar*)array, half_width * 2  - 1,  half_width);


    //array_copy[6] = 0;  //try disvalidate
    validate_compressed_bytes(array, array_copy, NUM_ELEMS * 2, PRINT);    

    delete [] array;
    delete [] array_copy;

    return 0;
}

int main(int argc, char** argv) {


    if(3 != argc) {
        print_usage();
        return 1;        
    }


    if((strcmp("0", argv[1]) &&
        strcmp("1", argv[1]) &&
        strcmp("2", argv[1]) &&
        strcmp("3", argv[1]) &&
        strcmp("4", argv[1])) ) 
    {
        print_usage();
        return 1;
    }

    unsigned long d = 0;
    if(ULONG_MAX == (d = strtoul (argv[2], NULL, 0)) || ERANGE == errno || 0 == d){
        print_usage();
        return 1;
    }

    

    GENERATION_DURATION_SEC = d;
    UNIT_TEST_DURATION_SEC = d;

    switch (atoi(argv[1]))
    {
        case 0:
            normal_operation();
        break;
        case 1:
            run_unit_test<gen_unit_test_1>("debug_log_file_unit_test_1.txt");
        break;
        case 2:
            run_unit_test<gen_unit_test_2>("debug_log_file_unit_test_2.txt");
        break;
        case 3:
            normal_operation();
            run_unit_test<gen_unit_test_2>("debug_log_file_unit_test_2.txt");
            run_unit_test<gen_unit_test_1>("debug_log_file_unit_test_1.txt");
        case 4:
            test_compression(d);
        break;

        default:
        cout << "input error" << endl;
    }
    return 0;
}