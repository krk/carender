#ifndef _PARSER_HPP_INCLUDED
#define _PARSER_HPP_INCLUDED

#include <vector>
#include <unordered_map>

#include "context.hpp"
#include "lexer.hpp"

namespace car
{
namespace parser
{

class Node
{
};

class Parser
{
    std::vector<Node> parse(const std::vector<lexer::Token> &tokens,
                            const std::unordered_map<std::string, std::string> &variables);
};
} // namespace parser
} // namespace car

#endif _PARSER_HPP_INCLUDED