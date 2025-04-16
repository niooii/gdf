#include <services/time.h>

namespace Services::Time {
    namespace detail {
        // updated in server/server and client/app respectively.
        f64 _internal_dt = 0;
    }
}
