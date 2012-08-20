/* -*- Mode: C++ -*-
//
// \file: Utils.cpp
//
// \author: Don Allon Hinton <don.hinton@gmx.com>
// \date: 16 Aug 2012 09:53:06 UTC
//
*/

#include "Utils.h"
#include <regex.h>

#include <iostream>

namespace clang_doc {

namespace {

std::string
fullyScopedName_i (const CXCursor& cursor) {
  std::string str;

  CXCursor parent = clang_getCursorSemanticParent(cursor);
  if (!clang_Cursor_isNull(parent) && parent.kind != CXCursor_TranslationUnit)
    str += fullyScopedName(parent);

  CXString cxs = clang_getCursorDisplayName(cursor);
  const char* s = clang_getCString(cxs);

  if (s[0] != 0) {
    if (str[0] != 0) // prevent initial "::"
      str += "::";
    str += s;
  }

  clang_disposeString(cxs);
  return str;
}

} // anonomous namespace

std::string
fullyScopedName (const CXCursor& cursor) {
  std::string str = fullyScopedName_i (cursor);

  size_t pos = str.find_first_of (' ');
  while (pos != std::string::npos) {
    str[pos] = '@';
    pos = str.find_first_of(' ', pos+1);
  }
  return str;
}

const std::string
make_filename(const std::string& file,
              const std::string& directory,
              const std::string& prefix,
              const std::string& suffix,
              bool full_path) {
  //std::cout << "make_filename: " << file.c_str() << std::endl;

#if 1
  // FIXME: use regex to strip off common root
  int len = prefix.length();
  if (len>0) ++len; // add separator
  std::string sub = file.substr(len) + suffix;
#else
  std::string sub = file + suffix;
#endif

  size_t pos = sub.find_first_of ('/');
  while (pos != std::string::npos) {
    sub[pos] = '_';
    pos = sub.find_first_of('/', pos+1);
  }

  std::string out_file;
  if (full_path)
    out_file += directory + "/";

  out_file += sub;

  return out_file;
}

const std::string
strip_final_seps (const std::string& str) {
  size_t last = str.length();
  while (str[last-1] == '/') --last;
  return str.substr (0, last);
}

} // clang_doc
