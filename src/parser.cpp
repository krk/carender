#include <ostream>
#include <memory>
#include <functional>
#include <algorithm>
#include <string>

#include "lexer.hpp"
#include "parser.hpp"

using Type = car::lexer::Token::Type;

namespace car
{
namespace parser
{

template <std::size_t S>
constexpr std::size_t string_length(
    char const (&)[S])
{
    return S - 1;
}

static bool parseExact(std::vector<lexer::Token>::const_iterator &begin, const std::vector<lexer::Token>::const_iterator &end,
                       std::ostream &error, const lexer::Token::Type type, const std::string &value = "")
{
    if (begin == end)
    {
        error << "Unexpected EOF after " << *(begin - 1) << std::endl;
        return false;
    }

    if (begin->GetType() == type && begin->GetValue() == value)
    {
        begin++;
        return true;
    }

    error << "Expected " << type;
    if (value.length() > 0)
    {
        error << " `" << value << "`";
    }
    error << " instead of " << *begin << std::endl;
    return false;
}

std::vector<std::string>
Parser::parseSymbols(std::vector<lexer::Token>::const_iterator &begin,
                     const std::vector<lexer::Token>::const_iterator end,
                     int count,
                     bool checkAllSymbols,
                     std::vector<std::string> &declared,
                     std::ostream &error)
{
    auto symbols = std::vector<std::string>();
    symbols.reserve(count);
    int seen = 0;
    for (auto &it = begin; it != end && seen < count; it++)
    {
        switch (it->GetType())
        {
        case Type::Symbol:
        {
            const auto &symbol = it->GetValue();
            // If checkAllSymbols is true, all symbols need to be declared.
            if (checkAllSymbols || seen == 0)
            {
                // Only first symbol is checked, subsequent symbols are interpreted as declarations.
                if (this->options.SymbolChecksEnabled() &&
                    this->options.Symbols().find(symbol) == this->options.Symbols().end())
                {
                    error << "Invalid symbol " << *it << std::endl;
                    goto fail;
                }
            }
            else
            {
                // If symbol is not defined, add it. It is an error to define same symbol more than once in a file.
                if (this->options.SymbolChecksEnabled() &&
                    this->options.Symbols().find(symbol) != this->options.Symbols().end())
                {
                    error << "Symbol already defined: " << *it << std::endl;
                    goto fail;
                }

                declared.push_back(symbol);
            }

            symbols.push_back(symbol);
            seen++;
            break;
        }
        default:
            error << "Expected symbol instead of " << *it << std::endl;
            goto fail;
        }
    }

    if (seen != count)
    {
        error << "Expected " << count << " symbols instead of " << seen << std::endl;
        goto fail;
    }

    // Next symbol must be an EndDirective.
    if (!parseExact(begin, end, error, Type::EndDirective))
    {
        goto fail;
    }

    return symbols;

fail:
    return {};
}

template <typename NodeType, bool checkAllSymbols, char... keywordChars>
std::vector<std::unique_ptr<Node>>
Parser::parseBlockWithTwoSymbols(std::vector<lexer::Token>::const_iterator &begin,
                                 const std::vector<lexer::Token>::const_iterator end,
                                 std::ostream &error)
{
    // {{#keyword symbol1 symbol2}} ... {{/keyword}}
    auto keyword = std::string({keywordChars...});
    std::vector<std::shared_ptr<Node>> children;
    auto const initial = begin;

    auto declared = std::vector<std::string>();
    auto symbols = this->parseSymbols(begin, end, 2, checkAllSymbols, declared, error);
    if (symbols.size() != 2)
    {
        goto fail;
    }
    if (!checkAllSymbols)
    {
        this->options.Symbols().insert(declared.begin(), declared.end());
    }

    // Parse children.
    for (auto &n : this->parseNodes(begin, end, error))
    {
        children.push_back(std::move(n));
    }

    if (children.size() == 0)
    {
        error << keyword << " node must have children." << std::endl;
        goto fail;
    }

    // Parse StartDirective EndBlock Keyword EndDirective.
    if (!parseExact(begin, end, error, Type::StartDirective))
    {
        goto fail;
    }

    if (!parseExact(begin, end, error, Type::EndBlock))
    {
        goto fail;
    }

    if (!parseExact(begin, end, error, Type::Keyword, keyword))
    {
        goto fail;
    }

    if (!parseExact(begin, end, error, Type::EndDirective))
    {
        goto fail;
    }

    // Return NodeType.
    {
        auto nodes = std::vector<std::unique_ptr<Node>>();

        nodes.push_back(std::make_unique<NodeType>(
            NodeType(
                symbols[0],
                symbols[1],
                Context(initial->GetContext().StartPos(), (begin - 1)->GetContext().EndPos()),
                children)));

        if (!checkAllSymbols)
        {
            for (const auto &symbol : declared)
            {
                this->options.Symbols().erase(symbol);
            }
        }

        return nodes;
    }

fail:
    if (!checkAllSymbols)
    {
        for (const auto &symbol : declared)
        {
            this->options.Symbols().erase(symbol);
        }
    }

    return {};
}

std::vector<std::unique_ptr<Node>>
Parser::parseIfEq(std::vector<lexer::Token>::const_iterator &begin,
                  const std::vector<lexer::Token>::const_iterator end,
                  std::ostream &error)
{
    // {{#ifeq symbol symbol}} ... {{/ifeq}}
    return parseBlockWithTwoSymbols<IfEqNode, true, 'i', 'f', 'e', 'q'>(begin, end, error);
}

std::vector<std::unique_ptr<Node>>
Parser::parseLoop(std::vector<lexer::Token>::const_iterator &begin,
                  const std::vector<lexer::Token>::const_iterator end,
                  std::ostream &error)
{
    // {{#loop range element}} ... {{/loop}}
    return parseBlockWithTwoSymbols<LoopNode, false, 'l', 'o', 'o', 'p'>(begin, end, error);
}

std::vector<std::unique_ptr<Node>>
Parser::parseBlock(std::vector<lexer::Token>::const_iterator &begin,
                   const std::vector<lexer::Token>::const_iterator end,
                   std::ostream &error)
{
    // Parse a single block including the corresponding EndBlock and EndDirective.

    auto &it = begin;

    // begin must be a Keyword.
    if (it->GetType() == Type::Keyword)
    {
        auto keyword = it->GetValue();
        auto pair = this->keywordParser.find(keyword);

        if (pair == this->keywordParser.end())
        {
            error << "Unsupported keyword `" << keyword << "` at " << it->GetContext() << std::endl;
            goto fail;
        }

        it++;
        auto parser = pair->second;

        return (this->*parser)(it, end, error); // LCOV_EXCL_LINE coverage not reported successfully on member function pointer.
    }

    error << "Expected Keyword instead of " << *it << std::endl;

fail:
    return {};
}

std::vector<std::unique_ptr<Node>>
Parser::parseNodes(std::vector<lexer::Token>::const_iterator &begin,
                   const std::vector<lexer::Token>::const_iterator end,
                   std::ostream &error)
{
    auto nodes = std::vector<std::unique_ptr<Node>>();

    for (auto &it = begin; it != end; it++)
    {
        switch (it->GetType())
        {
        case Type::StartDirective:
        {
            // PrintNode, LoopNode or other directive/block node.
            auto next = it + 1;
            if (next == end)
            {
                error << "Unexpected EOF after " << *it << std::endl;
                goto fail;
            }

            switch (next->GetType())
            {
            case Type::Symbol:
            {
                // Symbol after directive without a block start/end is a PrintNode.
                auto nextNext = next + 1;
                if (nextNext == end)
                {
                    error << "Unexpected EOF after " << *next << std::endl;
                    goto fail;
                }

                if (nextNext->GetType() != Type::EndDirective)
                {
                    error << "Expected EndDirective after " << *next << std::endl;
                    goto fail;
                }

                const auto &symbol = next->GetValue();
                if (this->options.SymbolChecksEnabled() &&
                    this->options.Symbols().find(symbol) == this->options.Symbols().end())
                {
                    error << "Invalid symbol " << *next << std::endl;
                    goto fail;
                }

                nodes.push_back(std::make_unique<PrintNode>(
                    PrintNode(symbol,
                              Context(it->GetContext().StartPos(), nextNext->GetContext().EndPos()))));

                // We have already consumed the next two tokens.
                it = nextNext;
                continue;
            }
            case Type::StartBlock:
            {
                auto nextNext = next + 1;
                auto blockNodes = this->parseBlock(nextNext, end, error);
                if (blockNodes.size() == 0)
                {
                    goto fail;
                }
                nodes.reserve(nodes.size() + blockNodes.size());
                nodes.insert(nodes.end(),
                             std::make_move_iterator(blockNodes.begin()),
                             std::make_move_iterator(blockNodes.end()));

                it = nextNext - 1;
                continue;
            }
            case Type::EndBlock:
            {
                // EndBlock cannot be parsed in this method, parseBlock will handle it.
                return nodes;
            }
            default:
            {
                error << "Text or StartDirective expected instead of " << *it << std::endl;
                goto fail;
            }
            }
            continue;
        }
        case Type::Text:
            // TextNode.
            nodes.push_back(std::make_unique<TextNode>(TextNode(it->GetValue(), it->GetContext())));
            continue;
        default:
        {
            // Return without error. If parsing is not complete, it will be handled by a caller.
            // If the caller is Parser::parse, it will return an error in case of incomplete parse.
            return nodes;
        }
        }
    }

    return nodes;

fail:
    return {};
}

bool Parser::parse(const std::vector<lexer::Token> &tokens,
                   std::vector<std::unique_ptr<Node>> &output,
                   std::ostream &error)
{
    auto begin = tokens.begin();
    auto end = tokens.end();
    auto nodes = parseNodes(begin, end, error);
    if (begin != end)
    {
        error << "Cannot parse at " << *begin << std::endl;
        return false;
    }

    output.insert(output.end(),
                  std::make_move_iterator(nodes.begin()),
                  std::make_move_iterator(nodes.end()));

    return true;
}

} // namespace parser
// LCOV_EXCL_START
} // namespace car
  // LCOV_EXCL_STOP