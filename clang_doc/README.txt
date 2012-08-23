Welcome to clang_doc, a doxygen-like source code documentation tool. 

clang_doc a commandline tool that generates html pages for a group of
source files, and can be run as a standalone tool or as part of the
make system.  It's based on the libclang API which is part of the
clang compiler front-end, and is currently under rapid development.

things clang_doc does now:

 - generates html pages for a group of files in a sub-project, e.g., a
   library

 - generates a tag for with external symbols

 - can read multiple tag files to generate cross sub-projects links.

things clang_doc will do:

 - generate an index.html files for each sub-project

 - generate an index.html for an overall project including multiple
   sub-projects

 - generate documenation pages for each namespace and class in a
   sub-project

 - generate additional html pages for files, variables, references,
   etc.
