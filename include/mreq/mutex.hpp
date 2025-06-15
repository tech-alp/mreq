#pragma once

#ifdef MREQ_PLATFORM_BAREMETAL
    #include "mreq/platform/baremetal/mutex.hpp"
#elif defined(MREQ_PLATFORM_FREERTOS)
    #include "mreq/platform/freertos/mutex.hpp"
#elif defined(MREQ_PLATFORM_POSIX)
    #include "mreq/platform/posix/mutex.hpp"
#else
    #error "No platform selected! Define MREQ_PLATFORM_(BAREMETAL|FREERTOS|POSIX)"
#endif