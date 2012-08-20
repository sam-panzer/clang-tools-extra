/* -*- Mode: C -*-
//
// \file: Clang_Doc.h
//
// \author: Don Allon Hinton <don.hinton@gmx.com>
// \date: 13 Aug 2012 19:19:58 UTC
//
*/

#ifndef INCLUDED_CLANG_DOC_H
#define INCLUDED_CLANG_DOC_H

#include "clang-c/Index.h"
#include "Html_File.h"

#include <set>
#include <map>
#include <string>
#include <vector>

namespace clang_doc {

class Clang_Doc {
public:
  Clang_Doc(int argc,
            char* argv[],
            const std::set<std::string>& files,
            const std::string& object_dir,
            const std::string& html_dir,
            const std::string& prefix);

  ~Clang_Doc(void);

  const char* object_dir(void) const {return object_dir_.c_str();}
  const char* html_dir(void) const {return html_dir_.c_str();}
  const char* prefix(void) const {return prefix_.c_str();}

  void generate_symbol_table(const std::set<std::string>& tag_files);
  void generate_html_files(const std::string& tag_file);

  CXChildVisitResult visitor(CXCursor cursor,
                             CXCursor parent,
                             CXClientData client_data);

private:

  void add_symbols(const std::set<std::string>& tags);
  void generate_tag_file(const std::string& tag_file);
  void parse_include_directives (void);

  int argc_;
  char** argv_;
  std::string object_dir_;
  std::string html_dir_;
  std::string prefix_;

  CXIndex idx_;
  const std::set<std::string> files_;
  std::set<std::string> other_files_;
  std::map<std::string, Definition> defmap_;
  std::vector<std::string> includes_;
};

} // clang_doc

#endif /* INCLUDED_CLANG_DOC_H */
