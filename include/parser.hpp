#ifndef _PARSER_HPP_INCLUDED
#define _PARSER_HPP_INCLUDED

#include <vector>
#include <unordered_map>
#include <memory>

#include "context.hpp"
#include "lexer.hpp"

namespace car
{
namespace parser
{

class Node
{
protected:
    Node(Context ctx) : ctx(ctx) {}

    Context ctx;
};

class PrintNode : public Node
{
public:
    PrintNode(std::string symbol, Context ctx) : Node(ctx), symbol(symbol) {}

private:
    const std::string symbol;
};

class TextNode : public Node
{
public:
    TextNode(std::string text, Context ctx) : Node(ctx), text(text) {}

private:
    const std::string text;
};

class LoopNode : public Node
{
public:
    LoopNode(std::string rangeSymbol, std::string elementSymbol, Context ctx)
        : Node(ctx), rangeSymbol(rangeSymbol), elementSymbol(elementSymbol), children(std::vector<std::unique_ptr<Node>>())
    {
    }

    LoopNode(std::string rangeSymbol, std::string elementSymbol, Context ctx, std::vector<std::unique_ptr<Node>> children)
        : Node(ctx), rangeSymbol(rangeSymbol), elementSymbol(elementSymbol), children(std::move(children)) {}

private:
    const std::string rangeSymbol;
    const std::string elementSymbol;
    std::vector<std::unique_ptr<Node>> children;
};

class Parser
{
public:
    std::vector<std::unique_ptr<Node>> parse(const std::vector<lexer::Token> &tokens,
                                             std::ostream &error);

private:
    std::vector<std::unique_ptr<Node>>
    parseNodes(std::vector<lexer::Token>::const_iterator begin,
               std::vector<lexer::Token>::const_iterator end,
               std::ostream &error);

    std::vector<std::unique_ptr<Node>>
    parseBlock(std::vector<lexer::Token>::const_iterator begin,
               std::vector<lexer::Token>::const_iterator end,
               std::ostream &error);
};

} // namespace parser
} // namespace car

#endif // _PARSER_HPP_INCLUDED