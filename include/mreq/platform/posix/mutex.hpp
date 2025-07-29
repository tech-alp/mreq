#include "mreq/internal/NonCopyable.hpp"
#include <pthread.h>

namespace mreq {

class Mutex : private internal::NonCopyable {
public:
    Mutex()  { pthread_mutex_init(&mtx, nullptr); }
    ~Mutex() { destroy(); }

    void lock()   { pthread_mutex_lock(&mtx); }
    void unlock() { pthread_mutex_unlock(&mtx); }
    bool try_lock() { return pthread_mutex_trylock(&mtx) == 0; }
    void destroy()  { pthread_mutex_destroy(&mtx); }
private:
    pthread_mutex_t mtx;
};

} // namespace mreq 
