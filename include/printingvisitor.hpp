#ifndef _CARENDER_PRINTING_VISITOR_HPP_INCLUDED
#define _CARENDER_PRINTING_VISITOR_HPP_INCLUDED

#include <ostream>

#include "parser.hpp"

namespace car
{
namespace parser
{

class PrintingVisitor : public Visitor
{
public:
    /**
    * Constructs a visitor that prints nodes.
    */
    PrintingVisitor(std::ostream &output) : output(output), loopDepth(0) {}

    void visit(const TextNode &n) override;
    void visit(const PrintNode &n) override;
    void visit(const LoopNode &n) override;
    void visit(const IfEqNode &n) override;

    virtual ~PrintingVisitor() = default;

private:
    void indent();

    std::ostream &output;
    int loopDepth;
};

} // namespace parser
} // namespace car

#endif // _CARENDER_PRINTING_VISITOR_HPP_INCLUDED