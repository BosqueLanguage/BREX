# BREX
This project is a reimagining of regular language processing tools motivated by the needs of the Bosque Object Notation language ([BSQON](https://github.com/BosqueLanguage/BSQON)) project. This notation language makes heavy use of regular expressions for data description. Practical experience with regular expressions, and emperical results from academic reseaerch including [Regexes are Hard: Decision-making, Difficulties, and Risks in Programming Regular Expressions](https://davisjam.github.io//files/publications/MichaelDonohueDavisLeeServant-RegexesAreHard-ASE19.pdf) and [Exploring Regular Expression Usage and Contextin Python](https://kstolee.github.io/papers/ISSTA2016.pdf), have shown that classic PCRE style regular expression languages have maintainability, readability, and correctness issues. These challenges are particularly acute in the context of the BSQON project, where regular expressions are extensively used to describe and validate rich datatypes. Thus, the BREX regex language is designed to be a more maintainable, readable, and correct alternative to classic PCRE style regular expressions as well as support novel features of that are useful in the context of describe and validating data with a regular language.

## Goals
Specific goals for the BREX language include:
- **Readability**: BREX should be more comprehensible than classic PCRE style regular expressions. Specifically, BREX uses quoted literals, allows line breaks and comments, simplifies the context dependent use of special characters, and supports named regex patterns for composition and reuse.

- **Correctness**: BREX aims to reliably produce correct expressions. This means eliminating rarely used but complex features (back-references) in favor of alternative workflows,  providing a more structured and less error prone syntax, and simplifying greedy/lazy matching behavior. 

- **Performance**: BREX is designed with performance as a primary goal. This is both for efficient average case matching, with a NFA simulation based engine, and for efficient worst case matching, avoiding pathalogical ReDOS behavior.

- **Extensibility**: BREX should expand the domain where regular language tools can be applied. Specifically, BREX adds new features, such as conjunction and negation, that are useful in data specification/validation, and provides a full language for describing URI paths and path-globs with well founded semantics.

- **Tooling**: As part of the BREX project we aim to provide a rich tooling ecosystem for working with these expressions. This includes tools for testing, sampling inputs, and synthesizing expressions from examples, and IDE highlighting/linting.

- **Universality** BREX is designed for everyone! Unicode (utf8) support is part of the design and implementation from the start and ASCII regexes are a distinct subset for use where the simplicity of the ASCII subset is desired.

## Overall Design
unicode and ascii aware

additional features for validation and data parsing tasks (conjunction, negation, uri paths)

NFA based engine

API

command line tool

## Notable Features
BREX includes a number of distinct features that are not present in classic PCRE style regular expressions. These include:

- **Quoted Literals**: BREX supports quoted literals, `/"this is a literal"*/`. This allows for the use of special characters in literals without escaping, making the expression more readable, and provides a way to distingush `/"unicode literal 🌶"/` from `/'ascii literals %x1F336;'/a`.

- **Unified Escaping**: BREX eschews the classic, and frequently error prone, PCRE style of using `\` to escape special characters in favor of a unified escaping mechanism. All characters can be escaped using hex codes `/"%x7;%0;"/` or memonic names `%a;`, `%NUL;`, or `%;` (for the quote literal). Unicode can be hex escaped or inserted directly into the string (or char range `/[🌵-🌶]/`)

- **Comments and Line Breaks**: BREX supports comments and splitting as well as whitespace within an expression (outside of a literal or range whitespace is ignored). This allows for the expression to be structured for readability `/[+-]? "0"|[1-9][0-9]+/`. 

- **Named Patterns**: BREX supports named patterns for composition and reuse allowing expressions to be built up in parts and common features to be shared across multiple expressions -- `/[+-]${Digit}+/`.

- **Negation**: BREX supports a syntactically limited from of negation (`!` operator) to express that a string does not match a regular expression -- `/!(".txt" | ".pdf")/`. This generalizes conjunction in ranges `[^a-z]` to the full language -- but only the top-level can be negated allowing us to preserve match efficiency.

- **Conjunction**: BREX supports the `&` operator to express, in a single regular expression, that a string must a member of multiple languages! This is a invaluable feature for data validation tasks where there are often multiple logically-distinct constrains that would otherwise be forced into a single expression -- check a zipcode is valid *and* for Kentucky `/"[0-9]{5}(-[0-9]{3})? & ^"4"[0-2]/` or with names `/${Zipcode} & ^${PrefixKY}/`.

- **Anchors**: BREX supports anchors that allow guarded and conditional matching before and after fixed regex -- `/${UserName}"_"^ <[a-zA-Z]+> $!(".tmp" | ".scratch")/` which matches the `[a-zA-Z]+` term but only when proceeded by the username/underscore and not followed by the given extensions.

- **URI Paths**: BREX includes a full language [TODO] for describing URI paths and path-globs with well founded semantics. This is useful for describing and validating URI paths. This allows us to easily express the constraints on these commonly occurring and complex strings -- `\file://home/user/["BREX" | "BSQON"]/**/["output_"[0-9]+].log\g`

## Examples

