/* -*- Mode: C++ -*-
//
// \file: clang_doc_main.cpp
//
// \author: Don Allon Hinton <don.hinton@gmx.com>
// \date: 13 Aug 2012 14:33:27 UTC
//
*/

#include "Clang_Doc.h"
#include <getopt.h>
#include <iostream>
#include <set>
#include <sys/param.h>
#include <stdlib.h>

namespace {

short g_debug = 0;
std::string g_root_dir = "";
std::string g_html_dir = "html";
std::string g_object_dir = ".obj";
std::string g_file;
std::string g_tag_out;
std::set<std::string> g_tags;


void usage(void) {

  printf("usage: clang_doc [clang_doc Options] -- [clang Options]\n\n");
  printf("Options:\n\n");
  printf("  -h, --help             this screen\n");
  printf("  -d, --debug            enable debugging output\n");
  printf("  -R, --root_dir=arg     root directory (this path is trimmed from input files\n");
  printf("                         when generating html filenames)\n");
  printf("  -D, --html_dir=arg     html output directory (default: html) -- it must exist\n");
  printf("  -O, --object_dir=arg   location to store translation unit objects (they only\n");
  printf("                         get regenerated if the underlying source changes.\n");
  printf("                         (default .obj) -- it must exist\n");
  printf("  -t, --tag_in=arg       input tag file(s) -- can provide multiple\n");
  printf("  -T, --tag_out=arg      out tag file\n");
  printf("  -f, --file=arg         input file (if not provided, read from stdin)\n\n");
  printf("Example:\n\n");
  printf("  find libclang -name \"*.h\" -or -name \"*.cpp\" | clang_doc -- -I ../../../include\n\n");
  printf("The html files use the llvm version of doxygen.css located in llvm/docs/.\n");
  printf("Please copy it to the html directory.\n\n");

}

int parse (int& argc, char**& argv) {
  int c;

  int option_index = 0;
  struct option long_options[] = {
    {"help", no_argument, 0, 'h'},
    {"debug", no_argument, 0, 'd'},
    {"root_dir", required_argument, 0, 'R'},
    {"html_dir", required_argument, 0, 'D'},
    {"object_dir", required_argument, 0, 'O'},
    {"tag_in", required_argument, 0, 't'},
    {"tag_out", required_argument, 0, 'T'},
    {"file", required_argument, 0, 'f'},
    {0, 0, 0, 0}
  };

  while (1) {
    char path[1024];
    c = getopt_long (argc, argv, "+:dR:D:O:f:t:T:h", long_options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 'd':
      g_debug = 1;
      break;
    case 'R':
      g_root_dir = realpath(optarg, path);
      break;
    case 'D':
      g_html_dir = realpath(optarg, path);
      break;
    case 'O':
      g_object_dir = realpath(optarg, path);
      break;
    case 'f':
      g_file = realpath(optarg, path);
      break;
    case 't':
      g_tags.insert (optarg);
      break;
    case 'T':
      g_tag_out = optarg;
      break;
    case '?':
    case 'h':
      usage();
      return 1;
    default:
      // ignore unknown options
      break;
    }
  }

  argc -= optind;
  argv += optind;

  return 0;
}

} // annonymous namespace

int
main (int argc , char *argv[]) {
  if (parse (argc, argv) != 0)
    return 1;

  std::cout << "file:       " << g_file.c_str() << "\n";
  std::cout << "root_dir:   " << g_root_dir.c_str() << "\n";
  std::cout << "html_dir:   " << g_html_dir.c_str() << "\n";
  std::cout << "object_dir: " << g_object_dir.c_str() << "\n";

  std::cout << "\nremaining commandline args:\n";
  for (int i = 0; i < argc; ++i)
    std::cout << argv[i] << " ";
  std::cout << "\n";

  std::set<std::string> files;
  if (g_file.empty()) {
    std::string line;
    while (true) {
      std::getline(std::cin, line);
      if (line.empty())
        break;
      char path[1024];
      files.insert(realpath(line.c_str(), path));
    }
  }
  else
    files.insert(g_file);

  // make sure we don't read our own tag file
  files.erase(g_tag_out);

#if 0
  std::cout << "\ninput files:\n";
  for (std::set<std::string>::const_iterator i = files.begin(), e = files.end();
       i != e; ++i) {
    std::cout << (*i).c_str() << "\n";
  }
#endif

  clang_doc::Clang_Doc doc =
    clang_doc::Clang_Doc(argc, argv, files, g_object_dir, g_html_dir, g_root_dir);

  doc.generate_symbol_table (g_tags);
  doc.generate_html_files (g_tag_out);

  std::cout << "\ndone...\n";

  return 0;
}
