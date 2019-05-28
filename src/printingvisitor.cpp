#include "printingvisitor.hpp"

namespace car
{
namespace parser
{

void PrintingVisitor::indent()
{
    for (int i = 0; i < this->loopDepth; i++)
    {
        this->output << "  ";
    }
}

void PrintingVisitor::visit(const TextNode &n)
{
    this->indent();
    this->output << "[TextNode `" << n.Text() << "`]" << std::endl;
}

void PrintingVisitor::visit(const PrintNode &n)
{
    this->indent();
    this->output << "[PrintNode symbol`" << n.Symbol() << "`]" << std::endl;
}

void PrintingVisitor::visit(const LoopNode &n, VisitReason reason)
{
    switch (reason)
    {
    case VisitReason::Enter:
        this->indent();
        loopDepth++;
        this->output << "[LoopNode `" << n.ElementSymbol() << "` in `" << n.RangeSymbol() << "` depth`" << loopDepth << "` {" << std::endl;
        break;
    case VisitReason::Exit:
        this->output << "} depth`" << loopDepth << "`" << std::endl;
        loopDepth--;
        break;
    }
}

} // namespace parser
} // namespace car
