#include "mreq/internal/NonCopyable.hpp"

namespace mreq {

class Mutex : private internal::NonCopyable {
public:
    Mutex() { handle = xSemaphoreCreateMutex(); }
    ~Mutex() { destroy(); }

    void lock()   { xSemaphoreTake(handle, portMAX_DELAY); }
    void unlock() { xSemaphoreGive(handle); }
    bool try_lock() { return xSemaphoreTake(handle, 0) == pdTRUE; }
    void destroy() {
        if (handle) {
            vSemaphoreDelete(handle);
            handle = nullptr;
        }
    }
private:
    SemaphoreHandle_t handle = nullptr;
};

} // namespace mreq 