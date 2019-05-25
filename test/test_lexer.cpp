#include "catch.hpp"
using Catch::Matchers::Equals;

#include <iostream>
#include <sstream>
#include "lexer.hpp"

using car::lexer::Context;
using car::lexer::Lexer;
using car::lexer::Token;
using car::lexer::TokenFactory;

namespace car
{

TEST_CASE("Lexer::lex", "[lexer]")
{
    std::stringstream error;
    auto lexer = car::lexer::Lexer();
    auto tokens = std::vector<car::lexer::Token>();
    std::string expectedErrors;

    SECTION("Text")
    {
        std::stringstream input("lorem ipsum ");
        lexer.lex(input, tokens, error);

        std::vector<car::lexer::Token> expectedTokens = {
            TokenFactory::newText("lorem ipsum ", Context(0, 11)),
        };
        REQUIRE_THAT(tokens, Equals(expectedTokens));
    }

    SECTION("StartDirective EndDirective")
    {
        std::stringstream input("{{}}");
        lexer.lex(input, tokens, error);

        expectedErrors = "Unexpected token '}' at [2, 3).\n";
        std::vector<car::lexer::Token> expectedTokens = {
            TokenFactory::newStartDirective(Context(0, 2)),
        };
        REQUIRE_THAT(tokens, Equals(expectedTokens));
    }

    SECTION("lexers gonna lex")
    {
        std::stringstream input("abc {{#xy }}\ndef# {{  # zdg s1  asfa2   }} art {{/audi}}");
        lexer.lex(input, tokens, error);

        std::vector<car::lexer::Token> expectedTokens = {
            TokenFactory::newText("abc ", Context(0, 4)),
            TokenFactory::newStartDirective(Context(4, 6)),
            TokenFactory::newStartBlock(Context(6, 7)),
            TokenFactory::newKeyword("xy", Context(7, 9)),
            TokenFactory::newEndDirective(Context(10, 12)),
            TokenFactory::newText("def# ", Context(13, 18)),
            TokenFactory::newStartDirective(Context(18, 20)),
            TokenFactory::newStartBlock(Context(22, 23)),
            TokenFactory::newKeyword("zdg", Context(24, 27)),
            TokenFactory::newSymbol("s1", Context(28, 31)),
            TokenFactory::newSymbol("asfa2", Context(32, 38)),
            TokenFactory::newEndDirective(Context(40, 42)),
            TokenFactory::newText("art ", Context(43, 47)),
            TokenFactory::newStartDirective(Context(47, 49)),
            TokenFactory::newEndBlock(Context(49, 50)),
            TokenFactory::newKeyword("audi", Context(50, 54)),
            TokenFactory::newEndDirective(Context(54, 56)),
        };
        REQUIRE_THAT(tokens, Equals(expectedTokens));
    };

    auto errors = error.str();

    if (expectedErrors.size() == 0)
    {
        REQUIRE(errors.size() == 0);
    }
    else
    {
        REQUIRE(errors == expectedErrors);
    }
}

} // namespace car