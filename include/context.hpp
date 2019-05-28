#ifndef _CONTEXT_HPP_INCLUDED
#define _CONTEXT_HPP_INCLUDED

#include <ostream>

namespace car
{
class Context
{
public:
    Context(int startPos, int endPos) : startPos(startPos), endPos(endPos) {}

    friend std::ostream &operator<<(std::ostream &os, const Context &ctx);

    Context &operator=(const Context &other)
    {
        this->startPos = other.startPos;
        this->endPos = other.endPos;

        return *this;
    }

    inline bool operator==(const Context &rhs) const
    {
        return (startPos == rhs.startPos) && (endPos == rhs.endPos);
    }

    inline bool operator!=(const Context &rhs) const
    {
        return !operator==(rhs);
    }

    int StartPos() const { return this->startPos; }
    int EndPos() const { return this->endPos; }

private:
    int startPos;
    int endPos;
};

} // namespace car
#endif