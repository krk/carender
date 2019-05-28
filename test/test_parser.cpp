#include "catch.hpp"
using Catch::Matchers::Equals;

#include <iostream>
#include <sstream>
#include "context.hpp"
#include "lexer.hpp"
#include "parser.hpp"

using car::Context;
using car::lexer::Lexer;
using car::lexer::Token;
using car::lexer::TokenFactory;
using car::parser::Node;
using car::parser::Parser;
using car::parser::PrintingVisitor;

namespace car
{

TEST_CASE("Parser::parse", "[parser]")
{
    std::stringstream error;
    std::stringstream dump;
    auto tokens = std::vector<Token>();
    std::vector<std::unique_ptr<Node>> nodes;
    auto lexer = Lexer();
    auto parser = Parser();
    auto visitor = PrintingVisitor(dump);
    std::string expectedDump;

    SECTION("PrintNode")
    {
        std::stringstream input("{{x}}");
        lexer.lex(input, tokens, error);
        auto errors = error.str();

        REQUIRE(errors.size() == 0);

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 1);

        for (auto const &n : nodes)
        {
            n->accept(visitor);
        }

        expectedDump = "[PrintNode symbol`x`]";
    }

    auto nodesDump = dump.str();

    if (expectedDump.size() == 0)
    {
        REQUIRE(nodesDump.size() == 0);
    }
    else
    {
        REQUIRE(nodesDump == expectedDump);
    }
}

} // namespace car