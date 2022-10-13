# lisb

A stack-based Scheme bytecode interpreter. 

## About this project

This my attempt at implementing a complete Scheme interpreter as specified in *Revised^7 Report on the Algorithmic Language Scheme* (abbreviated *R7RS-small*).

Why Scheme? Scheme is an old langue and it's very well documented. It also has a very simple syntax, which let's you focus on the juicy parts of implementing a programming language, namely the semantics. Scheme also has a lot of interesting features to implement, such as: first-class functions, closures, continuations, garbage collection, lexical scoping, tail-call optimization, and hygenic macros that let you extend the language by a large margin. Scheme is compact while at the same time very extensible.

Why a bytecode interpreter? I have written simple compilers that generate assembly code before, but never an interpreter. Writing a feature-rich compiler on your own is hard work and writing your own bytecode interpreter makes that work a little easier. I didn't want to make it too easy, however, so I chose a stack-based approach instead of using the heap, which is more in line with traditional Scheme implementations.

Much of the work is owed to the book *Crafting Interpreters*, especially the implementation of closures (which uses upvalues like in Lua).

## Roadmap

This is an ongoing project. For more information about what is has been done and what is left to do, see the roadmap below. I have tried to outline the most core aspects of the langue that I deem necessary to implement before I can progress and implement the language as a whole. For example, the fact that I have checked the box for *primitive functions* does not mean that I have implemented every primitive in the standard library, but that I have built the foundation for doing that. The idea is that I will update the roadmap with new goals as I continue to work on the project. I also haven't included things that I consider too small or part of a larger goal.

* [x] Generating s-expressions for source code
* [x] Support global and local variables (including scopes)
* [x] If-expressions and jumps.
* [x] Function definitions and applications.
* [x] Support for primitive functions.
* [x] Implement closures.
* [x] Support for garbage collection.
* [ ] Implement continuations.
* [ ] Implement quotes and evaluation.
* [ ] Support tail recursion.
* [ ] Hygenic macros.
* [ ] `<void>`
