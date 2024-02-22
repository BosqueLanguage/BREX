# BRex for better Regexing!
This project is a reimagining of regular language processing tools motivated by the needs of the Bosque Object Notation language ([BSQON](https://github.com/BosqueLanguage/BSQON)) project. This notation language makes heavy use of regular expressions for data description. Practical experience with regular expressions, and emperical results from academic reseaerch including [Regexes are Hard: Decision-making, Difficulties, and Risks in Programming Regular Expressions](https://davisjam.github.io//files/publications/MichaelDonohueDavisLeeServant-RegexesAreHard-ASE19.pdf) and [Exploring Regular Expression Usage and Contextin Python](https://kstolee.github.io/papers/ISSTA2016.pdf), have shown that classic PCRE style regular expression languages have maintainability, readability, and correctness issues. These challenges are particularly acute in the context of the BSQON project, where regular expressions are extensively used to describe and validate rich datatypes. Thus, the BRex regex language is designed to be a more maintainable, readable, and correct alternative to classic PCRE style regular expressions as well as support novel features of that are useful in the context of describe and validating data with a regular language.

## Goals
Specific goals for the BRex language include:
- **Readability**: BRex should be more comprehensible than classic PCRE style regular expressions. Specifically, BRex uses quoted literals, allows line breaks and comments, simplifies the context dependent use of special characters, and supports named regex patterns for composition and reuse.

- **Correctness**: BRex aims to reliably produce correct expressions. This means eliminating rarely used but complex features (back-references) in favor of alternative workflows,  providing a more structured and less error prone syntax, and simplifying greedy/lazy matching behavior. 

- **Performance**: BRex is designed with performance as a primary goal. This is both for efficient average case matching, with a NFA simulation based engine, and for efficient worst case matching, avoiding pathalogical ReDOS behavior.

- **Extensibility**: BRex should expand the domain where regular language tools can be applied. Specifically, BRex adds new features, such as conjunction and negation, that are useful in data specification/validation, and provides a full language for describing URI paths and path-globs with well founded semantics.

- **Tooling**: As part of the BRex project we aim to provide a rich tooling ecosystem for working with these expressions. This includes tools for testing, sampling inputs, and synthesizing expressions from examples, and IDE highlighting/linting.

- **Universality** BRex is designed for everyone! Unicode (utf8) support is part of the design and implementation from the start and ASCII regexes are a distinct subset for use where the simplicity of the ASCII subset is desired.

## Overall Design
BRex introduces new (text)[docs/regex_semantics.md] and (path)[docs/path_semantics.md] languages + core infrastructure components for structured text processing. The goal is to provide a core [native API](docs/native_api.md) for embedding into other applications that operates on byte buffers and provides a uniform interface for operating on strings with regular expressions. On top of this core API this project exposes a Node.js native module **TODO** and a (command line tool)[docs/brex_cmd.md], `brex`, that provides a simple AWK like interface for using BRex expressions to process text files. Finally, the project plans to leverage improvements in the expression semantics to create improved (and novel new) tools to working with these languages (see the issues tracker).

Unicode support is a foundational part of the BRex design and implementation. As such the BRex language support the full unicode char set and is fully utf8 aware. However, in many cases the simplicity of ASCII regexes is desired and BRex provides an explicit ASCII regex and processing pipeline as well. 

The matching engine is based on a NFA simulation to ensure that the average case performance is efficient and that the worst case performance is not pathalogical ([ReDOS](https://en.wikipedia.org/wiki/ReDoS)). This ensures that BRex can be used in a wide range of applications, particularly data validation, without severe risks around performance issues.

Finally, BRex includes a number of novel language features that extend classic regular expression langauges with features that are useful in the context of data validation and specification. These include named patterns, conjunction, negation, and an explicit URI path language ([BPath](docs/path_semnatics.md])). These features, combined with various ergonomic improvements, make BRex a powerful and expressive language for working with regular expressions and structured textual data more generally!

## Notable Features
BRex includes a number of distinct features that are not present in classic PCRE style regular expressions. These include:

- **Quoted Literals**: BRex supports quoted literals, `/"this is a literal"*/`. This allows for the use of special characters in literals without escaping, making the expression more readable, and provides a way to distingush `/"unicode literal 🌶"/` from `/'ascii literals %x59;'/a`.

- **Unified Escaping**: BRex eschews the classic, and frequently error prone, PCRE style of using `\` to escape special characters in favor of a unified escaping mechanism. All characters can be escaped using hex codes `/"%x7;%0;"/` or memonic names `%a;`, `%NUL;`, or `%;` (for the quote literal). Unicode can be hex escaped or inserted directly into the string (or char range `/[🌵-🌶]/`)

- **Comments and Line Breaks**: BRex supports comments and splitting as well as whitespace within an expression (outside of a literal or range whitespace is ignored). This allows for the expression to be structured for readability `/[+-]? "0"|[1-9][0-9]+/`. 

- **Named Patterns**: BRex supports named patterns for composition and reuse allowing expressions to be built up in parts and common features to be shared across multiple expressions -- `/[+-]${Digit}+/`.

- **Negation**: BRex supports a syntactically limited from of negation (`!` operator) to express that a string does not match a regular expression -- `/!(".txt" | ".pdf")/`. This generalizes conjunction in ranges `[^a-z]` to the full language -- but only the top-level can be negated allowing us to preserve match efficiency.

- **Conjunction**: BRex supports the `&` operator to express, in a single regular expression, that a string must a member of multiple languages! This is a invaluable feature for data validation tasks where there are often multiple logically-distinct constrains that would otherwise be forced into a single expression -- check a zipcode is valid *and* for Kentucky `/"[0-9]{5}(-[0-9]{3})? & ^"4"[0-2]/` or with names `/${Zipcode} & ^${PrefixKY}/`.

- **Anchors**: BRex supports anchors that allow guarded and conditional matching before and after fixed regex -- `/${UserName}"_"^ <[a-zA-Z]+> $!(".tmp" | ".scratch")/` which matches the `[a-zA-Z]+` term but only when proceeded by the username/underscore and not followed by the given extensions.

- **URI Paths**: BRex includes a full language **TODO** for describing URI paths and path-globs with well founded semantics. This is useful for describing and validating URI paths. This allows us to easily express the constraints on these commonly occurring and complex strings -- `\file://home/user/["BRex" | "BSQON"]/**/["output_"[0-9]+].log\g`

## Examples

