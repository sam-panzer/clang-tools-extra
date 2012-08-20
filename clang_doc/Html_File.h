/* -*- Mode: C++ -*-
//
// \file: Html_File.h
//
// \author: Don Allon Hinton <don.hinton@gmx.com>
// \date: 13 Aug 2012 15:11:44 UTC
//
*/

#ifndef INCLUDED_HTML_FILE_H
#define INCLUDED_HTML_FILE_H

#include <clang-c/Index.h>

#include <string>
#include <set>
#include <map>
#include <vector>

namespace clang_doc {

class TU_File;

struct Definition {
  std::string key;
  std::string file;
  std::string html_path;
  unsigned line;
  unsigned column;
  unsigned offset;
  bool from_tag_file;
};

class Html_File {
public:
  Html_File(int argc,
            char* argv[],
            CXIndex ctx,
            const std::vector<std::string>& includes,
            const std::set<std::string>& files,
            std::map<std::string, Definition>& defmap,
            const std::string& source_filename,
            const std::string& object_dir,
            const std::string& html_dir,
            const std::string& prefix);
  ~Html_File(void);

  const char* source_filename(void) const {return source_filename_.c_str();}
  const char* html_dir(void) const {return html_dir_.c_str();}
  const char* prefix(void) const {return prefix_.c_str();}
  const char* html_filename(void) const {return html_filename_.c_str();}

  void create_file(void);

private:
  void write_header(FILE* f);
  const std::string fix(const char* s);
  void write_token(FILE* f, CXFile file, CXToken tok,
                   const char* str, unsigned line, unsigned column);
  void write_comment_split(FILE* f, CXFile file, CXToken tok);
  void write_html(void);

private:
  int argc_;
  char** argv_;
  CXIndex idx_;
  TU_File* tu_file_;
  unsigned cur_line_;
  unsigned cur_column_;

  const std::vector<std::string> includes_;
  const std::set<std::string>& files_;
  std::map<std::string, Definition>& defmap_;

  std::string source_filename_;
  std::string object_dir_;
  std::string html_dir_;
  std::string prefix_;
  std::string html_filename_;
};

} // clang_doc

#endif /* INCLUDED_HTML_FILE_H */
