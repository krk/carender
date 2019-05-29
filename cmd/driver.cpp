#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>

#include "lexer.hpp"
#include "parser.hpp"
#include "renderer.hpp"
#include "driver.hpp"

using car::lexer::Lexer;
using car::lexer::Token;
using car::parser::Parser;
using car::parser::ParserOptions;
using car::renderer::Renderer;

namespace car
{
namespace driver
{

bool Driver::Render(std::istream &input)
{
    auto symbolNames = std::unordered_set<std::string>();
    std::transform(this->symbols.begin(), this->symbols.end(),
                   std::inserter(symbolNames, symbolNames.end()),
                   [](auto pair) { return pair.first; });

    std::transform(this->rangeSymbols.begin(), this->rangeSymbols.end(),
                   std::inserter(symbolNames, symbolNames.end()),
                   [](auto pair) { return pair.first; });

    // Lex.
    auto lexer = Lexer();
    auto tokens = std::vector<Token>();

    if (!lexer.lex(input, tokens, this->error))
    {
        this->error << "Driver cannot lex." << std::endl;
        return false;
    }
    if (tokens.size() == 0)
    {
        // Empty input is not an error.
        return true;
    }

    // Parse.
    auto options = ParserOptions(symbolNames);
    auto parser = Parser(options);

    auto nodes = parser.parse(tokens, error);
    if (nodes.size() == 0)
    {
        // There are more than one Token, there should be Nodes.
        this->error << "Driver cannot parse." << std::endl;
        return false;
    }

    // Render.
    auto renderer = Renderer(symbols, rangeSymbols, this->output, this->error);
    for (auto const &n : nodes)
    {
        n->accept(renderer);
    }

    if (renderer.HasError())
    {
        this->error << "Driver cannot render." << std::endl;
        return false;
    }

    return true;
}

} // namespace driver
} // namespace car
