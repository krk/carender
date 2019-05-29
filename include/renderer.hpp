#ifndef _RENDERER_HPP_INCLUDED
#define _RENDERER_HPP_INCLUDED

#include <ostream>
#include <string>
#include <unordered_map>

#include "parser.hpp"

using car::parser::LoopNode;
using car::parser::PrintNode;
using car::parser::TextNode;
using car::parser::Visitor;

namespace car
{
namespace renderer
{

class Renderer : public Visitor
{
public:
    Renderer(
        const std::unordered_map<std::string, std::string> &symbols,
        const std::unordered_map<std::string, std::vector<std::string>> &rangeSymbols,
        std::ostream &output,
        std::ostream &error)
        : symbols(symbols), rangeSymbols(rangeSymbols), output(output), error(error) {}

    void visit(const TextNode &n) override;
    void visit(const PrintNode &n) override;
    void visit(const LoopNode &n) override;

    virtual ~Renderer() = default;

private:
    std::unordered_map<std::string, std::string> symbols;
    const std::unordered_map<std::string, std::vector<std::string>> rangeSymbols;
    std::ostream &output;
    std::ostream &error;
};

} // namespace renderer
} // namespace car

#endif // _RENDERER_HPP_INCLUDED