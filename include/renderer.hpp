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
using car::parser::VisitReason;

namespace car
{
namespace renderer
{

class Renderer : public Visitor
{
public:
    Renderer(const std::unordered_map<std::string, std::string> &symbols, std::ostream &output, std::ostream &error)
        : symbols(symbols), output(output), error(error), loopDepth(0) {}

    void visit(const TextNode &n) override;
    void visit(const PrintNode &n) override;
    void visit(const LoopNode &n, VisitReason reason) override;

    virtual ~Renderer() = default;

private:
    const std::unordered_map<std::string, std::string> symbols;
    std::ostream &output;
    std::ostream &error;
    int loopDepth;
};

} // namespace renderer
} // namespace car

#endif // _RENDERER_HPP_INCLUDED