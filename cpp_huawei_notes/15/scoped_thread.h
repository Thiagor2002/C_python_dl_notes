#ifndef SCOPED_THREAD_H
#define SCOPED_THREAD_H

#include <thread>   // std::thread
#include <utility>  // std::forward/move

class scoped_thread {
public:
    template <typename... Arg>
    scoped_thread(Arg&&... arg) : thread_(std::forward<Arg>(arg)...)
    {
    }
    scoped_thread(scoped_thread&& other) = default;
    scoped_thread& operator=(scoped_thread&& other) = default;
    ~scoped_thread()
    {
        if (thread_.joinable()) {
            thread_.join();
        }
    }

private:
    std::thread thread_;
};

#endif // SCOPED_THREAD_H
