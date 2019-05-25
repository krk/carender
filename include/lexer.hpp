#ifndef _LEXER_HPP_INCLUDED
#define _LEXER_HPP_INCLUDED

#include <string>
#include <istream>
#include <ostream>
#include <vector>

namespace car
{
namespace lexer
{
class Context
{
public:
    Context(int startPos, int endPos) : startPos(startPos), endPos(endPos) {}

    friend std::ostream &operator<<(std::ostream &os, const Context &ctx);

    bool operator==(const Context &rhs) const
    {
        return (startPos == rhs.startPos) && (endPos == rhs.endPos);
    }

    bool operator!=(const Context &rhs) const
    {
        return !operator==(rhs);
    }

private:
    int startPos;
    int endPos;
};

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

    Token(const Type type, const Context context, const std::string value = "")
        : type(type), value(value), context(context) {}

    friend std::ostream &operator<<(std::ostream &os, const Token &tok);
    friend std::ostream &operator<<(std::ostream &os, const Token::Type &type);

    bool operator==(const Token &rhs) const
    {
        return (type == rhs.type) && (value == rhs.value) && (context == rhs.context);
    }

    bool operator!=(const Token &rhs) const
    {
        return !operator==(rhs);
    }

private:
    const Type type;
    const std::string value;
    const Context context;
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
    /// Creates an instance of the lexer with StartDirective "{{",
    // EndDirective "}}", StartBlock "#" and EndBlock "/".
    Lexer() {}

    bool lex(std::istream &input, std::vector<Token> &output, std::ostream &error);

private:
    const std::string readKeyword(std::istream &input);

    bool isInDirective = false;
    bool isInBlock = false;
};

} // namespace lexer
} // namespace car
#endif