#ifndef __SYLAR_FIBER_H__
#define __SYLAR_FIBER_H__

#include <memory>
#include <functional>
#include <ucontext.h>

namespace cppserver {

class Scheduler;

class Fiber : public std::enable_shared_from_this<Fiber> {
friend class Scheduler;
public:
    typedef std::shared_ptr<Fiber> ptr;

    /**
     * @brief Fiber states
     */
    enum State {
        INIT,
        HOLD, // paused state
        EXEC, // executing state
        TERM, // termination state
        READY, // ready to be executed
        EXCEPT // exception
    };
private:
    /**
     * @brief No arg ctor
     * @attention The first fiber construct in the thread
     */
    Fiber();

public:
    /**
     * @brief Ctor
     * @param[in] cb Callback function to be execed by fiber
     * @param[in] stacksize Stack size of fiber
     * @param[in] use_caller Whether called on mainfiber
     */
    Fiber(std::function<void()> cb, size_t stacksize = 0, bool use_caller = false);
    ~Fiber();

    /**
     * @brief Reset execution function
     * @pre getState() is one of INIT, TERM, EXCEPT
     * @post getState() = INIT
     */
    void reset(std::function<void()> cb);

    /**
     * @brief Move fiber to execution state
     * @pre getState() != EXEC
     * @post getState() = EXEC
     */
    void swapIn();

    /**
     * @brief Move fiber to ready state
     */
    void swapOut();

    /**
     * @brief swap fiber to exec state
     * @pre caller is the main fiber in thread
     */
    void call();

    /**
     * @brief swap fiber to backend
     * @pre caller is current fiber
     * @post return to main fiber in thread
     */
    void back();


    uint64_t getId() const { return m_id;}
    State getState() const { return m_state;}

public:

    /**
     * @brief 设置当前线程的运行协程
     * @param[in] f 运行协程
     */
    static void SetThis(Fiber* f);

    /**
     * @brief return current fiber
     */
    static Fiber::ptr GetThis();

    /**
     * @brief switch fiber to backend and set to ready state
     * @post getState() = READY
     */
    static void YieldToReady();

    /**
     * @brief switch fiber to backend and set to hold state
     * @post getState() = HOLD
     */
    static void YieldToHold();

    /**
     * @brief return total number of fibers
     */
    static uint64_t TotalFibers();

    /**
     * @brief 协程执行函数
     * @post return to fiber main thread
     */
    static void MainFunc();

    /**
     * @brief exec function
     * @post return to scheduler
     */
    static void CallerMainFunc();

    /**
     * @brief return current fiber id
     */
    static uint64_t GetFiberId();
private:
    /// fiber id
    uint64_t m_id = 0;
    /// stack size of fiber
    uint32_t m_stacksize = 0;
    /// fiber state enum
    State m_state = INIT;
    /// fiber exec context
    ucontext_t m_ctx;
    /// fiber stack pointer
    void* m_stack = nullptr;
    /// function to be executed by fiber
    std::function<void()> m_cb;
};

}

#endif
