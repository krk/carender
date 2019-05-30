#include <ostream>
#include "context.hpp"

namespace car
{

std::ostream &operator<<(std::ostream &os, const Context &ctx)
{
    os << "[" << ctx.startPos << ", " << ctx.endPos << ")";
    return os;
}

// LCOV_EXCL_START
} // namespace car
  // LCOV_EXCL_STOP