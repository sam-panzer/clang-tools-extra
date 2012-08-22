/* -*- Mode: C++ -*-
//
// \file: Clang_Doc.cpp
//
// \author: Don Allon Hinton <don.hinton@gmx.com>
// \date: 13 Aug 2012 19:20:11 UTC
//
*/

#include "Clang_Doc.h"
#include "TU_File.h"
#include "Utils.h"

#include <iostream>
#include <libgen.h>

namespace clang_doc {

namespace {

struct Visitor_Data{
  Clang_Doc* doc;
  CXFile file;
  const char* filename;
};

CXChildVisitResult
visitor_c(CXCursor cursor, CXCursor parent, CXClientData client_data) {
  Clang_Doc* d = (static_cast<Visitor_Data*>(client_data))->doc;
  return d->visitor(cursor, parent, client_data);
}

} // anonymous namespace

CXChildVisitResult
Clang_Doc::visitor(CXCursor cursor, CXCursor parent, CXClientData client_data) {
  CXSourceLocation loc = clang_getCursorLocation(cursor);
  CXFile cxfile;
  unsigned line;
  unsigned column;
  unsigned offset;
  clang_getExpansionLocation(loc, &cxfile, &line, &column, &offset);

  Visitor_Data* vd = static_cast<Visitor_Data*>(client_data);

  // we're just looking for definitions in this file, so
  // skip included files for now
  if (cxfile != vd->file)
    return CXChildVisit_Continue;


#if 0

  CXString cxstrfilename = clang_getFileName(cxfile);
  const char* sfilename = clang_getCString(cxstrfilename);
  CXCursorKind kindx = clang_getCursorKind(cursor);
  std::string strfullyscopednamex = fullyScopedName(cursor);
  unsigned declx = clang_isDeclaration(cursor.kind);
  unsigned defx = clang_isCursorDefinition(cursor);
  CXString cxspelling = clang_getCursorSpelling(cursor);
  CXString cxdisplayname = clang_getCursorDisplayName(cursor);
  CXString cxkind = clang_getCursorKindSpelling(cursor.kind);
  const char* sdisplayname = clang_getCString(cxdisplayname);
  const char* sspelling = clang_getCString(cxspelling);
  const char* skind = clang_getCString(cxkind);

  bool global = false;

  CXString cxusrx = clang_getCursorUSR(cursor);
  const char* susrx = clang_getCString(cxusrx);

  std::cout << "visitor: " << STR(sfilename) << ":" << line << ":" << column;
  std::cout << ": kind = " << kindx << " : " << STR (skind);
  std::cout << " (" << (global?"global":"local");
  std::cout << (defx?" definition) ":(declx?" declaration) ":") "));
  std::cout << STR(sspelling) << " \"" << STR(sdisplayname) << "\" ";
  std::cout << strfullyscopednamex.c_str();
  std::cout << " -- " << STR (susrx) << "\n";

  clang_disposeString(cxusrx);
  clang_disposeString(cxstrfilename);
  clang_disposeString(cxdisplayname);
  clang_disposeString(cxspelling);
  clang_disposeString(cxkind);
#endif

  if (clang_isDeclaration(cursor.kind)) {
    if (clang_isCursorDefinition(cursor)) {
      CXString cxusr = clang_getCursorUSR(cursor);
      std::string susr = clang_getCString(cxusr);
      // external linkage begins with "c:@", but not @aN, which is an anonymous
      // namespace otherwise, it would be "c:somefile.cpp@..."
      if (susr[2] == '@' && !(susr.substr(2,3) == "@aN")) {
        Definition def;
        // can't use usr since you can only generate it for the defs, not declarations.
        def.key = fullyScopedName(cursor);
        def.file = vd->filename;
        def.line = line;
        def.column = column;
        def.offset = offset;
        def.from_tag_file = false;
        defmap_.insert(std::pair<std::string, Definition>(def.key, def));
      }
      clang_disposeString(cxusr);
    }
  }

  return CXChildVisit_Recurse;
}

Clang_Doc::Clang_Doc(int argc,
                     char* argv[],
                     const std::set<std::string>& files,
                     const std::string& object_dir,
                     const std::string& html_dir,
                     const std::string& prefix)
  : argc_(argc),
    argv_(argv),
    files_ (files) {

  object_dir_ = strip_final_seps(object_dir);
  html_dir_ = strip_final_seps(html_dir);
  prefix_ = strip_final_seps(prefix);

  parse_include_directives();
  idx_ = clang_createIndex(0, 0);
}

Clang_Doc::~Clang_Doc(void) {
  clang_disposeIndex(idx_);
}

void
Clang_Doc::add_symbols(const std::set<std::string>& tags) {
  //std::cout << "Clang_Doc::add_symbols\n";

  std::cout << "tag files:\n";
  for (std::set<std::string>::iterator i = tags.begin(),
         e = tags.end(); i != e; ++i) {
    FILE* f = fopen((*i).c_str(), "r");
    if (f) {
      // scan file and add items to def_map_
      char symbol[1024] = {0};
      char file[1024] = {0};
      unsigned line;
      int result;
      while ((result = fscanf(f, "%s %s %u\n", symbol, file, &line)) != EOF) {
        Definition def;
        def.key = symbol;
        def.file = file;
        def.html_path = dirname((*i).c_str());
        def.line = line;
        def.from_tag_file = true;

        defmap_.insert(std::pair<std::string, Definition>(def.key, def));
      }
      fclose(f);
      std::cout << (*i).c_str() << "\n";
    }
  }
  std::cout << std::endl;
}

void
Clang_Doc::generate_symbol_table(const std::set<std::string>& tags) {
  //std::cout << "Clang_Doc::generate_symbol_table\n";

  add_symbols (tags);

  for (std::set<std::string>::const_iterator i = files_.begin(),
         e = files_.end(); i != e; ++i) {
    TU_File tu_file = TU_File (argc_, argv_, idx_, (*i), object_dir_, prefix_, true);
    CXTranslationUnit tu = tu_file.tu();
    if (!tu) {
      std::cerr << "error: failed to parse \"" << (*i).c_str() << "\"\n";
      continue;
    }

    CXFile file = clang_getFile(tu, (*i).c_str());

    Visitor_Data vd;
    vd.doc = this;
    vd.file = file;
    vd.filename = (*i).c_str();

    CXCursor c = clang_getTranslationUnitCursor(tu);
    clang_visitChildren(c, visitor_c, &vd);
  }

#if 0
  std::cout << "\n\nList of definition with external linkage\n";

  for (std::map<std::string, Definition>::iterator i = defmap_.begin(),
         e = defmap_.end(); i != e; ++i) {
    std::cout << (*i).second.file.c_str() << ":" << (*i).second.line;
    std::cout << ":" << (*i).second.column << ":   " << (*i).second.key.c_str();
    std::cout << "\n";
  }
  std::cout << std::endl;
#endif
}

void
Clang_Doc::generate_tag_file(const std::string& tag_file) {
  FILE* f = fopen(tag_file.c_str(), "w");
  if (f) {
    for (std::set<std::string>::const_iterator ci = files_.begin(),
           ce = files_.end(); ci != ce; ++ci) {
      std::string file = make_filename((*ci), html_dir_, prefix_, ".html", true);
      fprintf(f, "%s %s %u\n", (*ci).c_str(), file.c_str(), 0);
    }
    for (std::map<std::string, Definition>::iterator i = defmap_.begin(),
           e = defmap_.end(); i != e; ++i) {
      Definition& d = (*i).second;
      if (!d.key.empty() && d.from_tag_file == false) {
        fprintf(f, "%s %s %u\n", d.key.c_str(), d.file.c_str(), d.line);
      }
    }
    fclose(f);
  }
  else
    std::cerr << "error creating tag file: " << tag_file.c_str() << "\n";
}

void
Clang_Doc::generate_html_files(const std::string& tag_file) {
  for (std::set<std::string>::const_iterator i = files_.begin(),
         e = files_.end();i != e; ++i) {
    Html_File html_file =
      Html_File(argc_, argv_, idx_, includes_, files_, defmap_,
                (*i), object_dir_, html_dir_, prefix_);
    html_file.create_file();
  }
  generate_tag_file(tag_file);
}

void
Clang_Doc::parse_include_directives (void) {
  //std::cout << "parse_include_directives\n";

  bool getnext = false;
  bool found = false;
  int index = 0;
  for (int i = 0; i < argc_; ++i) {
    char* p = argv_[i];
    if (getnext) {
      found = true;
      getnext = false;
    } else {
      if (p[0] == '-' && p[1] == 'I') {
        if (p[2] == 0) {
          index = 0;
          getnext = true;
          continue;
        }
        index = 2;
        found = true;
      }
    }

    if (found) {
      found = false;
      includes_.push_back (p+index);
    }
  }

  std::cout << "include paths:\n";
  for (std::vector<std::string>::iterator i = includes_.begin(),
         e = includes_.end(); i != e; ++i)
    std::cout << (*i).c_str() << "\n";
  std::cout << std::endl;
}

} // clang_doc
