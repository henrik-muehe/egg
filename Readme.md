# Egg Parsing Expression Grammar Generator #

Egg is a parser generator for parsing expression grammars (PEGs). 
Its grammar is based on the grammar of Ian Piumarta's [`leg`](http://piumarta.com/software/peg/). 
Parsing expression grammars are a formalization of recursive top-down parsers; they are similar to context free grammars, with the primary difference being that alternation in a PEG is ordered, while it is unordered in context free grammars. 
Due to this quality, a PEG-based parser generator such as Egg does not need a separate lexer, and can do parsing in one pass.

Egg is written in C++11, using modern constructs. 
Egg supports inclusion of C++ code inline in rules, as well as returning objects of any default-constructable type from a rule match (this type may be different for different rules).

## Usage ##

This Readme contains only summary information, full documentation can be found in the `docs` folder; a detailed description of the grammar constructs can be found in the Egg Grammar Guide. Those who want to modify or contribute to the project should also read the Developer Guide.

### Usage Summary ###

    egg [command] [flags] [input-file [output-file]]
    
Supported flags are

- `-i --input`		input file (default stdin)
- `-o --output`		output file (default stdout)
- `-c --command`	command - either compile or print (default compile)
- `-n --name`		grammar name - if none given, takes the longest prefix of the input or output file name (output preferred) which is a valid Egg identifier (default empty)
- `--no-norm`       turns off grammar normalization

### Grammar Summary ###

A more complete grammar may be found in the Grammar Guide, and some simple example grammars may be found in the `grammars` directory. 

- A grammar is a sequence of rules
- Rules are of the form `name (":" type)? "=" matcher`. 
  `name` may be used in other rules (or even recursively in the matcher) to match the rule; if a type is given for the rule, `name ":" id` is a matcher that will bind `id` to a variable of type `type` returned by the rule.
  Rule names are composed of alphanumeric characters and underscores, where the first character may not be a digit.
- Matchers can be combined in sequence simply by writing them in sequence, `matcher_1 matcher_2`
- Choice between matchers is represented as `choice_1 "|" choice_2`; this choice is _ordered_, that is, if `choice_1` matches, no attempt will be made to match `choice_2`.
- Matchers can be grouped into a larger matcher by surrounding them with parentheses, `"(" matcher_1 matcher_2 ... ")"`
- Matchers can be made optional by appending a `?`, repeatable by appending a `*`, or repeatable at least once by appending a `+`.
- `"&" matcher` provides lookahead - the matcher will run, but no input will be consumed. 
  `!` works similarly, except `"!" matcher` only matches if `matcher` _doesn't_.
- Character literals and string literals are matchers for those characters or strings, and are denoted by surrounding them in single `'` or double `"` quotes, respectively. 
  ''', '"', and '\' are backslash-escaped as in C, the escapes "\n", "\r", and "\t" also work.
- A character class obeys the following syntax: `"[" (char_1 '-' char_2 | char)* "]"`. 
  `char_1 '-' char_2` will match any character between `char_1` and `char_2`, while `char` matches the given character.
- `.` matches any character, `;` is an empty matcher that always matches without consuming any input.
- An action consists of C++ code surrounded with curly braces `{ }`. 
  Any C++ code that can be placed in a function is permitted, assuming that it is syntactically complete. 
  Any variables bound from rule matchers are available in this code, as is `psStart`, the index of the start of the current match (or parenthesized matcher) and `ps`, the current parser state (`ps.pos` is the current index, `ps.string(i,n)` is the `n` characters starting at `i`, other public functions can be found in the Grammar Guide).
- An matcher can be surrounded with angle brackets `< >` to capture the string which is matched. 
  After this the variables `psCatch`, `psCatchLen`, and `psCapture` will be defined, holding the start index of the capture, the length of the capture, and the captured string, respectively.
- One-line comments start with a `#`
- Whitespace is not significant except to delimit tokens

### Using Generated Headers ###

Egg generates C++ headers implementing the input grammar; the generated code is in a namespace which is by default the same name as the input file (less extensions and any other suffix which is not a valid Egg identifier). 
These headers depend on the Egg header `parse.hpp`, which defines the `parse` namespace, and must be located in the same folder. 
(At some point this header may be inlined, but I need to determine how to address the licencing considerations of this first.) 
Each grammar rule generates a function with the same name; this function takes a `parse::state` reference as a parameter, and returns a `parse::result<T>`, where `T` is the type of the rule (if the rule is untyped, this defaults to `parse::value`, an empty class).

A `parse::state` object encapsulates the current parser state. 
Its constructor takes a `std::istream` reference as a parameter, which it will read from. 
It exposes a mutable public `pos` member, the current read index, as well as a variety of public methods: `operator[]` takes an index and returns the character at that index, `range(begin, len)` returns a `std::pair` of iterators pointing to the input character at `begin` and the character at most `len` characters later, and `string(begin, len)` returns the `std::string` represented by `range(begin, len)`.

A `parse::result<T>` optionally contains a value of type `T` (`T` must be default constructable). 
`parse::result<T>` is implicitly convertable to both `T` and `bool` - it will return the default value of `T` or `false` if no value is stored, and the value or `true` otherwise; the stored value can be explicitly returned with the `*` dereference operator. 
A `parse::result<T>` can be constructed either from a `T&`, which it will then contain, or a `parse::failure` object, which will leave it empty - `parse::result<T>` is also default constructable, which is equivalent to the `parse::failure` option. 
The templated methods `parse::match<T>(const T&)` and `parse::fail<T>()` are shorthands for these constructors.

## Installation ##

Run `make egg` from the main directory. 
Make is obviously required, but the only other requirement is a relatively modern C++ compiler with support for C++11 constructs; Egg is tested on clang++ 3.2 and g++ 4.7.3, but may work with older compilers.

## Testing ##

Run `make test` from the `grammars` directory. 
This may result in a fair bit of output, but if the last line reads "TESTS PASSED" then they have been successful.

## Licence ##

Egg is released under the MIT licence (see the included LICENCE file for details). 
Egg-generated parsers have no licence imposed on them, but do depend on some MIT licenced headers from the project - subject to further legal and technical consideration, Egg may be modified in the future to inline these headers with a GNU Bison-style licence exception.