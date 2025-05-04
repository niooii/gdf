#pragma once
#include <chrono>
#include <gdfe/def.h>

namespace Services::Time {
    namespace detail {
        extern f64 _internal_dt;
    }

    FORCEINLINE f64 delta() { return detail::_internal_dt; };

    FORCEINLINE u64 unix_millis()
    {
        auto now = std::chrono::system_clock::now();

        std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

        return static_cast<u64>(now_time_t);
    }
} // namespace Services::Time
