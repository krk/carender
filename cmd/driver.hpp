#ifndef _DRIVER_HPP_INCLUDED
#define _DRIVER_HPP_INCLUDED

#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>

#include "lexer.hpp"
#include "parser.hpp"
#include "renderer.hpp"

using car::lexer::Lexer;
using car::lexer::Token;
using car::parser::Parser;
using car::parser::ParserOptions;
using car::renderer::Renderer;

namespace car
{
namespace driver
{

class Driver
{
public:
    Driver(
        const std::unordered_map<std::string, std::string> &symbols,
        const std::unordered_map<std::string, std::vector<std::string>> &rangeSymbols,
        std::ostream &output,
        std::ostream &error)
        : symbols(symbols), rangeSymbols(rangeSymbols), output(output), error(error) {}

    bool Render(std::istream &input);

private:
    std::unordered_map<std::string, std::string> symbols;
    const std::unordered_map<std::string, std::vector<std::string>> rangeSymbols;
    std::ostream &output;
    std::ostream &error;
};

} // namespace driver
} // namespace car

#endif // _DRIVER_HPP_INCLUDED