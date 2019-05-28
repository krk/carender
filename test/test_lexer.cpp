#include "catch.hpp"
using Catch::Matchers::Equals;

#include <iostream>
#include <sstream>
#include "context.hpp"
#include "lexer.hpp"

using car::Context;
using car::lexer::Lexer;
using car::lexer::Token;
using car::lexer::TokenFactory;

namespace car
{

TEST_CASE("Lexer::Token", "[lexer]")
{
    std::stringstream output;
    std::string expected;

#define ASSERT_RESULTS()            \
    auto serialized = output.str(); \
    REQUIRE(serialized == expected);

    SECTION("StartDirective")
    {
        output << TokenFactory::newStartDirective(Context(0, 1000));
        expected = "[StartDirective at [0, 1000)]";

        ASSERT_RESULTS()
    }

    SECTION("EndDirective")
    {
        output << TokenFactory::newEndDirective(Context(0, 2));
        expected = "[EndDirective at [0, 2)]";

        ASSERT_RESULTS()
    }

    SECTION("StartBlock")
    {
        output << TokenFactory::newStartBlock(Context(0, 1));
        expected = "[StartBlock at [0, 1)]";

        ASSERT_RESULTS()
    }

    SECTION("EndBlock")
    {
        output << TokenFactory::newEndBlock(Context(0, 1));
        expected = "[EndBlock at [0, 1)]";

        ASSERT_RESULTS()
    }

    SECTION("Text")
    {
        output << TokenFactory::newText("p", Context(0, 1));
        expected = "[Text at [0, 1)] 'p'";

        ASSERT_RESULTS()
    }

    SECTION("Keyword")
    {
        output << TokenFactory::newKeyword("p", Context(0, 1));
        expected = "[Keyword at [0, 1)] 'p'";

        ASSERT_RESULTS()
    }

    SECTION("Symbol")
    {
        output << TokenFactory::newSymbol("p", Context(0, 1));
        expected = "[Symbol at [0, 1)] 'p'";

        ASSERT_RESULTS()
    }

    SECTION("unsupported")
    {
        output << Token((Token::Type)9999, Context(0, 1));
        expected = "[#unsupported token type# at [0, 1)]";

        ASSERT_RESULTS()
    }

#undef ASSERT_RESULTS
}

TEST_CASE("Lexer::lex", "[lexer]")
{
    std::stringstream error;
    auto lexer = Lexer();
    auto tokens = std::vector<Token>();
    std::string expectedErrors;

#define ASSERT_RESULTS()                   \
    auto errors = error.str();             \
    if (expectedErrors.size() == 0)        \
    {                                      \
        REQUIRE(errors.size() == 0);       \
    }                                      \
    else                                   \
    {                                      \
        REQUIRE(errors == expectedErrors); \
    }

    SECTION("Text single char")
    {
        std::stringstream input("p");
        lexer.lex(input, tokens, error);

        std::vector<Token> expectedTokens = {
            TokenFactory::newText("p", Context(0, 1)),
        };
        REQUIRE_THAT(tokens, Equals(expectedTokens));

        ASSERT_RESULTS()
    }

    SECTION("Text brace")
    {
        std::stringstream input("{q");
        lexer.lex(input, tokens, error);

        std::vector<Token> expectedTokens = {
            TokenFactory::newText("{q", Context(0, 2)),
        };
        REQUIRE_THAT(tokens, Equals(expectedTokens));

        ASSERT_RESULTS()
    }

    SECTION("Text")
    {
        std::stringstream input("lorem ipsum ");
        lexer.lex(input, tokens, error);

        std::vector<Token> expectedTokens = {
            TokenFactory::newText("lorem ipsum ", Context(0, 12)),
        };
        REQUIRE_THAT(tokens, Equals(expectedTokens));

        ASSERT_RESULTS()
    }

    SECTION("StartDirective")
    {
        std::stringstream input("{{}");
        lexer.lex(input, tokens, error);

        expectedErrors = "Unexpected token '}' at [2, 3).\n";
        std::vector<Token> expectedTokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
        };
        REQUIRE_THAT(tokens, Equals(expectedTokens));

        ASSERT_RESULTS()
    }

    SECTION("StartDirective StartBlock")
    {
        std::stringstream input("{{#");
        lexer.lex(input, tokens, error);

        expectedErrors = "Expected keyword at [2, 3).\n";
        std::vector<Token> expectedTokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newStartBlock(Context(2, 3)),
        };
        REQUIRE_THAT(tokens, Equals(expectedTokens));

        ASSERT_RESULTS()
    }

    SECTION("StartDirective StartBlock EndDirective")
    {
        std::stringstream input("{{#mop}}");
        lexer.lex(input, tokens, error);

        std::vector<Token> expectedTokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newStartBlock(Context(2, 3)),
            TokenFactory::newKeyword("mop", Context(3, 6)),
            TokenFactory::newEndDirective(Context(6, 8)),
        };
        REQUIRE_THAT(tokens, Equals(expectedTokens));

        ASSERT_RESULTS()
    }

    SECTION("StartDirective EndBlock EndDirective")
    {
        std::stringstream input("{{/mop}}");
        lexer.lex(input, tokens, error);

        std::vector<Token> expectedTokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newEndBlock(Context(2, 3)),
            TokenFactory::newKeyword("mop", Context(3, 6)),
            TokenFactory::newEndDirective(Context(6, 8)),
        };
        REQUIRE_THAT(tokens, Equals(expectedTokens));

        ASSERT_RESULTS()
    }

    SECTION("StartDirective Symbol EndDirective")
    {
        std::stringstream input("{{x}}");
        lexer.lex(input, tokens, error);

        std::vector<Token> expectedTokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newSymbol("x", Context(2, 3)),
            TokenFactory::newEndDirective(Context(3, 5)),
        };
        REQUIRE_THAT(tokens, Equals(expectedTokens));

        ASSERT_RESULTS()
    }

    SECTION("StartDirective Symbol")
    {
        std::stringstream input("{{x");
        lexer.lex(input, tokens, error);

        expectedErrors = "Directive not closed.\n";
        std::vector<Token> expectedTokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newSymbol("x", Context(-2, -1)),
        };
        REQUIRE_THAT(tokens, Equals(expectedTokens));

        ASSERT_RESULTS()
    }

    SECTION("StartDirective StartBlock Symbol EndDirective")
    {
        std::stringstream input("{{#mop floors}}");
        lexer.lex(input, tokens, error);

        std::vector<Token> expectedTokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newStartBlock(Context(2, 3)),
            TokenFactory::newKeyword("mop", Context(3, 6)),
            TokenFactory::newSymbol("floors", Context(7, 13)),
            TokenFactory::newEndDirective(Context(13, 15)),
        };
        REQUIRE_THAT(tokens, Equals(expectedTokens));

        ASSERT_RESULTS()
    }

    SECTION("StartDirective StartBlock Keyword")
    {
        std::stringstream input("{{#mop");
        lexer.lex(input, tokens, error);

        expectedErrors = "Block not closed.\n";
        std::vector<Token> expectedTokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newStartBlock(Context(2, 3)),
            TokenFactory::newKeyword("mop", Context(3, -1)),
        };
        REQUIRE_THAT(tokens, Equals(expectedTokens));

        ASSERT_RESULTS()
    }

    SECTION("StartDirective StartBlock Keyword error")
    {
        std::stringstream input("{{#mop}x");
        lexer.lex(input, tokens, error);

        expectedErrors = "Block not closed.\n";
        std::vector<Token> expectedTokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newStartBlock(Context(2, 3)),
            TokenFactory::newKeyword("mop", Context(3, 6)),
            TokenFactory::newText("}x", Context(6, 8)),
        };
        REQUIRE_THAT(tokens, Equals(expectedTokens));

        ASSERT_RESULTS()
    }

    SECTION("StartDirective StartBlock Keyword Symbol")
    {
        std::stringstream input("{{#mop #nop}}");
        lexer.lex(input, tokens, error);

        std::vector<Token> expectedTokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newStartBlock(Context(2, 3)),
            TokenFactory::newKeyword("mop", Context(3, 6)),
            // Symbols can start with '#', although that is not recommended.
            TokenFactory::newSymbol("#nop", Context(7, 11)),
            TokenFactory::newEndDirective(Context(11, 13)),
        };
        REQUIRE_THAT(tokens, Equals(expectedTokens));

        ASSERT_RESULTS()
    }

    SECTION("StartDirective StartBlock Keyword space")
    {
        std::stringstream input("{{#mop   a  b \tcd }}");
        lexer.lex(input, tokens, error);

        std::vector<Token> expectedTokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
            TokenFactory::newStartBlock(Context(2, 3)),
            TokenFactory::newKeyword("mop", Context(3, 6)),
            TokenFactory::newSymbol("a", Context(9, 10)),
            TokenFactory::newSymbol("b", Context(12, 13)),
            TokenFactory::newSymbol("cd", Context(15, 17)),
            TokenFactory::newEndDirective(Context(18, 20)),

        };
        REQUIRE_THAT(tokens, Equals(expectedTokens));

        ASSERT_RESULTS()
    }

    SECTION("loop")
    {
        std::stringstream input(R"(First line.
{{header}}
{{#loop somearray item}}
This is a {{item}}.
{{/loop}}
{{footer}}
Last line.)");
        lexer.lex(input, tokens, error);

        std::vector<Token> expectedTokens = {};
        REQUIRE(tokens.size() == 24);

        ASSERT_RESULTS()
    }

    SECTION("lexers gonna lex")
    {
        std::stringstream input("abc {{#xy }}\ndef# {{  # zdg s1  asfa2   }} art {{/audi}}");
        lexer.lex(input, tokens, error);

        std::vector<Token> expectedTokens = {
            TokenFactory::newText("abc ", Context(0, 4)),
            TokenFactory::newStartDirective(Context(4, 6)),
            TokenFactory::newStartBlock(Context(6, 7)),
            TokenFactory::newKeyword("xy", Context(7, 9)),
            TokenFactory::newEndDirective(Context(10, 12)),
            TokenFactory::newText("def# ", Context(13, 18)),
            TokenFactory::newStartDirective(Context(18, 20)),
            TokenFactory::newStartBlock(Context(22, 23)),
            TokenFactory::newKeyword("zdg", Context(24, 27)),
            TokenFactory::newSymbol("s1", Context(28, 30)),
            TokenFactory::newSymbol("asfa2", Context(32, 37)),
            TokenFactory::newEndDirective(Context(40, 42)),
            TokenFactory::newText("art ", Context(43, 47)),
            TokenFactory::newStartDirective(Context(47, 49)),
            TokenFactory::newEndBlock(Context(49, 50)),
            TokenFactory::newKeyword("audi", Context(50, 54)),
            TokenFactory::newEndDirective(Context(54, 56)),
        };
        REQUIRE_THAT(tokens, Equals(expectedTokens));

        ASSERT_RESULTS()
    };
}

} // namespace car