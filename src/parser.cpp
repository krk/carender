#include <ostream>
#include <memory>

#include "lexer.hpp"
#include "parser.hpp"

using Type = car::lexer::Token::Type;

namespace car
{
namespace parser
{

std::vector<std::unique_ptr<Node>>
Parser::parseBlock(std::vector<lexer::Token>::const_iterator begin,
                   std::vector<lexer::Token>::const_iterator end,
                   std::ostream &error)
{
    // begin is StartBlock or EndBlock.
    return {};
}

std::vector<std::unique_ptr<Node>>
Parser::parseNodes(std::vector<lexer::Token>::const_iterator begin,
                   std::vector<lexer::Token>::const_iterator end,
                   std::ostream &error)
{
    auto nodes = std::vector<std::unique_ptr<Node>>();

    for (auto it = begin; it != end; it++)
    {
        switch (it->GetType())
        {
        case Type::StartDirective:
        {
            // PrintNode, LoopNode or other directive/block node.
            auto next = it + 1;
            if (next == end)
            {
                error << "Syntax error: Unexpected EOF after " << *it;
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
                    error << "Syntax error: Unexpected EOF after " << *it;
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
                auto blockNodes = this->parseBlock(next, end, error);
                if (blockNodes.size() == 0)
                {
                    goto fail;
                }
                nodes.reserve(nodes.size() + blockNodes.size());
                // nodes.insert(nodes.end(), blockNodes.begin(), blockNodes.end());
                continue;
            }
            default:
            {
                error << "Syntax error: Text or StartDirective expected instead of " << *it;
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
            error << "Syntax error: Text or StartDirective expected instead of " << *it;
            goto fail;
        }
        }
    }

    return nodes;

fail:
    nodes.clear();
    return nodes;
}

std::vector<std::unique_ptr<Node>>
Parser::parse(const std::vector<lexer::Token> &tokens,
              std::ostream &error)
{
    return parseNodes(tokens.begin(), tokens.end(), error);
}

} // namespace parser
} // namespace car