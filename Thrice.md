# Thrice Language Specification

_Thrice_ is a programing language that is equivalent to C language with quality
of life improvements, while compiling down to C.

Name Thrice comes from the idea of the program running three times: once as a
constant expression, once as an executable and once as a script.

## Symbol

A `symbol` is a semantic object. These are `binding`s, `variable`s, `procedure`s
and `type`s.

## Source

A _source_ is a file that has UTF-8 encoded text in it, which is a list of
symbol definitions.

File extension of a source is `.tr`. Name of a source can only contain English
letters and decimal digits. It must start with an uppercase English letter.

## Module

A _module_ is a directory, which contains at least one source or module.

Name of a module has to obey the same rules as the source names.

## Package

A _package_ is a Thrice library. It is also an executable if it has an entry
point.

Packages can be structured as a single source or module. In both of these cases,
the name of the source or the module becomes the name of the package.

If the package is formed out of a single source, that source can have an entry
point in it. Otherwise, the package can have a single source that has an entry
point, which is called a _main file_. Main file has to be directly under the
module directory that forms the package.

## Visibility

_Visibility_ is whether a symbol can be accessed in the definition of another
symbol.

A symbol is always visible in the source it is defined in. Its visibility can be
increased using a _visibility modifier_. There are 3 visibility modifiers in
Thrice:

- `private` makes a symbol visible in the module it is defined in,
- `protected` makes a symbol visible in the package it is defined in,
- `public` makes a symbol visible everywhere.

The visibility modifier must be the first thing in the definition of a symbol.

The main file is considered as a file outside of the package. Thus, it can only
access `public` symbols of the executable package it is in. On the other hand,
the entry point in a package that is formed from a single source can access all
the symbols defined in that source as normal.
