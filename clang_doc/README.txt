Welcome to clang_doc, a doxygen-like source code documentation tool. 

clang_doc a commandline tool that generates html pages for a group of
source files, and can be run as a standalone tool or as part of the
make system.  It's based on the libclang API which is part of the
clang compiler front-end, and is currently under rapid development.

things clang_doc does now:

 - generates html pages for a group of files in a sub-project, e.g., a
   library

 - generates a tag for each sub-project listing external symbols

 - reads multiple tag files from other sub-projects to generate cross
   sub-projects links.

things clang_doc will do:

 - generate an index.html file for each sub-project

 - generate an index.html file for an overall project containing
   multiple sub-projects

 - generate documenation pages for each namespace and class in a
   sub-project

 - generate additional html pages for files, variables, references,
   etc., as needed


To see how clang_doc can be used in a make system, please take a look
at my changes to llvm/Makefile.rules:

https://github.com/donhinton/llvm/commit/1a8039b07fd442da13abc99582e03621afb0322b
