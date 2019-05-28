#include "renderer.hpp"

namespace car
{
namespace renderer
{

void Renderer::visit(const TextNode &n)
{
    this->output << n.Text();
}

void Renderer::visit(const PrintNode &n)
{
    const auto &symbol = this->symbols.find(n.Symbol());
    if (symbol == this->symbols.end())
    {
        this->error << "Symbol not found: `" << n.Symbol() << "`" << std::endl;
        return;
    }

    this->output << symbol->second;
}

void Renderer::visit(const LoopNode &n, VisitReason reason)
{
    switch (reason)
    {
    case VisitReason::Enter:
        loopDepth++;
        output << "[LoopNode `" << n.ElementSymbol() << "` in `" << n.RangeSymbol() << "` depth`" << loopDepth << "` {" << std::endl;
        break;
    case VisitReason::Exit:
        output << "} depth`" << loopDepth << "`" << std::endl;
        loopDepth--;
        break;
    }
}

} // namespace renderer
} // namespace car
