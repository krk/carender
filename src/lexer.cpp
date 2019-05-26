#include "lexer.hpp"

using namespace car;

namespace car
{
namespace lexer
{

const Token TokenFactory::newStartDirective(const Context context)
{
    return Token(Token::Type::StartDirective, context);
}

const Token TokenFactory::newEndDirective(const Context context)
{
    return Token(Token::Type::EndDirective, context);
}

const Token TokenFactory::newStartBlock(const Context context)
{
    return Token(Token::Type::StartBlock, context);
}

const Token TokenFactory::newEndBlock(const Context context)
{
    return Token(Token::Type::EndBlock, context);
}

const Token TokenFactory::newKeyword(const std::string text, const Context context)
{
    return Token(Token::Type::Keyword, context, text);
}

const Token TokenFactory::newSymbol(const std::string text, const Context context)
{
    return Token(Token::Type::Symbol, context, text);
}

const Token TokenFactory::newText(const std::string text, const Context context)
{
    return Token(Token::Type::Text, context, text);
}

std::ostream &operator<<(std::ostream &os, const Token::Type &type)
{
    switch (type)
    {
    case Token::Type::StartDirective:
        os << "StartDirective";
        break;
    case Token::Type::EndDirective:
        os << "EndDirective";
        break;
    case Token::Type::StartBlock:
        os << "StartBlock";
        break;
    case Token::Type::EndBlock:
        os << "EndBlock";
        break;
    case Token::Type::Text:
        os << "Text";
        break;
    case Token::Type::Keyword:
        os << "Keyword";
        break;
    case Token::Type::Symbol:
        os << "Symbol";
        break;
    default:
        os << "#unsupported token type#";
        break;
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const Token &tok)
{
    os << "[" << tok.type << " at " << tok.context << "]";
    if (tok.value.size() > 0)
    {
        os << " '" << tok.value << "'";
    }
    return os;
}

const std::string Lexer::readIdentifier(std::istream &input)
{
    std::string word;
    char c;
    while (input >> c && !std::isspace(c) && c != '}')
    {
        word += c;
    }

    if (input.tellg() != EOF)
    {
        input.seekg(-1, std::ios_base::cur);
    }

    return word;
}

bool Lexer::lex(std::istream &input, std::vector<Token> &output, std::ostream &error)
{
    char prev_c;
    char c;

    long textPos = -1;
    std::string text;

    input >> std::noskipws;

    while (input >> c)
    {
        if (this->isInDirective)
        {
            if (this->isInBlock)
            {
                auto pos = input.tellg();
                // Token can be a Symbol or an EndDirective.
                if (std::isspace(c))
                {
                    output.push_back(TokenFactory::newSymbol(text, Context(textPos, pos - 1L)));
                    textPos = -1;
                    text = "";
                    input >> std::ws;
                }
                else if (c == '}' && prev_c == '}')
                {
                    output.push_back(TokenFactory::newEndDirective(Context(pos - 2L, pos)));
                    input >> std::ws;
                    this->isInBlock = false;
                    this->isInDirective = false;
                }
                else if (c == '}' && input.peek() == '}')
                {
                    if (text.size() != 0)
                    {
                        output.push_back(TokenFactory::newSymbol(text, Context(textPos, textPos + text.size())));
                        textPos = -1;
                        text = "";
                    }

                    prev_c = c;
                    continue;
                }
                else
                {
                    if (textPos == -1)
                    {
                        textPos = pos - 1L;
                    }
                    text += c;
                }
            }
            else
            {
                auto pos = input.tellg();
                auto ctx = Context(pos - 1L, pos);
                auto isSymbol = false;

                // Token can be a StartBlock, an EndBlock or a Symbol.
                switch (c)
                {
                case '#':
                    output.push_back(TokenFactory::newStartBlock(ctx));
                    break;
                case '/':
                    output.push_back(TokenFactory::newEndBlock(ctx));
                    break;
                case '}':
                    if (input.peek() == '}')
                    {
                        // End directive.
                        input >> c;
                        pos = input.tellg();
                        this->isInBlock = false;
                        this->isInDirective = false;
                        output.push_back(TokenFactory::newEndDirective(Context(pos - 2L, pos)));
                        continue;
                    }
                    error << "Unexpected token '}' at " << ctx << "." << std::endl;
                    return false;
                default:
                    isSymbol = true;
                    break;
                }

                // Token can be a keyword.
                input >> std::ws;
                pos = input.tellg();
                auto identifier = this->readIdentifier(input);

                if (isSymbol)
                {
                    identifier = c + identifier;
                }

                if (identifier.size() == 0)
                {
                    error << "Expected keyword at " << ctx << "." << std::endl;
                    return false;
                }

                output.push_back(isSymbol
                                     ? TokenFactory::newSymbol(identifier, Context(pos - 1L, input.tellg()))
                                     : TokenFactory::newKeyword(identifier, Context(pos, input.tellg())));
                input >> std::ws;
                this->isInBlock = !isSymbol;
            }
        }
        else
        {
            auto pos = input.tellg();
            // Token can be either a Text or a StartDirective.
            if (c == '{' && prev_c == '{')
            {
                if (text.size() != 0)
                {
                    output.push_back(TokenFactory::newText(text, Context(textPos, pos - 2L)));
                    textPos = -1;
                    text = "";
                }

                output.push_back(TokenFactory::newStartDirective(Context(pos - 2L, pos)));
                this->isInDirective = true;
                input >> std::ws;
            }
            else if (c == '{' && input.peek() == '{')
            {
                prev_c = c;
                continue;
            }
            else
            {
                if (textPos == -1)
                {
                    textPos = pos - 1L;
                }
                text += c;
            }
        }

        prev_c = c;
    }

    if (text.size() != 0)
    {
        output.push_back(TokenFactory::newText(text, Context(textPos, textPos + text.size())));
        textPos = -1;
        text = "";
    }

    if (this->isInBlock)
    {
        error << "Block not closed." << std::endl;
        return false;
    }

    if (this->isInDirective)
    {
        error << "Directive not closed." << std::endl;
        return false;
    }

    return true;
}

} // namespace lexer
} // namespace car