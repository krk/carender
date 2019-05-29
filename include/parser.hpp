#ifndef _PARSER_HPP_INCLUDED
#define _PARSER_HPP_INCLUDED

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <iostream>

#include "context.hpp"
#include "lexer.hpp"

namespace car
{
namespace parser
{

class TextNode;
class PrintNode;
class LoopNode;

class Visitor
{
public:
    virtual ~Visitor() = default;

    virtual void visit(const TextNode &n) = 0;
    virtual void visit(const PrintNode &n) = 0;
    virtual void visit(const LoopNode &n) = 0;
};

class Node
{
public:
    virtual ~Node() = default;

    virtual void accept(Visitor &v) = 0;

    const Context &Ctx() const { return this->ctx; }

protected:
    Node(Context ctx) : ctx(ctx) {}

    const Context ctx;
};

class PrintNode : public Node
{
public:
    virtual ~PrintNode() = default;

    PrintNode(std::string symbol, Context ctx) : Node(ctx), symbol(symbol) {}

    void accept(Visitor &v) override
    {
        v.visit(*this);
    }

    const std::string &Symbol() const { return this->symbol; }

private:
    const std::string symbol;
};

class TextNode : public Node
{
public:
    virtual ~TextNode() = default;

    TextNode(std::string text, Context ctx) : Node(ctx), text(text) {}

    void accept(Visitor &v) override
    {
        v.visit(*this);
    }

    const std::string &Text() const { return this->text; }

private:
    const std::string text;
};

class LoopNode : public Node
{
public:
    virtual ~LoopNode() = default;

    LoopNode(std::string rangeSymbol, std::string elementSymbol, Context ctx)
        : Node(ctx), rangeSymbol(rangeSymbol), elementSymbol(elementSymbol), children(std::vector<std::shared_ptr<Node>>())
    {
    }

    LoopNode(std::string rangeSymbol, std::string elementSymbol, Context ctx, std::vector<std::shared_ptr<Node>> children)
        : Node(ctx), rangeSymbol(rangeSymbol), elementSymbol(elementSymbol), children(children)
    {
    }

    void accept(Visitor &v) override
    {
        v.visit(*this);
    }

    const std::string &RangeSymbol() const
    {
        return this->rangeSymbol;
    }
    const std::string &ElementSymbol() const
    {
        return this->elementSymbol;
    }

    const std::vector<std::shared_ptr<Node>> &Children() const
    {
        return this->children;
    }

private:
    const std::string rangeSymbol;
    const std::string elementSymbol;
    const std::vector<std::shared_ptr<Node>> children;
};

class ParserOptions
{
public:
    ParserOptions(const std::unordered_set<std::string> &symbols)
        : symbols(symbols)
    {
    }

    std::unordered_set<std::string> &Symbols()
    {
        return this->symbols;
    }

private:
    std::unordered_set<std::string> symbols;
};

class Parser
{
public:
    Parser(ParserOptions options) : options(options) {}

    std::vector<std::unique_ptr<Node>> parse(const std::vector<lexer::Token> &tokens,
                                             std::ostream &error);

private:
    typedef std::vector<std::unique_ptr<Node>> (Parser::*nodeParser)(std::vector<lexer::Token>::const_iterator &begin,
                                                                     const std::vector<lexer::Token>::const_iterator end,
                                                                     std::ostream &error);

    std::vector<std::string>
    parseSymbols(std::vector<lexer::Token>::const_iterator &begin,
                 const std::vector<lexer::Token>::const_iterator end,
                 int count,
                 std::vector<std::string> &declared,
                 std::ostream &error);

    bool
    parseExact(std::vector<lexer::Token>::const_iterator &begin,
               const lexer::Token::Type type,
               const std::string &value = "") const;

    std::vector<std::unique_ptr<Node>> parseNodes(std::vector<lexer::Token>::const_iterator &begin,
                                                  const std::vector<lexer::Token>::const_iterator end,
                                                  std::ostream &error);

    std::vector<std::unique_ptr<Node>>
    parseBlock(std::vector<lexer::Token>::const_iterator &begin,
               const std::vector<lexer::Token>::const_iterator end,
               std::ostream &error);

    std::vector<std::unique_ptr<Node>>
    parseLoop(std::vector<lexer::Token>::const_iterator &begin,
              const std::vector<lexer::Token>::const_iterator end,
              std::ostream &error);

    std::unordered_map<std::string, nodeParser> keywordParser = {
        {"loop", &Parser::parseLoop},
    };

    ParserOptions options;
};

} // namespace parser
} // namespace car

#endif // _PARSER_HPP_INCLUDED