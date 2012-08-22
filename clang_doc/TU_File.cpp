/* -*- Mode: C++ -*-
//
// \file: TU_File.cpp
//
// \author: Don Allon Hinton <don.hinton@gmx.com>
// \date: 13 Aug 2012 17:54:34 UTC
//
*/

#include "TU_File.h"
#include "Utils.h"

#include <sys/stat.h>
#include <iostream>

namespace clang_doc {

namespace {

// Diagnostic functions taken from c-index-test.c

void PrintExtent(FILE *out, unsigned begin_line, unsigned begin_column,
                 unsigned end_line, unsigned end_column) {
  fprintf(out, "[%d:%d - %d:%d]", begin_line, begin_column,
          end_line, end_column);
}

void PrintDiagnostic(CXDiagnostic Diagnostic) {
  FILE *out = stderr;
  CXFile file;
  CXString Msg;
  unsigned display_opts = CXDiagnostic_DisplaySourceLocation
    | CXDiagnostic_DisplayColumn | CXDiagnostic_DisplaySourceRanges
    | CXDiagnostic_DisplayOption;
  unsigned i, num_fixits;

  if (clang_getDiagnosticSeverity(Diagnostic) == CXDiagnostic_Ignored)
    return;

  Msg = clang_formatDiagnostic(Diagnostic, display_opts);
  fprintf(stderr, "%s\n", clang_getCString(Msg));
  clang_disposeString(Msg);

  clang_getSpellingLocation(clang_getDiagnosticLocation(Diagnostic),
                            &file, 0, 0, 0);
  if (!file)
    return;

#if 1
  num_fixits = clang_getDiagnosticNumFixIts(Diagnostic);
  fprintf(stderr, "Number FIX-ITs = %d\n", num_fixits);
  for (i = 0; i != num_fixits; ++i) {
    CXSourceRange range;
    CXString insertion_text = clang_getDiagnosticFixIt(Diagnostic, i, &range);
    CXSourceLocation start = clang_getRangeStart(range);
    CXSourceLocation end = clang_getRangeEnd(range);
    unsigned start_line, start_column, end_line, end_column;
    CXFile start_file, end_file;
    clang_getSpellingLocation(start, &start_file, &start_line,
                              &start_column, 0);
    clang_getSpellingLocation(end, &end_file, &end_line, &end_column, 0);
    if (clang_equalLocations(start, end)) {
      /* Insertion. */
      if (start_file == file)
        fprintf(out, "FIX-IT: Insert \"%s\" at %d:%d\n",
                clang_getCString(insertion_text), start_line, start_column);
    } else if (strcmp(clang_getCString(insertion_text), "") == 0) {
      /* Removal. */
      if (start_file == file && end_file == file) {
        fprintf(out, "FIX-IT: Remove ");
        PrintExtent(out, start_line, start_column, end_line, end_column);
        fprintf(out, "\n");
      }
    } else {
      /* Replacement. */
      if (start_file == end_file) {
        fprintf(out, "FIX-IT: Replace ");
        PrintExtent(out, start_line, start_column, end_line, end_column);
        fprintf(out, " with \"%s\"\n", clang_getCString(insertion_text));
      }
      break;
    }
    clang_disposeString(insertion_text);
  }
#endif
}

void PrintDiagnosticSet(CXDiagnosticSet Set) {
  int i = 0, n = clang_getNumDiagnosticsInSet(Set);
  for ( ; i != n ; ++i) {
    CXDiagnostic Diag = clang_getDiagnosticInSet(Set, i);
    CXDiagnosticSet ChildDiags = clang_getChildDiagnostics(Diag);
    PrintDiagnostic(Diag);
    if (ChildDiags)
      PrintDiagnosticSet(ChildDiags);
  }
}

void PrintDiagnostics(CXTranslationUnit TU) {
  CXDiagnosticSet TUSet = clang_getDiagnosticSetFromTU(TU);
  PrintDiagnosticSet(TUSet);
  clang_disposeDiagnosticSet(TUSet);
}

} // annonymous namespace


TU_File::TU_File(int argc,
                 char* argv[],
                 CXIndex idx,
                 const std::string& source_filename,
                 const std::string& object_dir,
                 const std::string& prefix,
                 bool reparse)
  : idx_(idx),
    tu_(0),
    argc_(argc),
    argv_(argv),
    source_filename_(source_filename),
    length_(0),
    reparse_ (reparse) {
  object_dir_ = strip_final_seps(object_dir);
  prefix_ = strip_final_seps(prefix);
  tu_filename_ = make_filename(source_filename_, object_dir_, prefix_, ".tu");

  struct stat st;
  if (stat(source_filename_.c_str(), &st) == 0)
    length_ = st.st_size;

  load_tu();
}

TU_File::~TU_File(void) {
  if (tu_) {
    PrintDiagnostics(tu_);
    clang_disposeTranslationUnit(tu_);
  }
}

void
TU_File::load_tu(void) {
  struct stat st;

  if (!reparse_ && stat(tu_filename_.c_str(), &st) == 0) {
    std::cout << "found tu file: " << tu_filename_.c_str() << std::endl;
    // Note that this will crash in 3.1 if the any of the source files have
    // changed and it has to be regenerated.  I think the head works though.
    // The solution right now is to delete the files from a previous run if
    // any have changed, i.e., during development.
    tu_ = clang_createTranslationUnit(idx_, tu_filename_.c_str());
    return;
  }
  std::cout << "parsing file: " << source_filename_.c_str() << std::endl;

  tu_ = clang_parseTranslationUnit(idx_,
                                   source_filename_.c_str(),
                                   argv_,
                                   argc_,
                                   0,
                                   0,
                                   clang_defaultEditingTranslationUnitOptions());//0);
  if (tu_)
  {
    int ret = clang_saveTranslationUnit(tu_,
                                        tu_filename_.c_str(),
                                        clang_defaultSaveOptions(tu_));

    if (ret == CXSaveError_None)
    {
      // what should we do here?
    }
  }
}

} // clang_doc
