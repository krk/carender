#ifndef _CARENDER_PARSER_HPP_INCLUDED
#define _CARENDER_PARSER_HPP_INCLUDED

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
class IfEqNode;

class Visitor
{
public:
    virtual ~Visitor() = default;

    virtual void visit(const TextNode &n) = 0;
    virtual void visit(const PrintNode &n) = 0;
    virtual void visit(const LoopNode &n) = 0;
    virtual void visit(const IfEqNode &n) = 0;
};

class Node
{
public:
    virtual ~Node() = default;

    /**
    * Accept a visitor.
    */
    virtual void accept(Visitor &v) = 0;

    /**
    * Get context of the node.
    */
    const Context &Ctx() const { return this->ctx; }

protected:
    Node(Context ctx) : ctx(ctx) {}

    const Context ctx;
};

class PrintNode : public Node
{
public:
    virtual ~PrintNode() = default;

    /**
    * Constructs a PrintNode.
    */
    PrintNode(std::string symbol, Context ctx) : Node(ctx), symbol(symbol) {}

    void accept(Visitor &v) override
    {
        v.visit(*this);
    }

    /**
    * Get symbol name to be printed.
    */
    const std::string &Symbol() const { return this->symbol; }

private:
    const std::string symbol;
};

class TextNode : public Node
{
public:
    virtual ~TextNode() = default;

    /**
    * Constructs a TextNode.
    */
    TextNode(std::string text, Context ctx) : Node(ctx), text(text) {}

    void accept(Visitor &v) override
    {
        v.visit(*this);
    }

    /**
    * Get text of the node.
    */
    const std::string &Text() const { return this->text; }

private:
    const std::string text;
};

class LoopNode : public Node
{
public:
    virtual ~LoopNode() = default;

    /**
    * Constructs a LoopNode.
    */
    LoopNode(std::string rangeSymbol, std::string elementSymbol, Context ctx)
        : Node(ctx), rangeSymbol(rangeSymbol), elementSymbol(elementSymbol), children(std::vector<std::shared_ptr<Node>>())
    {
    }

    /**
    * Constructs a LoopNode.
    */
    LoopNode(std::string rangeSymbol, std::string elementSymbol, Context ctx, std::vector<std::shared_ptr<Node>> children)
        : Node(ctx), rangeSymbol(rangeSymbol), elementSymbol(elementSymbol), children(children)
    {
    }

    void accept(Visitor &v) override
    {
        v.visit(*this);
    }

    /**
    * Get the range symbol of the loop node.
    */
    const std::string &RangeSymbol() const
    {
        return this->rangeSymbol;
    }

    /**
    * Get the element symbol of the loop node.
    */
    const std::string &ElementSymbol() const
    {
        return this->elementSymbol;
    }

    /**
    * Get children of the loop node.
    */
    const std::vector<std::shared_ptr<Node>> &Children() const
    {
        return this->children;
    }

private:
    const std::string rangeSymbol;
    const std::string elementSymbol;
    const std::vector<std::shared_ptr<Node>> children;
};

class IfEqNode : public Node
{
public:
    virtual ~IfEqNode() = default;

    /**
    * Constructs an IfEqNode.
    */
    IfEqNode(std::string leftSymbol, std::string rightSymbol, Context ctx)
        : Node(ctx), leftSymbol(leftSymbol), rightSymbol(rightSymbol), children(std::vector<std::shared_ptr<Node>>())
    {
    }

    /**
    * Constructs a IfEqNode.
    */
    IfEqNode(std::string leftSymbol, std::string rightSymbol, Context ctx, std::vector<std::shared_ptr<Node>> children)
        : Node(ctx), leftSymbol(leftSymbol), rightSymbol(rightSymbol), children(children)
    {
    }

    void accept(Visitor &v) override
    {
        v.visit(*this);
    }

    /**
    * Get the left symbol of the loop node.
    */
    const std::string &LeftSymbol() const
    {
        return this->leftSymbol;
    }

    /**
    * Get the right symbol of the loop node.
    */
    const std::string &RightSymbol() const
    {
        return this->rightSymbol;
    }

    /**
    * Get children of the loop node.
    */
    const std::vector<std::shared_ptr<Node>> &Children() const
    {
        return this->children;
    }

private:
    const std::string leftSymbol;
    const std::string rightSymbol;
    const std::vector<std::shared_ptr<Node>> children;
};

class ParserOptions
{
public:
    /**
    * Constructs an instance of the parser options for the `car` template language.
    */
    ParserOptions(const std::unordered_set<std::string> &symbols)
        : symbols(symbols), symbolChecksEnabled(symbols.size() > 0)
    {
    }

    /**
    * Constructs an instance of the parser options for the `car` template language.
    */
    ParserOptions()
        : ParserOptions(std::unordered_set<std::string>())
    {
    }

    /**
    * Get defined symbol names.
    */
    std::unordered_set<std::string> &Symbols()
    {
        return this->symbols;
    }

    /**
    * Get SymbolChecksEnabled option.
    */
    bool SymbolChecksEnabled()
    {
        return this->symbolChecksEnabled;
    }

private:
    std::unordered_set<std::string> symbols;
    const bool symbolChecksEnabled;
};

class Parser
{
public:
    /**
    * Constructs an instance of the parser for the `car` template language.
    */
    Parser(ParserOptions options) : options(options) {}

    /**
    * Parses `car` template language tokens into parser nodes.
    */
    bool parse(const std::vector<lexer::Token> &tokens,
               std::vector<std::unique_ptr<Node>> &output,
               std::ostream &error);

private:
    typedef std::vector<std::unique_ptr<Node>> (Parser::*nodeParser)(std::vector<lexer::Token>::const_iterator &begin,
                                                                     const std::vector<lexer::Token>::const_iterator end,
                                                                     std::ostream &error);

    std::vector<std::string>
    parseSymbols(std::vector<lexer::Token>::const_iterator &begin,
                 const std::vector<lexer::Token>::const_iterator end,
                 int count,
                 bool checkAllSymbols,
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

    template <typename NodeType, bool checkAllSymbols, char... keywordChars>
    std::vector<std::unique_ptr<Node>>
    parseBlockWithTwoSymbols(std::vector<lexer::Token>::const_iterator &begin,
                             const std::vector<lexer::Token>::const_iterator end,
                             std::ostream &error);

    std::vector<std::unique_ptr<Node>> parseLoop(std::vector<lexer::Token>::const_iterator &begin,
                                                 const std::vector<lexer::Token>::const_iterator end,
                                                 std::ostream &error);

    std::vector<std::unique_ptr<Node>>
    parseIfEq(std::vector<lexer::Token>::const_iterator &begin,
              const std::vector<lexer::Token>::const_iterator end,
              std::ostream &error);

    std::unordered_map<std::string, nodeParser> keywordParser = {
        {"loop", &Parser::parseLoop},
        {"ifeq", &Parser::parseIfEq},
    };

    ParserOptions options;
};

} // namespace parser
} // namespace car

#endif // _CARENDER_PARSER_HPP_INCLUDED