#ifndef __CPPSERVER_NONCOPYABLE_H__
#define __CPPSERVER_NONCOPYABLE_H__

namespace cppserver
{

    /**
     * Interface for noncopyable, nonassignable objects
     * For use of =delete, see https://stackoverflow.com/questions/5513881/meaning-of-delete-after-function-declaration
     */
    class Noncopyable
    {
    public:
        Noncopyable() = default;
        ~Noncopyable() = default;
        // Delete copy constructor:
        // delete the copy constructor so you cannot copy-construct an object
        // of this class from a different object of this class
        Noncopyable(const Noncopyable &) = delete;
        // Delete assignment operator:
        // delete the `=` operator (`operator=()` class method) to disable copying
        // an object of this class
        Noncopyable &operator=(const Noncopyable &) = delete;
    };

}

#endif
