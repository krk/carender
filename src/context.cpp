#include <ostream>
#include "context.hpp"

namespace car
{

std::ostream &operator<<(std::ostream &os, const Context &ctx)
{
    os << "[" << ctx.startPos << ", " << ctx.endPos << ")";
    return os;
}

} // namespace car