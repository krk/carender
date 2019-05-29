#include "renderer.hpp"

namespace car
{
namespace renderer
{

void Renderer::visit(const TextNode &n)
{
    if (this->hasError)
    {
        return;
    }

    this->output << n.Text();
}

void Renderer::visit(const PrintNode &n)
{
    if (this->hasError)
    {
        return;
    }

    const auto &symbol = this->symbols.find(n.Symbol());
    if (symbol == this->symbols.end())
    {
        this->hasError = true;
        this->error << "Symbol not found: `" << n.Symbol() << "`" << std::endl;
        return;
    }

    this->output << symbol->second;
}

void Renderer::visit(const LoopNode &n)
{
    if (this->hasError)
    {
        return;
    }

    auto rangeSym = this->rangeSymbols.find(n.RangeSymbol());
    if (rangeSym == this->rangeSymbols.end())
    {
        this->hasError = true;
        this->error << "Range symbol not found: `" << n.RangeSymbol() << "`" << std::endl;
        return;
    }

    const auto &range = rangeSym->second;
    const auto &element = n.ElementSymbol();

    // Symbol names must be unique across the program, i.e. every symbol is global-scoped.
    auto elemSym = this->symbols.find(element);
    if (elemSym != this->symbols.end())
    {
        this->hasError = true;
        this->error << "Symbol names must be unique across the program, redefined `" << element << "` at " << n.Ctx() << std::endl;
        return;
    }

    for (const auto &value : range)
    {
        this->symbols[element] = value;

        for (auto const &child : n.Children())
        {
            child->accept(*this);
        }
    }
    this->symbols.erase(element);
}

} // namespace renderer
} // namespace car
