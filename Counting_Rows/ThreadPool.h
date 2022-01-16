#pragma once
#include <queue>
#include <future>
#include <condition_variable>
#include <vector>
#include <functional>



// Auxiliary class - thread-safe queue
template<class T>
class BlockedQueue {
public:
    void push(T& item) {
        std::lock_guard<std::mutex> l(m_locker);
        // normal thread-safe push
        m_task_queue.push(item);
        // we make an alert so that the thread that caused
        // pop wakes up and takes the item from the queue
        m_event_holder.notify_one();
    }
    // blocking method for getting an item from a queue
    void pop(T& item) {
        std::unique_lock<std::mutex> l(m_locker);
        if (m_task_queue.empty())
            //we are waiting for the push to be called
            m_event_holder.wait(l, [this] {return !m_task_queue.empty(); });
        item = m_task_queue.front();
        m_task_queue.pop();
    }
    // non-blocking method of getting an item from a queue
    // returns false if the queue is empty
    bool fast_pop(T& item) {
        std::unique_lock<std::mutex> l(m_locker);
        if (m_task_queue.empty())
            // just get out
            return false;
        // taking the element
        item = m_task_queue.front();
        m_task_queue.pop();
        return true;
    }
private:
    std::mutex m_locker;
    // task queue
   std::queue<T> m_task_queue;
    // notifier
    std::condition_variable m_event_holder;
};
//---------------------------------------------------------------------------------------------------------------------
void taskFunc(std::string file);
// convenient definition for code reduction
typedef std::function<void()> task_type;
// type pointer to a function that is a reference for task functions
typedef void (*FuncType) (std::string);
//------------------------------------------------------------------------------------------------------------------------
// thread pool
class ThreadPool {
public:

    ThreadPool();
    // launch
    void start();
    // stop
    void stop();
    // task forwarding
    void push_task(FuncType f, std::string arg1);
    // input function for the stream
    void threadFunc(int qindex);
private:
    // number of threads
    int m_thread_count;
    // threads
    std::vector<std::thread> m_threads;
    // task queues for threads
    std::vector<BlockedQueue<task_type>> m_thread_queues;
    // for an even distribution of tasks
    unsigned m_qindex;
};
//----------------------------------------------------------------------------------------------------------------
//an intermediary class that accepts requests and forms a task for execution
class RequestHandler {
public:
    RequestHandler();
    ~RequestHandler();
    // sending a request for execution
        void pushRequest(FuncType f, std::string arg1);
   private:
    // thread pool
    ThreadPool m_tpool;
};