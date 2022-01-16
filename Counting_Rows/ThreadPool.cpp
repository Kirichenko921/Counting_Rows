#include "ThreadPool.h"

ThreadPool::ThreadPool() :
    m_thread_count(std::thread::hardware_concurrency() != 0 ? std::thread::hardware_concurrency() : 4), // the value of the number of threads is taken by the number of processor cores
    m_thread_queues(m_thread_count) {

}
void ThreadPool::start() {
    for (int i = 0; i < m_thread_count; i++) {
        m_threads.emplace_back(&ThreadPool::threadFunc, this, i);
    }
}

void ThreadPool::stop() {
    for (int i = 0; i < m_thread_count; i++) {
        // we put a dummy task in each queue
        // to end the flow
        task_type empty_task;
        m_thread_queues[i].push(empty_task);
    }
    for (auto& t : m_threads) {
        t.join();
    }
}
void ThreadPool::push_task(FuncType f, std::string arg1) {
    // we calculate the index of the queue, where we put the task
    int queue_to_push = m_qindex++ % m_thread_count;
    // forming a functor
    task_type new_task = ([=] { f(arg1); });
    //we put it in the queue
    m_thread_queues[queue_to_push].push(new_task);
}

void ThreadPool::threadFunc(int qindex) {
    while (true) {
        // processing the next task
        task_type task_to_do;
        bool res;
        int i = 0;
        for (; i < m_thread_count; i++) {
            // an attempt to quickly pick up a task from any queue, starting with your own
            if (res = m_thread_queues[(qindex + i) % m_thread_count].fast_pop(task_to_do))
                break;
        }

        if (!res) {
            // calling the blocking queue receipt
            m_thread_queues[qindex].pop(task_to_do);
        }
        else if (!task_to_do) {
            //to prevent the thread from hanging, 
            //  we put the dummy task back
            m_thread_queues[(qindex + i) % m_thread_count].push(task_to_do);
        }
        if (!task_to_do) {
            return;
        }
        // completing the task
        task_to_do();
    }
}

RequestHandler::RequestHandler() {
    m_tpool.start();
}
RequestHandler::~RequestHandler() {
    m_tpool.stop();
}

void RequestHandler::pushRequest(FuncType f, std::string arg1) {
    m_tpool.push_task(f, arg1);
}