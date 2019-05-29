#include "catch.hpp"
using Catch::Matchers::Equals;

#include <iostream>
#include <sstream>
#include "context.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "printingvisitor.hpp"

using car::Context;
using car::lexer::Lexer;
using car::lexer::Token;
using car::lexer::TokenFactory;
using car::parser::Node;
using car::parser::Parser;
using car::parser::ParserOptions;
using car::parser::PrintingVisitor;

namespace car
{

#define ASSERT_RESULTS()                        \
    {                                           \
        auto errors = error.str();              \
        if (expectedError.size() == 0)          \
        {                                       \
            REQUIRE(errors.size() == 0);        \
        }                                       \
        else                                    \
        {                                       \
            REQUIRE(errors == expectedError);   \
        }                                       \
        auto nodesDump = dump.str();            \
        if (expectedDump.size() == 0)           \
        {                                       \
            REQUIRE(nodesDump.size() == 0);     \
        }                                       \
        else                                    \
        {                                       \
            REQUIRE(nodesDump == expectedDump); \
        }                                       \
    }

TEST_CASE("Parser::parse no symbol check", "[parser]")
{
    std::stringstream error;
    std::stringstream lexerError;
    std::stringstream dump;
    auto tokens = std::vector<Token>();
    std::vector<std::unique_ptr<Node>> nodes;
    auto lexer = Lexer();
    auto symbols = std::unordered_set<std::string>();
    auto parser = Parser(ParserOptions(symbols));
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
        expectedError = "Unexpected EOF after [Symbol at [2, 3)] 'x'\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("Only StartDirective")
    {
        tokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedDump = "";
        expectedError = "Unexpected EOF after [StartDirective at [0, 2)]\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
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
        expectedError = "Unexpected EOF after [Symbol at [2, 3)] 'x'\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("StartDirective Symbol Symbol")
    {
        tokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newSymbol("x", Context(2, 3)),
            TokenFactory::newSymbol("y", Context(4, 5)),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedDump = "";
        expectedError = "Expected EndDirective after [Symbol at [2, 3)] 'x'\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
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
        expectedError = "Text or StartDirective expected instead of [StartDirective at [0, 2)]\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
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
        expectedError = "Expected Keyword instead of [Text at [3, 6)] 'abc'\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("EndDirective")
    {
        tokens = {
            TokenFactory::newEndDirective(Context(0, 2)),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedDump = "";
        expectedError = "Cannot parse at [EndDirective at [0, 2)]\n";
        ASSERT_RESULTS()
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

        expectedDump = "[TextNode `dinle`]\n";
        ASSERT_RESULTS()
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

        expectedDump = "[PrintNode symbol`x`]\n";
        ASSERT_RESULTS()
    }

    SECTION("StartBlock Keyword")
    {
        std::stringstream input("{{#cmd}}");
        lexer.lex(input, tokens, error);
        auto errors = error.str();

        REQUIRE(errors.size() == 0);

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedDump = "";
        expectedError = "Unsupported keyword `cmd` at [3, 6)\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("Loop no symbols")
    {
        std::stringstream input("{{#loop}}");
        lexer.lex(input, tokens, error);
        auto errors = error.str();

        REQUIRE(errors.size() == 0);

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedDump = "";
        expectedError = "Expected symbol instead of [EndDirective at [7, 9)]\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("Loop one symbol")
    {
        tokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newStartBlock(Context(2, 3)),
            TokenFactory::newKeyword("loop", Context(3, 7)),
            TokenFactory::newSymbol("abc", Context(7, 10)),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedError = "Expected 2 symbols instead of 1\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("Loop two symbols")
    {
        tokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newStartBlock(Context(2, 3)),
            TokenFactory::newKeyword("loop", Context(3, 7)),
            TokenFactory::newSymbol("abc", Context(7, 10)),
            TokenFactory::newSymbol("def", Context(11, 14)),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedError = "Unexpected EOF after [Symbol at [11, 14)] 'def'\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("Loop two symbols Text")
    {
        tokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newStartBlock(Context(2, 3)),
            TokenFactory::newKeyword("loop", Context(3, 7)),
            TokenFactory::newSymbol("abc", Context(7, 10)),
            TokenFactory::newSymbol("def", Context(11, 14)),
            TokenFactory::newText("zzz", Context(15, 18)),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedError = "Expected EndDirective instead of [Text at [15, 18)] 'zzz'\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("Loop two symbols EndDirective(invalid)")
    {
        tokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newStartBlock(Context(2, 3)),
            TokenFactory::newKeyword("loop", Context(3, 7)),
            TokenFactory::newSymbol("abc", Context(7, 10)),
            TokenFactory::newSymbol("def", Context(11, 14)),
            Token(Token::Type::EndDirective, Context(14, 16), "###"),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedError = "Expected EndDirective instead of [EndDirective at [14, 16)] '###'\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("Loop one symbol EndDirective")
    {
        std::stringstream input("{{#loop range1}}");
        lexer.lex(input, tokens, error);
        auto errors = error.str();

        REQUIRE(errors.size() == 0);

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedDump = "";
        expectedError = "Expected symbol instead of [EndDirective at [14, 16)]\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("Loop")
    {
        std::stringstream input("{{#loop range1 elem}}");
        lexer.lex(input, tokens, error);
        auto errors = error.str();

        REQUIRE(errors.size() == 0);

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedDump = "";
        expectedError = "loop node must have children.\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("Loop Text child, no end")
    {
        std::stringstream input("{{#loop range1 elem}}ABC");
        lexer.lex(input, tokens, error);
        auto errors = error.str();

        REQUIRE(errors.size() == 0);

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedDump = "";
        expectedError = "Unexpected EOF after [Text at [21, 24)] 'ABC'\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("Loop Text child, end")
    {
        std::stringstream input("{{#loop range1 elem}}ABC{{/loop}}");
        lexer.lex(input, tokens, error);
        auto errors = error.str();

        REQUIRE(errors.size() == 0);

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 1);

        for (auto const &n : nodes)
        {
            n->accept(visitor);
        }

        expectedDump = "[LoopNode `elem` in `range1` depth`1` {\n  [TextNode `ABC`]\n} depth`1`\n";
        ASSERT_RESULTS()
    }

    SECTION("Loop two symbols - invalid ending - 1")
    {
        tokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newStartBlock(Context(2, 3)),
            TokenFactory::newKeyword("loop", Context(3, 7)),
            TokenFactory::newSymbol("abc", Context(7, 10)),
            TokenFactory::newSymbol("def", Context(11, 14)),
            TokenFactory::newEndDirective(Context(14, 16)),
            TokenFactory::newText("audare", Context(16, 22)),
            TokenFactory::newSymbol("inv", Context(23, 26)),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedError = "Expected StartDirective instead of [Symbol at [23, 26)] 'inv'\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("Loop two symbols - invalid ending - 2")
    {
        tokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newStartBlock(Context(2, 3)),
            TokenFactory::newKeyword("loop", Context(3, 7)),
            TokenFactory::newSymbol("abc", Context(7, 10)),
            TokenFactory::newSymbol("def", Context(11, 14)),
            TokenFactory::newEndDirective(Context(14, 16)),
            TokenFactory::newText("audare", Context(16, 22)),
            TokenFactory::newStartDirective(Context(22, 24)),
            TokenFactory::newSymbol("inv", Context(24, 26)),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedError = "Unexpected EOF after [Symbol at [24, 26)] 'inv'\nloop node must have children.\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("Loop two symbols - invalid ending - 3")
    {
        tokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newStartBlock(Context(2, 3)),
            TokenFactory::newKeyword("loop", Context(3, 7)),
            TokenFactory::newSymbol("abc", Context(7, 10)),
            TokenFactory::newSymbol("def", Context(11, 14)),
            TokenFactory::newEndDirective(Context(14, 16)),
            TokenFactory::newText("audare", Context(16, 22)),
            TokenFactory::newStartDirective(Context(22, 24)),
            Token(Token::Type::EndBlock, Context(24, 28), "###"),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedError = "Expected EndBlock instead of [EndBlock at [24, 28)] '###'\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("Loop two symbols - invalid ending - 4")
    {
        tokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newStartBlock(Context(2, 3)),
            TokenFactory::newKeyword("loop", Context(3, 7)),
            TokenFactory::newSymbol("abc", Context(7, 10)),
            TokenFactory::newSymbol("def", Context(11, 14)),
            TokenFactory::newEndDirective(Context(14, 16)),
            TokenFactory::newText("audare", Context(16, 22)),
            TokenFactory::newStartDirective(Context(22, 24)),
            TokenFactory::newEndBlock(Context(24, 25)),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedError = "Unexpected EOF after [EndBlock at [24, 25)]\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("Loop two symbols - invalid ending - 5")
    {
        tokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newStartBlock(Context(2, 3)),
            TokenFactory::newKeyword("loop", Context(3, 7)),
            TokenFactory::newSymbol("abc", Context(7, 10)),
            TokenFactory::newSymbol("def", Context(11, 14)),
            TokenFactory::newEndDirective(Context(14, 16)),
            TokenFactory::newText("audare", Context(16, 22)),
            TokenFactory::newStartDirective(Context(22, 24)),
            TokenFactory::newEndBlock(Context(24, 25)),
            TokenFactory::newKeyword("nope", Context(25, 29)),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedError = "Expected Keyword `loop` instead of [Keyword at [25, 29)] 'nope'\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("Loop two symbols - invalid ending - 6")
    {
        tokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newStartBlock(Context(2, 3)),
            TokenFactory::newKeyword("loop", Context(3, 7)),
            TokenFactory::newSymbol("abc", Context(7, 10)),
            TokenFactory::newSymbol("def", Context(11, 14)),
            TokenFactory::newEndDirective(Context(14, 16)),
            TokenFactory::newText("audare", Context(16, 22)),
            TokenFactory::newStartDirective(Context(22, 24)),
            TokenFactory::newEndBlock(Context(24, 25)),
            TokenFactory::newKeyword("loop", Context(25, 29)),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedError = "Unexpected EOF after [Keyword at [25, 29)] 'loop'\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("Loop two symbols - invalid ending - 7")
    {
        tokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newStartBlock(Context(2, 3)),
            TokenFactory::newKeyword("loop", Context(3, 7)),
            TokenFactory::newSymbol("abc", Context(7, 10)),
            TokenFactory::newSymbol("def", Context(11, 14)),
            TokenFactory::newEndDirective(Context(14, 16)),
            TokenFactory::newText("audare", Context(16, 22)),
            TokenFactory::newStartDirective(Context(22, 24)),
            TokenFactory::newEndBlock(Context(24, 25)),
            TokenFactory::newKeyword("loop", Context(25, 29)),
            TokenFactory::newText("nope", Context(30, 34)),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedError = "Expected EndDirective instead of [Text at [30, 34)] 'nope'\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("IfEq one symbol")
    {
        std::stringstream input("{{#ifeq left}}");
        lexer.lex(input, tokens, error);
        auto errors = error.str();

        REQUIRE(errors.size() == 0);

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedDump = "";
        expectedError = "Expected symbol instead of [EndDirective at [12, 14)]\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("IfEq two symbols")
    {
        std::stringstream input("{{#ifeq left right}}");
        lexer.lex(input, tokens, error);
        auto errors = error.str();

        REQUIRE(errors.size() == 0);

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedDump = "";
        expectedError = "ifeq node must have children.\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("IfEq two symbols with child")
    {
        std::stringstream input("{{#ifeq left right}}ABC{{/ifeq}}");
        lexer.lex(input, tokens, error);
        auto errors = error.str();

        REQUIRE(errors.size() == 0);

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 1);

        for (auto const &n : nodes)
        {
            n->accept(visitor);
        }

        expectedDump = "[IfEqNode `left` `right`] {\n[TextNode `ABC`]\n}\n";
        expectedError = "";
        ASSERT_RESULTS()
    }

    SECTION("IfEq no symbols")
    {
        std::stringstream input("{{#ifeq}}");
        lexer.lex(input, tokens, error);
        auto errors = error.str();

        REQUIRE(errors.size() == 0);

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedDump = "";
        expectedError = "Expected symbol instead of [EndDirective at [7, 9)]\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("IfEq one symbol")
    {
        tokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newStartBlock(Context(2, 3)),
            TokenFactory::newKeyword("ifeq", Context(3, 7)),
            TokenFactory::newSymbol("abc", Context(7, 10)),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedError = "Expected 2 symbols instead of 1\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("IfEq two symbols")
    {
        tokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newStartBlock(Context(2, 3)),
            TokenFactory::newKeyword("ifeq", Context(3, 7)),
            TokenFactory::newSymbol("abc", Context(7, 10)),
            TokenFactory::newSymbol("def", Context(11, 14)),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedError = "Unexpected EOF after [Symbol at [11, 14)] 'def'\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("IfEq two symbols Text")
    {
        tokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newStartBlock(Context(2, 3)),
            TokenFactory::newKeyword("ifeq", Context(3, 7)),
            TokenFactory::newSymbol("abc", Context(7, 10)),
            TokenFactory::newSymbol("def", Context(11, 14)),
            TokenFactory::newText("zzz", Context(15, 18)),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedError = "Expected EndDirective instead of [Text at [15, 18)] 'zzz'\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("IfEq two symbols EndDirective(invalid)")
    {
        tokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newStartBlock(Context(2, 3)),
            TokenFactory::newKeyword("ifeq", Context(3, 7)),
            TokenFactory::newSymbol("abc", Context(7, 10)),
            TokenFactory::newSymbol("def", Context(11, 14)),
            Token(Token::Type::EndDirective, Context(14, 16), "###"),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedError = "Expected EndDirective instead of [EndDirective at [14, 16)] '###'\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("IfEq one symbol EndDirective")
    {
        std::stringstream input("{{#ifeq range1}}");
        lexer.lex(input, tokens, error);
        auto errors = error.str();

        REQUIRE(errors.size() == 0);

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedDump = "";
        expectedError = "Expected symbol instead of [EndDirective at [14, 16)]\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("IfEq two symbols - invalid ending - 1")
    {
        tokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newStartBlock(Context(2, 3)),
            TokenFactory::newKeyword("ifeq", Context(3, 7)),
            TokenFactory::newSymbol("abc", Context(7, 10)),
            TokenFactory::newSymbol("def", Context(11, 14)),
            TokenFactory::newEndDirective(Context(14, 16)),
            TokenFactory::newText("audare", Context(16, 22)),
            TokenFactory::newSymbol("inv", Context(23, 26)),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedError = "Expected StartDirective instead of [Symbol at [23, 26)] 'inv'\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("IfEq two symbols - invalid ending - 2")
    {
        tokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newStartBlock(Context(2, 3)),
            TokenFactory::newKeyword("ifeq", Context(3, 7)),
            TokenFactory::newSymbol("abc", Context(7, 10)),
            TokenFactory::newSymbol("def", Context(11, 14)),
            TokenFactory::newEndDirective(Context(14, 16)),
            TokenFactory::newText("audare", Context(16, 22)),
            TokenFactory::newStartDirective(Context(22, 24)),
            TokenFactory::newSymbol("inv", Context(24, 26)),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedError = "Unexpected EOF after [Symbol at [24, 26)] 'inv'\nifeq node must have children.\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("IfEq two symbols - invalid ending - 3")
    {
        tokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newStartBlock(Context(2, 3)),
            TokenFactory::newKeyword("ifeq", Context(3, 7)),
            TokenFactory::newSymbol("abc", Context(7, 10)),
            TokenFactory::newSymbol("def", Context(11, 14)),
            TokenFactory::newEndDirective(Context(14, 16)),
            TokenFactory::newText("audare", Context(16, 22)),
            TokenFactory::newStartDirective(Context(22, 24)),
            Token(Token::Type::EndBlock, Context(24, 28), "###"),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedError = "Expected EndBlock instead of [EndBlock at [24, 28)] '###'\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("IfEq two symbols - invalid ending - 4")
    {
        tokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newStartBlock(Context(2, 3)),
            TokenFactory::newKeyword("ifeq", Context(3, 7)),
            TokenFactory::newSymbol("abc", Context(7, 10)),
            TokenFactory::newSymbol("def", Context(11, 14)),
            TokenFactory::newEndDirective(Context(14, 16)),
            TokenFactory::newText("audare", Context(16, 22)),
            TokenFactory::newStartDirective(Context(22, 24)),
            TokenFactory::newEndBlock(Context(24, 25)),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedError = "Unexpected EOF after [EndBlock at [24, 25)]\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("IfEq two symbols - invalid ending - 5")
    {
        tokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newStartBlock(Context(2, 3)),
            TokenFactory::newKeyword("ifeq", Context(3, 7)),
            TokenFactory::newSymbol("abc", Context(7, 10)),
            TokenFactory::newSymbol("def", Context(11, 14)),
            TokenFactory::newEndDirective(Context(14, 16)),
            TokenFactory::newText("audare", Context(16, 22)),
            TokenFactory::newStartDirective(Context(22, 24)),
            TokenFactory::newEndBlock(Context(24, 25)),
            TokenFactory::newKeyword("nope", Context(25, 29)),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedError = "Expected Keyword `ifeq` instead of [Keyword at [25, 29)] 'nope'\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("IfEq two symbols - invalid ending - 6")
    {
        tokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newStartBlock(Context(2, 3)),
            TokenFactory::newKeyword("ifeq", Context(3, 7)),
            TokenFactory::newSymbol("abc", Context(7, 10)),
            TokenFactory::newSymbol("def", Context(11, 14)),
            TokenFactory::newEndDirective(Context(14, 16)),
            TokenFactory::newText("audare", Context(16, 22)),
            TokenFactory::newStartDirective(Context(22, 24)),
            TokenFactory::newEndBlock(Context(24, 25)),
            TokenFactory::newKeyword("ifeq", Context(25, 29)),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedError = "Unexpected EOF after [Keyword at [25, 29)] 'ifeq'\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("IEq two symbols - invalid ending - 7")
    {
        tokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newStartBlock(Context(2, 3)),
            TokenFactory::newKeyword("ifeq", Context(3, 7)),
            TokenFactory::newSymbol("abc", Context(7, 10)),
            TokenFactory::newSymbol("def", Context(11, 14)),
            TokenFactory::newEndDirective(Context(14, 16)),
            TokenFactory::newText("audare", Context(16, 22)),
            TokenFactory::newStartDirective(Context(22, 24)),
            TokenFactory::newEndBlock(Context(24, 25)),
            TokenFactory::newKeyword("ifeq", Context(25, 29)),
            TokenFactory::newText("nope", Context(30, 34)),
        };

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedError = "Expected EndDirective instead of [Text at [30, 34)] 'nope'\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }
}

TEST_CASE("Parser::parse symbol check", "[parser]")
{
    std::stringstream error;
    std::stringstream lexerError;
    std::stringstream dump;
    auto tokens = std::vector<Token>();
    std::vector<std::unique_ptr<Node>> nodes;
    auto lexer = Lexer();
    auto options = ParserOptions(std::unordered_set<std::string>({"validus"}));
    auto parser = Parser(options);
    auto visitor = PrintingVisitor(dump);
    std::string expectedDump;
    std::string expectedError;

    SECTION("Unknown symbol")
    {
        std::stringstream input("{{x}}");
        lexer.lex(input, tokens, lexerError);
        auto errors = lexerError.str();

        REQUIRE(errors.size() == 0);

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedDump = "";
        expectedError = "Invalid symbol [Symbol at [2, 3)] 'x'\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("Known symbol")
    {
        std::stringstream input("{{validus}}");
        lexer.lex(input, tokens, lexerError);
        auto errors = lexerError.str();

        REQUIRE(errors.size() == 0);

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 1);

        for (auto const &n : nodes)
        {
            n->accept(visitor);
        }

        expectedDump = "[PrintNode symbol`validus`]\n";
        ASSERT_RESULTS()
    }

    SECTION("Unknown symbol in loop")
    {
        std::stringstream input("{{#loop x x}}QED{{/loop}}");
        lexer.lex(input, tokens, lexerError);
        auto errors = lexerError.str();

        REQUIRE(errors.size() == 0);

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedError = "Invalid symbol [Symbol at [8, 9)] 'x'\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("Known symbol in loop - redefined")
    {
        std::stringstream input("{{#loop validus validus}}QED{{/loop}}");
        lexer.lex(input, tokens, lexerError);
        auto errors = lexerError.str();

        REQUIRE(errors.size() == 0);

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 0);

        expectedError = "Symbol already defined: [Symbol at [16, 23)] 'validus'\nCannot parse at [StartDirective at [0, 2)]\n";
        ASSERT_RESULTS()
    }

    SECTION("Known symbol in loop")
    {
        std::stringstream input("{{#loop validus i}}QED{{/loop}}");
        lexer.lex(input, tokens, lexerError);
        auto errors = lexerError.str();

        REQUIRE(errors.size() == 0);

        nodes = parser.parse(tokens, error);

        REQUIRE(nodes.size() == 1);

        for (auto const &n : nodes)
        {
            n->accept(visitor);
        }

        expectedDump = "[LoopNode `i` in `validus` depth`1` {\n  [TextNode `QED`]\n} depth`1`\n";

        ASSERT_RESULTS()
    }
}

} // namespace car