/* -*- Mode: C++ -*-
//
// \file: TU_File.h
//
// \author: Don Allon Hinton <don.hinton@gmx.com>
// \date: 13 Aug 2012 17:54:25 UTC
//
*/

#ifndef INCLUDED_TU_FILE_H
#define INCLUDED_TU_FILE_H

#include "clang-c/Index.h"
#include <string>

namespace clang_doc {

class TU_File {
public:
  TU_File(int argc,
          char* argv[],
          CXIndex idx,
          const std::string& source_filename,
          const std::string& object_dir,
          const std::string& prefix,
          bool reparse = false);

  ~TU_File(void);

  const char* source_filename(void) const {return source_filename_.c_str();}
  const char* object_dir(void) const {return object_dir_.c_str();}
  const char* prefix(void) const {return prefix_.c_str();}
  const char* tu_filename(void) const {return tu_filename_.c_str();}

  CXTranslationUnit tu(void) const {return tu_;}
  unsigned length(void) const {return length_;}

protected:

  void load_tu(void);

private:

  CXIndex idx_;
  CXTranslationUnit tu_;

  int argc_;
  char** argv_;

  std::string source_filename_;
  std::string object_dir_;
  std::string prefix_;
  std::string tu_filename_;

  unsigned length_;
  bool reparse_;
};

} // clang_doc

#endif /* INCLUDED_TU_FILE_H */
