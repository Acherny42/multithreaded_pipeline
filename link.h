
#include <iostream>
#include <queue>
#include <mutex>
#include <atomic>
#include <condition_variable>

#ifndef CAN_LINK_H
#define CAN_LINK_H

//one consumer many producers.
template <typename T>
class link {
public:

    explicit link(size_t maxSize) :
        m_maxSize(maxSize),
        m_detached(false) {}

    bool pop(T& e){
        std::unique_lock<std::mutex> lock(m_mutex);

        while(m_queue.empty()){

            if(m_detached)
                return false;

            m_cv.wait(lock);
        }

        e = std::move(m_queue.front());
        m_queue.pop();

        return true;
    }

    bool push(T&& e) {
        std::unique_lock<std::mutex> lock(m_mutex);
                
        if(m_queue.size() == m_maxSize || m_detached)
            return false;

        m_queue.push(std::move(e));
        lock.unlock();
        m_cv.notify_one();//only one consumer

        return true;
    }

    void detach() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_detached = true;
        lock.unlock();
        m_cv.notify_all();        
    }


private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    const size_t m_maxSize;
    std::condition_variable m_cv;
    bool m_detached;
};



#endif






