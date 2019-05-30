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

void PrintingVisitor::visit(const LoopNode &n)
{
    this->indent();
    this->loopDepth++;
    this->output << "[LoopNode `" << n.ElementSymbol() << "` in `" << n.RangeSymbol() << "` depth`" << loopDepth << "` {" << std::endl;

    for (auto const &child : n.Children())
    {
        child->accept(*this);
    }

    this->output << "} depth`" << this->loopDepth << "`" << std::endl;
    this->loopDepth--;
}

void PrintingVisitor::visit(const IfEqNode &n)
{
    this->indent();
    this->output << "[IfEqNode `" << n.LeftSymbol() << "` `" << n.RightSymbol() << "`] {" << std::endl;

    for (auto const &child : n.Children())
    {
        child->accept(*this);
    }

    this->output << "}" << std::endl;
}

} // namespace parser
// LCOV_EXCL_START
} // namespace car
  // LCOV_EXCL_STOP