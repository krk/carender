#ifndef _CARENDER_CONTEXT_HPP_INCLUDED
#define _CARENDER_CONTEXT_HPP_INCLUDED

#include <ostream>

namespace car
{
class Context
{
public:
    /**
    * Construct a Context object representing a range in the source code.
    */
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

    /**
    * Get the start position of the Context.
    */
    int StartPos() const { return this->startPos; }

    /**
    * Get the end position of the Context.
    */
    int EndPos() const { return this->endPos; }

private:
    int startPos;
    int endPos;
};

} // namespace car
#endif // _CARENDER_CONTEXT_HPP_INCLUDED