#ifndef _PARSER_HPP_INCLUDED
#define _PARSER_HPP_INCLUDED

#include <vector>
#include <unordered_map>
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

enum class VisitReason
{
    Enter,
    Exit,
};

class Visitor
{
public:
    virtual void visit(const TextNode &n) = 0;
    virtual void visit(const PrintNode &n) = 0;
    virtual void visit(const LoopNode &n, VisitReason reason) = 0;
};

class Node
{
public:
    virtual void accept(Visitor &v) = 0;

protected:
    Node(Context ctx) : ctx(ctx) {}

    Context ctx;
};

class PrintNode : public Node
{
public:
    PrintNode(std::string symbol, Context ctx) : Node(ctx), symbol(symbol) {}

    void accept(Visitor &v) override
    {
        v.visit(*this);
    }

    const std::string &Symbol() const { return this->symbol; }

private:
    std::string symbol;
};

class TextNode : public Node
{
public:
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
    LoopNode(std::string rangeSymbol, std::string elementSymbol, Context ctx)
        : Node(ctx), rangeSymbol(rangeSymbol), elementSymbol(elementSymbol), children(std::vector<std::unique_ptr<Node>>())
    {
    }

    LoopNode(std::string rangeSymbol, std::string elementSymbol, Context ctx, std::vector<std::unique_ptr<Node>> children)
        : Node(ctx), rangeSymbol(rangeSymbol), elementSymbol(elementSymbol), children(std::move(children)) {}

    void accept(Visitor &v) override
    {
        v.visit(*this, VisitReason::Enter);
        for (auto const &c : this->children)
        {
            c->accept(v);
        }
        v.visit(*this, VisitReason::Exit);
    }

    const std::string &RangeSymbol() const { return this->rangeSymbol; }
    const std::string &ElementSymbol() const { return this->elementSymbol; }
    const std::vector<std::unique_ptr<Node>> &Children() const { return this->children; }

private:
    const std::string rangeSymbol;
    const std::string elementSymbol;
    std::vector<std::unique_ptr<Node>> children;
};

class PrintingVisitor : public Visitor
{
public:
    PrintingVisitor(std::ostream &output) : output(output), loopDepth(0) {}

    void visit(const TextNode &n) override
    {
        output << "[TextNode `" << n.Text() << "`]";
    }

    void visit(const PrintNode &n) override
    {
        output << "[PrintNode symbol`" << n.Symbol() << "`]";
    }

    void visit(const LoopNode &n, VisitReason reason) override
    {
        switch (reason)
        {
        case VisitReason::Enter:
            loopDepth++;
            output << "[LoopNode `" << n.ElementSymbol() << "` in `" << n.RangeSymbol() << "` depth`" << loopDepth << "` {";

            for (auto const &child : n.Children())
            {
                child->accept(*this);
            }

            break;
        case VisitReason::Exit:
            output << "} depth`" << loopDepth << "`";
            loopDepth--;
            break;
        }
    }

private:
    std::ostream &output;
    int loopDepth;
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