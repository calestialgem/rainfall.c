<!-- SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com> -->
<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Thrice Language Specification

_Thrice_ is a programing language that is at least equivalent to C language,
meaning anything that can be written in C can be written in Thrice with
equivalent semantics.

Thrice tries to add the fallowing features to C language:

- compile-time calculations,
- generics,
- modules,
- packages,
- closures,
- tagged unions,
- tuples,
- resource management with RAII,
- immutability and invisibility by default.

And it removes the fallowing features to have a cleaner language:

- macros,
- implicit narrowing conversions,
- implicit array decay,
- empty parameter list meaning any,
- untyped variadic function arguments.

## Symbol

A _symbol_ is a semantic object.

## Source

A _source_ is a file that has UTF-8 encoded text in it, which is a list of
symbol definitions.

File extension of a source is `.tr`. Name of a source can only contain the lower
and upper case versions of the letters in the English alphabet and decimal
digits. It must start with an upper case letter.

## Modules

A _module_ is a directory, which might contain sources or submodules.

Name of a module has the same rules as the name of a source.

## Packages

A _package_ is a module that is not contained by a supermodule.

A package can be formed out of a single source without a directory, which is
called a _single source package_. Name of a such a package is the name of the
source that defines it. Such a source can include an entry point.

A source with the name `main` that is contained by a package, which is a
directory, is called the _main source_ and it must have an entry point.

All packages are libraries. A package that has an entry point is also an
executable at the same time.

## Visibility

Visibility is whether a symbol can be accessed in the definition of another
symbol.

A symbol is always visible in the source it is defined in. It can be more
accessable using a visibility modifier. There are three visibility modifiers in
Thrice:

- `module` makes a symbol visible in the module it is defined in,
- `intern` makes a symbol visible in the package it is defined in,
- `extern` makes a symbol visible everywhere.

## Namespaces

A Thrice _namespace_ is a collection of symbols. Every source, main source,
module, package, and single source package has their own namespaces.

Namespace of a source or a main source is the set of all the symbols it defines.

Namespace of a module is the junction of all the namespaces of the sources and
submodules it contains.

Namespace of a package is the junction of all the namespaces of the sources and
submodules it contains other than the main source.
