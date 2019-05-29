#include "catch.hpp"
using Catch::Matchers::Equals;

#include <iostream>
#include <sstream>
#include "context.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "renderer.hpp"

using car::Context;
using car::lexer::Lexer;
using car::lexer::Token;
using car::parser::LoopNode;
using car::parser::Node;
using car::parser::Parser;
using car::parser::ParserOptions;
using car::parser::PrintNode;
using car::renderer::Renderer;

namespace car
{

#define ASSERT_RESULTS()                           \
    {                                              \
        auto errors = error.str();                 \
        if (expectedError.size() == 0)             \
        {                                          \
            REQUIRE(errors.size() == 0);           \
            auto rendered = dump.str();            \
            if (expectedDump.size() == 0)          \
            {                                      \
                REQUIRE(rendered.size() == 0);     \
            }                                      \
            else                                   \
            {                                      \
                REQUIRE(rendered == expectedDump); \
            }                                      \
        }                                          \
        else                                       \
        {                                          \
            REQUIRE(errors == expectedError);      \
        }                                          \
    }

std::vector<std::unique_ptr<Node>>
parseNodes(std::string text,
           const std::unordered_map<std::string, std::string> &symbols,
           const std::unordered_map<std::string, std::vector<std::string>> &rangeSymbols)
{
    auto symbolNames = std::unordered_set<std::string>();
    std::transform(symbols.begin(), symbols.end(),
                   std::inserter(symbolNames, symbolNames.end()),
                   [](auto pair) { return pair.first; });

    std::transform(rangeSymbols.begin(), rangeSymbols.end(),
                   std::inserter(symbolNames, symbolNames.end()),
                   [](auto pair) { return pair.first; });

    std::stringstream parserError;
    std::stringstream lexerError;

    auto tokens = std::vector<Token>();
    std::vector<std::unique_ptr<Node>> nodes;
    auto lexer = Lexer();
    auto parser = Parser(ParserOptions(symbolNames));

    std::stringstream input(text);
    lexer.lex(input, tokens, lexerError);
    auto lexerErrors = lexerError.str();
    REQUIRE(lexerErrors.size() == 0);

    nodes = parser.parse(tokens, parserError);
    auto parserErrors = parserError.str();
    REQUIRE(parserErrors.size() == 0);
    REQUIRE(nodes.size() > 0);

    return nodes;
}

TEST_CASE("Renderer", "[renderer]")
{
    std::stringstream dump;
    std::stringstream error;
    auto symbols = std::unordered_map<std::string, std::string>();
    auto rangeSymbols = std::unordered_map<std::string, std::vector<std::string>>();

    auto visitor = Renderer(symbols, rangeSymbols, dump, error);
    std::string expectedDump;
    std::string expectedError;

    SECTION("Loop Text")
    {
        rangeSymbols["xs"] = {"x1", "x2", "x3"};
        auto nodes = parseNodes("{{#loop xs x}}ABC{{/loop}}", symbols, rangeSymbols);
        auto renderer = Renderer(symbols, rangeSymbols, dump, error);
        for (auto const &n : nodes)
        {
            n->accept(renderer);
        }

        expectedError = "";
        expectedDump = "ABCABCABC";
        ASSERT_RESULTS()
    }

    SECTION("Loop Text Symbol")
    {
        rangeSymbols["xs"] = {"x1", "x2", "x3"};
        auto nodes = parseNodes("{{#loop xs x}}Let there be {{x}}!\n{{/loop}}", symbols, rangeSymbols);
        auto renderer = Renderer(symbols, rangeSymbols, dump, error);
        for (auto const &n : nodes)
        {
            n->accept(renderer);
        }

        expectedError = "";
        expectedDump = "Let there be x1!\nLet there be x2!\nLet there be x3!\n";
        ASSERT_RESULTS()
    }

    SECTION("Symbol redefined")
    {
        rangeSymbols["xs"] = {"x1", "x2", "x3"};
        symbols["x"] = "this is X.";
        std::vector<std::unique_ptr<Node>> nodes;
        nodes.push_back(std::make_unique<PrintNode>(PrintNode("x", Context(0, 1))));
        nodes.push_back(std::make_unique<LoopNode>(LoopNode("xs", "x", Context(10, 20))));

        auto renderer = Renderer(symbols, rangeSymbols, dump, error);
        for (auto const &n : nodes)
        {
            n->accept(renderer);
        }

        expectedError = "Symbol names must be unique across the program, redefined `x` at [10, 20)\n";
        ASSERT_RESULTS()
    }

    SECTION("Loop and Print")
    {
        rangeSymbols["xs"] = {"x1", "x2", "x3"};
        symbols["x"] = "this is X.";
        std::vector<std::unique_ptr<Node>> nodes;
        nodes.push_back(std::make_unique<LoopNode>(LoopNode("x", "x", Context(1, 10))));
        nodes.push_back(std::make_unique<PrintNode>(PrintNode("xs", Context(12, 19))));

        auto renderer = Renderer(symbols, rangeSymbols, dump, error);
        for (auto const &n : nodes)
        {
            n->accept(renderer);
        }

        // Renderer does not continue on error, second error
        // is not expected: "Symbol not found: `xs`".
        expectedError = "Range symbol not found: `x`\n";
        ASSERT_RESULTS()
    }

    SECTION("Renderers gonna rende")
    {
        rangeSymbols["bottles"] = {"99", "98", "97"};
        rangeSymbols["hello"] = {"hi", "hello", "nihao"};
        symbols["name"] = "Aragorn";

        auto nodes = parseNodes(R"({{#loop hello h}}{{h}} {{name}}!
{{#loop bottles count}}{{count}} bottles on the wall, take one out, pass it around,
{{/loop}}
{{/loop}}
)",
                                symbols, rangeSymbols);
        auto renderer = Renderer(symbols, rangeSymbols, dump, error);
        for (auto const &n : nodes)
        {
            n->accept(renderer);
        }

        expectedError = "";
        expectedDump = R"(hi Aragorn!
99 bottles on the wall, take one out, pass it around,
98 bottles on the wall, take one out, pass it around,
97 bottles on the wall, take one out, pass it around,
hello Aragorn!
99 bottles on the wall, take one out, pass it around,
98 bottles on the wall, take one out, pass it around,
97 bottles on the wall, take one out, pass it around,
nihao Aragorn!
99 bottles on the wall, take one out, pass it around,
98 bottles on the wall, take one out, pass it around,
97 bottles on the wall, take one out, pass it around,
)";
        ASSERT_RESULTS()
    }
}

} // namespace car