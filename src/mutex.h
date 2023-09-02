#ifndef __CPPSERVER_MUTEX_H__
#define __CPPSERVER_MUTEX_H__

#include <thread>
#include <functional>
#include <memory>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <atomic>
#include <list>

#include "noncopyable.h"
#include "fiber.h"

namespace cppserver {

/**
 * @brief 信号量
 */
class Semaphore : Noncopyable {
public:
    /**
     * @brief Semaphore Constructor
     * @param[in] count Initial value of semaphore, i.e. total number of resources
     */
    Semaphore(uint32_t count = 0);

    ~Semaphore();

    /**
     * @brief claim semaphore
     */
    void wait();

    /**
     * @brief release semaphore
     */
    void notify();
private:
    sem_t m_semaphore;
};

/**
 * @brief Template implementation of scoped lock, using mutex
 */
template<class T>
struct ScopedLockImpl {
public:
    /**
     * @brief Constructor
     * @param[in] mutex Mutex
     */
    ScopedLockImpl(T& mutex)
        :m_mutex(mutex) {
        m_mutex.lock();
        m_locked = true;
    }

    /**
     * @brief Release lock in destructor
     */
    ~ScopedLockImpl() {
        unlock();
    }

    void lock() {
        if(!m_locked) {
            m_mutex.lock();
            m_locked = true;
        }
    }

    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    /// mutex
    T& m_mutex;
    /// guard against multiple
    bool m_locked;
};

/**
 * @brief Read lock implementation
 */
template<class T>
struct ReadScopedLockImpl {
public:
    /**
     * @brief Constructor
     * @param[in] mutex read lock
     */
    ReadScopedLockImpl(T& mutex)
        :m_mutex(mutex) {
        m_mutex.rdlock();
        m_locked = true;
    }

    /**
     * @brief Release lock in destructor
     */
    ~ReadScopedLockImpl() {
        unlock();
    }

    /**
     * @brief lock in mutex
     */
    void lock() {
        if(!m_locked) {
            m_mutex.rdlock();
            m_locked = true;
        }
    }

    /**
     * @brief release lock
     */
    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    /// mutex
    T& m_mutex;
    // flag to control whether being locked
    bool m_locked;
};

/**
 * @brief Scoped write lock
 */
template<class T>
struct WriteScopedLockImpl {
public:
    /**
     * @brief Constructor
     * @param[in] mutex
     */
    WriteScopedLockImpl(T& mutex)
        :m_mutex(mutex) {
        m_mutex.wrlock();
        m_locked = true;
    }

    /**
     * @brief Destructor for unlock
     */
    ~WriteScopedLockImpl() {
        unlock();
    }

    /**
     * @brief Write lock
     */
    void lock() {
        if(!m_locked) {
            m_mutex.wrlock();
            m_locked = true;
        }
    }

    /**
     * @brief 解锁
     */
    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    /// Mutex
    T& m_mutex;
    bool m_locked;
};

/**
 * @brief Mutex
 */
class Mutex : Noncopyable {
public:
    /// 局部锁
    typedef ScopedLockImpl<Mutex> Lock;

    Mutex() {
        pthread_mutex_init(&m_mutex, nullptr);
    }
    ~Mutex() {
        pthread_mutex_destroy(&m_mutex);
    }
    void lock() {
        pthread_mutex_lock(&m_mutex);
    }
    void unlock() {
        pthread_mutex_unlock(&m_mutex);
    }
private:
    /// mutex
    pthread_mutex_t m_mutex;
};

/**
 * @brief For testing purpose only
 */
class NullMutex : Noncopyable{
public:
    typedef ScopedLockImpl<NullMutex> Lock;

    NullMutex() {}
    ~NullMutex() {}
    void lock() {}
    void unlock() {}
};

/**
 * @brief Read-write mutex
 */
class RWMutex : Noncopyable{
public:
    typedef ReadScopedLockImpl<RWMutex> ReadLock;
    typedef WriteScopedLockImpl<RWMutex> WriteLock;

    /**
     * @brief Constructor
     */
    RWMutex() {
        pthread_rwlock_init(&m_lock, nullptr);
    }

    /**
     * @brief 析构函数
     */
    ~RWMutex() {
        pthread_rwlock_destroy(&m_lock);
    }

    /**
     * @brief Lock read
     */
    void rdlock() {
        pthread_rwlock_rdlock(&m_lock);
    }

    /**
     * @brief Lock write
     */
    void wrlock() {
        pthread_rwlock_wrlock(&m_lock);
    }

    void unlock() {
        pthread_rwlock_unlock(&m_lock);
    }
private:
    pthread_rwlock_t m_lock;
};

/**
 * @brief For testing purpose only
 */
class NullRWMutex : Noncopyable {
public:
    typedef ReadScopedLockImpl<NullMutex> ReadLock;
    typedef WriteScopedLockImpl<NullMutex> WriteLock;

    NullRWMutex() {}
    ~NullRWMutex() {}

    void rdlock() {}
    void wrlock() {}
    void unlock() {}
};

/**
 * @briefSpinlock impl based on ScopedLockImpl
 */
class Spinlock : Noncopyable {
public:
    typedef ScopedLockImpl<Spinlock> Lock;

    /**
     * @brief Constructor
     */
    Spinlock() {
        pthread_spin_init(&m_mutex, 0);
    }

    /**
     * @brief Destructor
     */
    ~Spinlock() {
        pthread_spin_destroy(&m_mutex);
    }

    void lock() {
        pthread_spin_lock(&m_mutex);
    }

    void unlock() {
        pthread_spin_unlock(&m_mutex);
    }
private:
    pthread_spinlock_t m_mutex;
};

/**
 * @brief 原子锁
 */
class CASLock : Noncopyable {
public:
    /// 局部锁
    typedef ScopedLockImpl<CASLock> Lock;

    /**
     * @brief 构造函数
     */
    CASLock() {
        m_mutex.clear();
    }

    /**
     * @brief 析构函数
     */
    ~CASLock() {
    }

    void lock() {
        while(std::atomic_flag_test_and_set_explicit(&m_mutex, std::memory_order_acquire));
    }

    void unlock() {
        std::atomic_flag_clear_explicit(&m_mutex, std::memory_order_release);
    }
private:
    /// Atomic flag
    volatile std::atomic_flag m_mutex;
};

class Scheduler;
class FiberSemaphore : Noncopyable {
public:
    typedef Spinlock MutexType;

    FiberSemaphore(size_t initial_concurrency = 0);
    ~FiberSemaphore();

    bool tryWait();
    void wait();
    void notify();

    size_t getConcurrency() const { return m_concurrency;}
    void reset() { m_concurrency = 0;}
private:
    MutexType m_mutex;
    std::list<std::pair<Scheduler*, Fiber::ptr> > m_waiters;
    size_t m_concurrency;
};



}

#endif
