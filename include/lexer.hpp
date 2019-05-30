#ifndef _CARENDER_LEXER_HPP_INCLUDED
#define _CARENDER_LEXER_HPP_INCLUDED

#include <string>
#include <istream>
#include <ostream>
#include <vector>

#include "context.hpp"

using Context = car::Context;

namespace car
{
namespace lexer
{
class Token
{
public:
    enum class Type
    {
        StartDirective,
        EndDirective,
        StartBlock,
        EndBlock,
        Text,
        Keyword,
        Symbol,
    };

    /**
    * Constructs a Token for the `car` template language.
    */
    Token(const Type type, const Context context, const std::string value = "")
        : type(type), value(value), context(context) {}

    friend std::ostream &operator<<(std::ostream &os, const Token &tok);
    friend std::ostream &operator<<(std::ostream &os, const Token::Type &type);

    Token &operator=(const Token &other)
    {
        this->type = other.type;
        this->value = other.value;
        this->context = other.context;

        return *this;
    }

    bool operator==(const Token &rhs) const
    {
        return (type == rhs.type) && (value == rhs.value) && (context == rhs.context);
    }

    bool operator!=(const Token &rhs) const
    {
        return !operator==(rhs);
    }

    /**
    * Get type of the Token.
    */
    Type GetType() const { return this->type; }

    /**
    * Get value of the Token.
    */
    const std::string &GetValue() const { return this->value; }

    /**
    * Get context of the Token.
    */
    const Context &GetContext() const { return this->context; }

private:
    Type type;
    std::string value;
    Context context;
};

class TokenFactory
{
public:
    static const Token newStartDirective(const Context context);
    static const Token newEndDirective(const Context context);
    static const Token newStartBlock(const Context context);
    static const Token newEndBlock(const Context context);
    static const Token newKeyword(const std::string text, const Context context);
    static const Token newSymbol(const std::string text, const Context context);
    static const Token newText(const std::string text, const Context context);
};

class Lexer
{
public:
    /**
    * Constructs an instance of the lexer for the `car` template language.
    */
    Lexer() {}

    /**
    * Lexes `car` template language into tokens.
    */
    bool lex(std::istream &input, std::vector<Token> &output, std::ostream &error);

private:
    const std::string readIdentifier(std::istream &input);

    bool isInDirective = false;
    bool isInBlock = false;
};

} // namespace lexer
} // namespace car
#endif // _CARENDER_LEXER_HPP_INCLUDED