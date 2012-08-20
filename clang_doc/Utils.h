/* -*- Mode: C++ -*-
//
// \file: Utils.h
//
// \author: Don Allon Hinton <don.hinton@gmx.com>
// \date: 16 Aug 2012 09:55:34 UTC
//
//
*/

#ifndef INCLUDED_UTILS_H
#define INCLUDED_UTILS_H

#include "clang-c/Index.h"
#include <string>

#define STR(x) (x?x:"xxxxxxxxxxxxxxxxxx")

namespace clang_doc {

std::string
fullyScopedName(const CXCursor& cursor);

const std::string
make_filename(const std::string& file,
              const std::string& html_dir,
              const std::string& prefix,
              const std::string& suffix,
              bool full_path = true);

const std::string
strip_final_seps(const std::string& str);

} // clang_doc

#endif /* INCLUDED_UTILS_H */
