#include <services/events.h>

namespace Services::Events::detail {
    GDF_Semaphore flush_signal = GDF_CreateSemaphore(NULL);
}