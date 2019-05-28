#include <ostream>
#include <memory>
#include <functional>

#include "lexer.hpp"
#include "parser.hpp"

using Type = car::lexer::Token::Type;

namespace car
{
namespace parser
{

std::vector<std::unique_ptr<Node>>
Parser::parseLoop(std::vector<lexer::Token>::const_iterator &begin,
                  const std::vector<lexer::Token>::const_iterator end,
                  std::ostream &error) const
{
    // {{#loop range element}} ... {{/loop}}
    // Parse a loop block with its children and EndBlock.

    for (auto &it = begin; it != end; it++)
    {
        switch (it->GetType())
        {
        case Type::Keyword:
        {
            break;
        }
        default:
            break;
        }
    }
}

std::vector<std::unique_ptr<Node>>
Parser::parseBlock(std::vector<lexer::Token>::const_iterator &begin,
                   const std::vector<lexer::Token>::const_iterator end,
                   std::ostream &error) const
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
            error << "Syntax error: Unsupported keyword `" << keyword << "` at " << it->GetContext() << std::endl;
            goto fail;
        }

        it++;
        auto parser = pair->second;
        return (this->*parser)(it, end, error);
    }

    error << "Syntax error: Expected Keyword instead of " << *it << std::endl;

fail:
    return {};
}

std::vector<std::unique_ptr<Node>>
Parser::parseNodes(std::vector<lexer::Token>::const_iterator &begin,
                   const std::vector<lexer::Token>::const_iterator end,
                   std::ostream &error) const
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
                error << "Syntax error: Unexpected EOF after " << *it << std::endl;
                goto fail;
            }

            switch (next->GetType())
            {
            case Type::Symbol:
            {
                // Symbol after directive without a block start/end is a PrintNode.
                auto nextNext = next + 1;
                if (nextNext == end || nextNext->GetType() != Type::EndDirective)
                {
                    error << "Syntax error: Unexpected EOF after " << *next << std::endl;
                    goto fail;
                }

                nodes.push_back(std::make_unique<PrintNode>(
                    PrintNode(next->GetValue(),
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
            default:
            {
                error << "Syntax error: Text or StartDirective expected instead of " << *it << std::endl;
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
        error << "Syntax error: Cannot parse at " << *begin << std::endl;
        return {};
    }
    return nodes;
}

} // namespace parser
} // namespace car