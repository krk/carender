# carender - car template renderer

`carender` renders a `car` template with provided variables to create a text output.

## Quick Start

On a system that can run binaries built on `debian:stretch`, run the following commands to see a rendered template:
```sh
./build_in_docker.sh
cd from_docker/bin
RANGE_SYMBOLS=../../examples/fruits/ranges.txt SYMBOLS=../../examples/fruits/symbols.txt ./carender ../../examples/fruits/template.car
```

## Usage
`carender` can be linked as a static library. A command line tool is provided as an example of using the library.

### High-level Driver API

Driver API is designed to be batteries-included, for consumers that do not want to configure subcomponents (`lexer`, `parser`, `renderer`) separately.

```c++
#include "driver.hpp"
#include <fstream>

// symbols and rangeSymbols are unordered_map instances defining valid symbols and their values for the template.
auto driver = car::driver::Driver(symbols, rangeSymbols, std::cout, std::cerr);
auto templ = std::ifstream("template.car");

if(driver.Render(templ)) {
    // We have the template rendered to the `output` stream, `std::cout` in this case.
    return;
}
// There were some errors, they are written to the `error` stream, `std::cerr` in this case.
```

See the sample application for an example usage of the Driver API at [main.cpp](cmd/main.cpp)

### Low-level API

Low-level API allows for configuration of the subcomponents. New options may be added in new versions.

#### Lexer

`Lexer` consumes an input stream and emits a stream of tokens as a `std::vector<car::lexer::Token>`. See [test_lexer.cpp](test/test_lexer.cpp) and [driver.cpp](cmd/driver.cpp) for usage examples.

```c++
auto lexer = car::lexer::Lexer();
auto tokens = std::vector<car::lexer::Token>();

lexer.lex(input, tokens, error);
```

#### Parser

`Parser` consumes a token stream and emits a stream of nodes as a `std::vector<std::unique_ptr<Node>>`. See [test_parser.cpp](test/test_parser.cpp) and [driver.cpp](cmd/driver.cpp) for usage examples.

```c++
auto options = ParserOptions(symbolNames);
auto parser = Parser(options);

auto nodes = parser.parse(tokens, error);
```

#### Renderer

`Renderer` consumes a node stream and emits text. See [test_renderer.cpp](test/test_renderer.cpp) and [driver.cpp](cmd/driver.cpp) for usage examples.

```c++
auto renderer = Renderer(symbols, rangeSymbols, output, error);
for (auto const &n : nodes)
{
    n->accept(renderer);
}
```

# `car` template language

## An example to get a taste:

### Template
```
Hello {{name}},

{{#loop items item}}
I like {{item}}{{#ifeq item favorite}} very much{{/ifeq}}.
{{/loop}}

Cheers!
```

### Variables
```
name: "Donald"
items: ["apples", "pineapples", "oranges"]
favorite: "pineapples"
```

### Rendered output
```
Hello Donald,

I like apples.
I like pineapplesvery much.
I like oranges.

Cheers!
```

## Grammar
In an approximate E-BNF:
```
TEMPLATE = TEXT | PRINT | LOOP | IFEQ

CHILDREN = TEMPLATE

START_DIRECTIVE = "{{"
END_DIRECTIVE = "}}"
START_BLOCK = "#"
END_BLOCK = "/"
SYMBOL = TEXT

TEXT = a string not containing START_DIRECTIVE

PRINT = START_DIRECTIVE, SYMBOL, END_DIRECTIVE

LOOP = START_DIRECTIVE, START_BLOCK, "loop", END_DIRECTIVE, CHILDREN, START_DIRECTIVE, END_BLOCK, "loop", END_DIRECTIVE

IFEQ = START_DIRECTIVE, START_BLOCK, "ifeq", END_DIRECTIVE, CHILDREN, START_DIRECTIVE, END_BLOCK, "ifeq", END_DIRECTIVE
```