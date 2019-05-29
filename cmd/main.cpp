#include <cstdlib>
#include <unordered_map>
#include <string>
#include <vector>
#include <fstream>

#include "driver.hpp"

using car::driver::Driver;

std::unordered_map<std::string, std::string>
readSymbols(std::istream &input)
{
    auto symbols = std::unordered_map<std::string, std::string>();
    while (!input.eof())
    {
        std::string name;
        std::string value;
        input >> name;
        std::getline(input, value);
        if (value.length() > 0)
        {
            symbols[name] = value.substr(1);
        }
    }

    return symbols;
}

std::unordered_map<std::string, std::vector<std::string>>
readRangeSymbols(std::istream &input)
{
    auto symbols = std::unordered_map<std::string, std::vector<std::string>>();
    std::string name;
    std::vector<std::string> values;

    while (!input.eof())
    {
        std::string value;
        std::getline(input, value);
        if (value.length() > 0)
        {
            if (value[0] == ' ')
            {
                values.push_back(value.substr(1));
            }
            else
            {
                if (name != "")
                {
                    symbols[name] = values;
                    values = std::vector<std::string>();
                }
                name = value;
            }
        }
    }

    if (name != "")
    {
        symbols[name] = values;
        values = std::vector<std::string>();
    }

    return symbols;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << argv[0] << " reads input from stdin, writes output to stdout and errors to stderr." << std::endl;
        std::cout << "Symbols are read from text file in the environment variable SYMBOLS." << std::endl;
        std::cout << "Range Symbols are read from text file in the environment variable RANGE_SYMBOLS." << std::endl;
        std::cout << std::endl;
        std::cout << "Example usage: " << std::endl;
        std::cout << "RANGE_SYMBOLS=ranges.txt SYMBOLS=symbols.txt " << argv[0] << " template.car > template.out" << std::endl;

        return EXIT_FAILURE;
    }

    auto symbols = std::unordered_map<std::string, std::string>();
    if (const char *symbolsName = std::getenv("SYMBOLS"))
    {
        auto ss = std::ifstream(symbolsName);
        symbols = readSymbols(ss);
    }

    auto rangeSymbols = std::unordered_map<std::string, std::vector<std::string>>();
    if (const char *rangeSymbolsName = std::getenv("RANGE_SYMBOLS"))
    {
        auto rss = std::ifstream(rangeSymbolsName);
        rangeSymbols = readRangeSymbols(rss);
    }

    if (std::getenv("DEBUG"))
    {
        for (auto pair : symbols)
        {
            std::cout << "symbol `" << pair.first << "`, value `" << pair.second << "`" << std::endl;
        }

        for (auto pair : rangeSymbols)
        {
            std::cout << "rangeSymbol `" << pair.first << "`" << std::endl;
            for (auto elem : pair.second)
            {
                std::cout << "  value `" << elem << "`" << std::endl;
            }
        }
    }
    auto driver = Driver(symbols, rangeSymbols, std::cout, std::cerr);

    auto templ = std::ifstream(argv[1]);
    auto result = driver.Render(templ) ? EXIT_SUCCESS : EXIT_FAILURE;

    return result;
}