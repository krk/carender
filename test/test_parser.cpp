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
    std::stringstream lexerError;
    std::stringstream dump;
    auto tokens = std::vector<Token>();
    std::vector<std::unique_ptr<Node>> nodes;
    auto lexer = Lexer();
    auto parser = Parser();
    auto visitor = PrintingVisitor(dump);
    std::string expectedDump;
    std::string expectedError;

    SECTION("No nodes")
    {
        std::stringstream input("{{x}");
        lexer.lex(input, tokens, lexerError);
        auto errors = lexerError.str();

        REQUIRE(errors == "Unexpected token '}' at [3, 4).\n");

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedDump = "";
        expectedError = "Syntax error: Unexpected EOF after [Symbol at [2, 3)] 'x'\nSyntax error: Cannot parse at [StartDirective at [0, 2)]\n";
    }

    SECTION("Only StartDirective")
    {
        tokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedDump = "";
        expectedError = "Syntax error: Unexpected EOF after [StartDirective at [0, 2)]\nSyntax error: Cannot parse at [StartDirective at [0, 2)]\n";
    }

    SECTION("StartDirective Symbol")
    {
        tokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newSymbol("x", Context(2, 3)),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedDump = "";
        expectedError = "Syntax error: Unexpected EOF after [Symbol at [2, 3)] 'x'\nSyntax error: Cannot parse at [StartDirective at [0, 2)]\n";
    }

    SECTION("StartDirective EndDirective")
    {
        tokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newEndDirective(Context(2, 4)),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedDump = "";
        expectedError = "Syntax error: Text or StartDirective expected instead of [StartDirective at [0, 2)]\nSyntax error: Cannot parse at [StartDirective at [0, 2)]\n";
    }

    SECTION("StartDirective StartBlock Text")
    {
        tokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newStartBlock(Context(2, 3)),
            TokenFactory::newText("abc", Context(3, 6)),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedDump = "";
        expectedError = "Syntax error: Expected Keyword instead of [Text at [3, 6)] 'abc'\nSyntax error: Cannot parse at [StartDirective at [0, 2)]\n";
    }

    SECTION("EndDirective")
    {
        tokens = {
            TokenFactory::newEndDirective(Context(0, 2)),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedDump = "";
        expectedError = "Syntax error: Cannot parse at [EndDirective at [0, 2)]\n";
    }

    SECTION("Text")
    {
        tokens = {
            TokenFactory::newText("dinle", Context(0, 5)),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 1);

        for (auto const &n : nodes)
        {
            n->accept(visitor);
        }

        expectedDump = "[TextNode `dinle`]";
    }

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

    SECTION("StartBlock Keyword")
    {
        std::stringstream input("{{#cmd}}");
        lexer.lex(input, tokens, error);
        auto errors = error.str();

        REQUIRE(errors.size() == 0);

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        for (auto const &n : nodes)
        {
            n->accept(visitor);
        }

        expectedDump = "";
        expectedError = "Syntax error: Unsupported keyword `cmd` at [3, 6)\nSyntax error: Cannot parse at [StartDirective at [0, 2)]\n";
    }

    auto errors = error.str();
    if (expectedError.size() == 0)
    {
        REQUIRE(errors.size() == 0);
    }
    else
    {
        REQUIRE(errors == expectedError);
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
} // namespace car

} // namespace car