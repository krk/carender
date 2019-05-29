#include <ostream>
#include <memory>
#include <functional>
#include <algorithm>

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

// Merge these two when "if constexpr" is available.
// If we merge them now, test coverage will not be 100%.
#define PARSE_EXACT(type)                                              \
    if (begin == end)                                                  \
    {                                                                  \
        error << "Unexpected EOF after " << *(begin - 1) << std::endl; \
        goto fail;                                                     \
    }                                                                  \
    if (!this->parseExact(begin, (type), ""))                          \
    {                                                                  \
        error << "Expected " << type;                                  \
        error << " instead of " << *begin << std::endl;                \
        goto fail;                                                     \
    }

#define PARSE_EXACT_2(type, value)                                     \
    if (begin == end)                                                  \
    {                                                                  \
        error << "Unexpected EOF after " << *(begin - 1) << std::endl; \
        goto fail;                                                     \
    }                                                                  \
    if (!this->parseExact(begin, (type), (value)))                     \
    {                                                                  \
        error << "Expected " << type << " `" << value << "`";          \
        error << " instead of " << *begin << std::endl;                \
        goto fail;                                                     \
    }

#define CHECK_SYMBOL(symbol, it)                                               \
    if (!this->options.Symbols().empty() &&                                    \
        this->options.Symbols().find(symbol) == this->options.Symbols().end()) \
    {                                                                          \
        error << "Invalid symbol " << *it << std::endl;                        \
        goto fail;                                                             \
    }

#define CHECK_SYMBOL_UNDEFINED(symbol, it)                                     \
    if (!this->options.Symbols().empty() &&                                    \
        this->options.Symbols().find(symbol) != this->options.Symbols().end()) \
    {                                                                          \
        error << "Symbol already defined: " << *it << std::endl;               \
        goto fail;                                                             \
    }

bool Parser::parseExact(std::vector<lexer::Token>::const_iterator &begin,
                        const Type type,
                        const std::string &value) const
{
    if (begin->GetType() == type && begin->GetValue() == value)
    {
        begin++;
        return true;
    }

    return false;
}

std::vector<std::string>
Parser::parseSymbols(std::vector<lexer::Token>::const_iterator &begin,
                     const std::vector<lexer::Token>::const_iterator end,
                     int count,
                     std::vector<std::string> &declared,
                     std::ostream &error)
{
    auto symbols = std::vector<std::string>();
    int seen = 0;
    for (auto &it = begin; it != end && seen < count; it++)
    {
        switch (it->GetType())
        {
        case Type::Symbol:
        {
            const auto &symbol = it->GetValue();
            if (seen == 0)
            {
                // Only first symbol is checked, subsequent symbols are interpreted as declarations.
                CHECK_SYMBOL(symbol, it)
            }
            else
            {
                // If symbol is not defined, add it. It is an error to define same symbol more than once in a file.
                CHECK_SYMBOL_UNDEFINED(symbol, it);
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
    PARSE_EXACT(Type::EndDirective)

    return symbols;

fail:
    return {};
}

std::vector<std::unique_ptr<Node>>
Parser::parseLoop(std::vector<lexer::Token>::const_iterator &begin,
                  const std::vector<lexer::Token>::const_iterator end,
                  std::ostream &error)
{
    std::vector<std::shared_ptr<Node>> children;
    auto const initial = begin;

    // {{#loop range element}} ... {{/loop}}
    // Parse a single loop block with its children and EndBlock. `begin` will be at the first token after the Keyword.

    auto declared = std::vector<std::string>();
    auto symbols = this->parseSymbols(begin, end, 2, declared, error);
    if (symbols.size() != 2)
    {
        goto fail;
    }
    this->options.Symbols().insert(declared.begin(), declared.end());

    // Parse children.
    for (auto &n : this->parseNodes(begin, end, error))
    {
        children.push_back(std::move(n));
    }

    if (children.size() == 0)
    {
        error << "LoopNode must have children." << std::endl;
        goto fail;
    }

    // Parse StartDirective EndBlock Keyword EndDirective.
    PARSE_EXACT(Type::StartDirective)
    PARSE_EXACT(Type::EndBlock)
    PARSE_EXACT_2(Type::Keyword, "loop")
    PARSE_EXACT(Type::EndDirective)

    // Return LoopNode.
    {
        auto nodes = std::vector<std::unique_ptr<Node>>();

        nodes.push_back(std::make_unique<LoopNode>(
            LoopNode(
                symbols[0],
                symbols[1],
                Context(initial->GetContext().StartPos(), (begin - 1)->GetContext().EndPos()),
                children)));

        return nodes;
    }

fail:
    for (const auto &symbol : declared)
    {
        this->options.Symbols().erase(symbol);
    }

    return {};
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
        return (this->*parser)(it, end, error);
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
                CHECK_SYMBOL(symbol, next)

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

std::vector<std::unique_ptr<Node>>
Parser::parse(const std::vector<lexer::Token> &tokens,
              std::ostream &error)
{
    auto begin = tokens.begin();
    auto end = tokens.end();
    auto nodes = parseNodes(begin, end, error);
    if (begin != end)
    {
        error << "Cannot parse at " << *begin << std::endl;
        return {};
    }
    return nodes;
}

} // namespace parser
} // namespace car