#ifndef _PRINTING_VISITOR_HPP_INCLUDED
#define _PRINTING_VISITOR_HPP_INCLUDED

#include <ostream>

#include "parser.hpp"

namespace car
{
namespace parser
{

class PrintingVisitor : public Visitor
{
public:
    PrintingVisitor(std::ostream &output) : output(output), loopDepth(0) {}

    virtual void indent();

    void visit(const TextNode &n) override;
    void visit(const PrintNode &n) override;
    void visit(const LoopNode &n) override;
    void visit(const IfEqNode &n) override;

    virtual ~PrintingVisitor() = default;

private:
    std::ostream &output;
    int loopDepth;
};

} // namespace parser
} // namespace car

#endif // _PRINTING_VISITOR_HPP_INCLUDED