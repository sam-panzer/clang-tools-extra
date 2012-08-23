/* -*- Mode: C++ -*-
//
// \file: Html_File.cpp
//
// \author: Don Allon Hinton <don.hinton@gmx.com>
// \date: 13 Aug 2012 15:11:35 UTC
//
*/

#include "Html_File.h"
#include "TU_File.h"
#include "Utils.h"

#include <iostream>
#include <sys/stat.h>
#include <libgen.h>
#include <sys/param.h>
#include <stdlib.h>

namespace clang_doc {

Html_File::Html_File(int argc,
                     char* argv[],
                     CXIndex idx,
                     const std::vector<std::string>& includes,
                     const std::set<std::string>& files,
                     std::map<std::string, Definition>& defmap,
                     const std::string& source_filename,
                     const std::string& object_dir,
                     const std::string& html_dir,
                     const std::string& prefix)
  : argc_(argc),
    argv_(argv),
    idx_(idx),
    tu_file_(0),
    includes_ (includes),
    files_(files),
    defmap_(defmap),
    source_filename_(source_filename) {
  object_dir_ = strip_final_seps(object_dir);
  html_dir_ = strip_final_seps(html_dir);
  prefix_ = strip_final_seps(prefix);
  html_filename_ = make_filename(source_filename_, html_dir_, prefix_, ".html");
}

Html_File::~Html_File(void) {
  delete(tu_file_);
}

void
Html_File::write_header(FILE* f) {
  fprintf (f, "<html><head>\n");
  fprintf (f, "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=iso-8859-1\"/>");
  fprintf (f, "<meta name=\"keywords\" content=\"clang,clang_doc, C, C++\"/>");
  fprintf (f, "<meta name=\"description\" content=\"C++ source code API documentation for clang.\"/>");
  fprintf (f, "<title>clang: %s Source File</title>", source_filename_.c_str());
  fprintf (f, "<link href=\"doxygen.css\" rel=\"stylesheet\" type=\"text/css\"/>");
  fprintf (f, "</head><body>");
  fprintf (f, "<p class=\"title\">clang Code Documentation</p>");
  fprintf (f, "<div class=\"navigation\" id=\"top\">");
  fprintf (f, "  <div class=\"tabs\">");
  fprintf (f, "    <ul>");
  fprintf (f, "      <li><a href=\"index.html\"><span>Main&nbsp;Page</span></a></li>");
  fprintf (f, "    </ul>");
  fprintf (f, "  </div>");
  fprintf (f, "</div>");
  fprintf (f, "<div class=\"contents\">");
  fprintf (f, "<h1>%s</h1>", source_filename_.c_str());
  fprintf (f, "<div class=\"fragment\">");
  fprintf (f, "<pre class=\"fragment\">");
}

const std::string
Html_File::fix(const char* s) {
  const char* p = s;
  static std::string str;
  str = "";
  while (*p) {
    switch (*p) {
    case ('<'):
      str += "&lt;";
      break;
    case ('>'):
      str += "&gt;";
      break;
    case ('&'):
      str += "&amp;";
      break;
    case ('"'):
      str += "&quot;";
      break;
    default:
      str.push_back (*p);
      break;
    }
    ++p;
  }
  return str;
}

namespace {
std::string
munge_fullyscopedname(const std::string& str) {
  // strip out the "class@" instances and convert "_Bool@" to "bool@"
  std::string result = str;
  size_t pos = result.find("class@");
  while (pos != std::string::npos) {
    result.replace (pos, 6,"");
    pos = result.find("class@");
  }

  pos = result.find("enum@");
  while (pos != std::string::npos) {
    result.replace (pos, 5,"");
    pos = result.find("enum@");
  }

#if 0
  pos = result.find("struct@");
  while (pos != std::string::npos) {
    result.replace (pos, 7,"");
    pos = result.find("struct@");
  }
#endif

  pos = result.find("@_Bool");
  while (pos != std::string::npos) {
    result.replace (pos, 6,"@bool");
    pos = result.find("@_Bool");
  }
  return result;
}
} // anonymous namespace

void
Html_File::write_token(FILE* f,
                       CXFile file,
                       CXToken tok,
                       const char* str,
                       unsigned line,
                       unsigned column)
{
  static bool preprocessor = false;
  static bool include = false;

  CXSourceLocation tloc = clang_getTokenLocation(tu_file_->tu(), tok);
  CXCursor c = clang_getCursor(tu_file_->tu(), tloc);

  CXCursorKind curkind = clang_getCursorKind(c);

  if (cur_line_ <= line) cur_column_ = 1;

  for (; cur_line_ <= line; ++cur_line_)
    fprintf (f, "\n<a name=\"l%05i\"></a>%05i", cur_line_, cur_line_);

  for (; cur_column_ <= column; ++cur_column_)
    fprintf (f , " ");

  switch (clang_getTokenKind(tok)) {
  case (CXToken_Punctuation):
    if (strcmp(str, "#") == 0)
      preprocessor = true;
    fprintf(f, "%s", str);
    break;
  case (CXToken_Keyword):
    fprintf(f, "<span class=\"keyword\">%s</span>", str);
    break;
  case (CXToken_Comment):
    fprintf(f, "<span class=\"comment\">%s</span>", str);
    break;
  case (CXToken_Literal): {
    //include = false; // disable include links for now
    if (include) {
      include = false;
      // found an include file
      std::string t;
      const char* p = str;
      while (*p) {
        if (*p != '"')
          t += *p;
        ++p;
      }

      // first, use this file's path, then all the include paths

      bool found_include = false;
      char path[PATH_MAX];
      std::string includefile = realpath(dirname(tu_file_->source_filename()), path);
      includefile += "/" + t;
      struct stat st;
      if (stat(includefile.c_str(), &st) == 0) {
        found_include = true;
      } else {
        for (std::vector<std::string>::const_iterator i = includes_.begin(),
               e = includes_.end(); i != e; ++i) {
          char path[PATH_MAX];

          includefile = realpath((*i).c_str(), path);
          includefile += "/" + t;
          if (stat(includefile.c_str(), &st) == 0) {
            found_include = true;
            break;
          }
        }
      }
      if (found_include) {
        if (files_.find(includefile) != files_.end()) {
          t = make_filename(includefile, html_dir_, prefix_, ".html", false);
          fprintf(f, "<a class=\"code\" href=\"%s\" title="">%s</a>",
                  t.c_str(), str);
          break;
        }
        std::map<std::string, Definition>::iterator i = defmap_.find(includefile);
        if (i != defmap_.end()) {
          t = i->second.file.c_str();
          fprintf(f, "<a class=\"code\" href=\"%s\" title="">%s</a>",
                  t.c_str(), str);
          break;
        }
      }
    }
    // not an include or include not found
    std::string s = fix(str);
    fprintf(f, "%s",  s.c_str() );
    break;
  }
  case (CXToken_Identifier): {
    if (preprocessor) {
      preprocessor = false;
      if (strcmp(str, "include") == 0)
        include = true;
      fprintf(f, "<span class=\"code\">%s</span>", str);
      break;
    }

    CXCursor ref = clang_getCursorReferenced(c);
    std::string fsn = munge_fullyscopedname(fullyScopedName(ref));

    // FIXME: Generalize debuggin
    if (false && (line == 2601 || line == 737))
    {
      std::cout << str << " : " << fsn.c_str() << std::endl;
    }

    // Calling clang_getCursorDefinition() does not work properly
    // for template classes, i.e., it will find the method
    // declaration, not the definition, if they differ.  However,
    // once you have the declaration's location, you can use it
    // get that cursor, and find the definition that way.
    CXSourceLocation decloc =
      clang_getCursorLocation(clang_getCursorDefinition(c));
    CXCursor cref =
      clang_getCursorDefinition(clang_getCursor(tu_file_->tu(),
                                                decloc));

    std::string rfile;
    std::string html_dir;
    unsigned refl = line;
    bool found = false;

    if (!clang_Cursor_isNull(cref) && curkind != CXCursor_Namespace) {
      CXSourceLocation refloc = clang_getCursorLocation(cref);
      if (!clang_equalLocations(tloc, refloc)) {
        CXFile cxfile;
        unsigned col;
        unsigned off;
        clang_getExpansionLocation(refloc, &cxfile, &refl, &col, &off);
        if (cxfile == file)
          found = true;
        else {
          CXString cxfn = clang_getFileName(cxfile);
          const char* fn = clang_getCString(cxfn);
          if (fn) {
            if (files_.find(fn) != files_.end()) {
              rfile = fn;
              found = true;
            }
          }
          clang_disposeString(cxfn);
        }
      }
    }
    else if (!clang_isDeclaration(curkind) && !fsn.empty()) {
      std::map<std::string, Definition>::iterator r = defmap_.find(fsn);
      if (r != defmap_.end()) {
        found = true;
        rfile = r->second.file.c_str();
        html_dir = r->second.html_path.c_str();
        refl = r->second.line;
      }
    }

    // since we are linking to lines, no need to link to same line
    if (found && (!rfile.empty() || refl != line)) {
      if (!rfile.empty())
        rfile = make_filename(rfile, html_dir, prefix_, ".html", !html_dir.empty());
      fprintf(f, "<a class=\"code\" href=\"%s#l%05i\" title="">%s</a>",
              rfile.c_str(), refl , str);
      break;
    }
    fprintf(f, "<span class=\"code\">%s</span>", str);
    break;
  }
  }
  cur_column_ += strlen(str);
}

// FIXME:  change this to just printing comments, and call write_token()
//         directly from write_html() for non-comments.
void
Html_File::write_comment_split(FILE* f, CXFile file, CXToken tok) {
  unsigned line;
  unsigned column;
  unsigned offset;

  CXSourceLocation loc = clang_getTokenLocation(tu_file_->tu(), tok);
  clang_getExpansionLocation(loc, &file, &line, &column, &offset);

  CXTokenKind kind = clang_getTokenKind(tok);
  CXString s = clang_getTokenSpelling(tu_file_->tu(), tok);

  std::string sub = clang_getCString(s);
  clang_disposeString(s);

  // actually split up multi-line comments and send one at
  // a time -- that way each line gets line numbers.
  if (kind == CXToken_Comment || kind == CXToken_Literal) {
    std::string str = sub.c_str();
    if (kind == CXToken_Comment)
      str = fix(sub.c_str());
    size_t i;
    size_t begin = 0;
    for (i = 0; i < str.length(); ++i) {
      if (str[i] == '\n') {
        sub = str.substr(begin, i-begin);
        if (begin)
          line++; column = 1;
        begin = i + 1;
        write_token(f, file, tok, sub.c_str(), line, column);
      }
    }
    if (begin) {
      line++;
      column = 1;
    }
    sub = str.substr(begin, i-begin);
  }
  write_token(f, file, tok, sub.c_str(), line, column);
}

void
Html_File::write_html(void) {
  cur_line_ = 1;
  cur_column_ = 1;

  CXFile file = clang_getFile(tu_file_->tu(), source_filename_.c_str());
  CXSourceRange range
    = clang_getRange(clang_getLocationForOffset(tu_file_->tu(), file, 0),
                     clang_getLocationForOffset(tu_file_->tu(),
                                                file, tu_file_->length()));

  CXToken *tokens;
  unsigned num;
  clang_tokenize(tu_file_->tu(), range, &tokens, &num);

  FILE* f = fopen(html_filename_.c_str(), "w");
  if (f) {
    write_header(f);

    for (unsigned i = 0; i < num; ++i)
      write_comment_split(f, file, tokens[i]);

    fprintf(f, "</pre></div></div></body></html>");
    fclose(f);
  }
  else
    std::cerr << "error: could not create file: " << html_filename_.c_str() << "\n";

  clang_disposeTokens(tu_file_->tu(), tokens, num);
}

void
Html_File::create_file(void) {
  if (!tu_file_)
    tu_file_ = new TU_File(argc_, argv_, idx_, source_filename_, object_dir_, prefix_);

  write_html();
}

} // clang_doc
