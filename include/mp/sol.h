/*
 .sol format support.

 .sol is a format for representing solutions of mathematical optimization
 problems. It is described in the technical report "Hooking Your Solver
 to AMPL" (http://www.ampl.com/REFS/hooking2.pdf).

 Copyright (C) 2014 AMPL Optimization Inc

 Permission to use, copy, modify, and distribute this software and its
 documentation for any purpose and without fee is hereby granted,
 provided that the above copyright notice appear in all copies and that
 both that the copyright notice and this permission notice and warranty
 disclaimer appear in supporting documentation.

 The author and AMPL Optimization Inc disclaim all warranties with
 regard to this software, including all implied warranties of
 merchantability and fitness.  In no event shall the author be liable
 for any special, indirect or consequential damages or any damages
 whatsoever resulting from loss of use, data or profits, whether in an
 action of contract, negligence or other tortious action, arising out
 of or in connection with the use or performance of this software.

 Author: Victor Zverovich
 */

#ifndef MP_SOL_H_
#define MP_SOL_H_

#include <cstdio>
#include <cstring>

#include "mp/posix.h"
#include "mp/problem-base.h"

namespace mp {

namespace internal {

void WriteMessage(fmt::BufferedFile &file, const char *message);

// Suffix value visitor that counts values.
class SuffixValueCounter {
 private:
  int num_values_;

 public:
  SuffixValueCounter() : num_values_(0) {}

  int num_values() const { return num_values_; }

  template <typename T>
  void Visit(int, T) { ++num_values_; }
};

// Suffix value visitor that writes values to a file.
class SuffixValueWriter {
 private:
  fmt::BufferedFile &file_;

 public:
  explicit SuffixValueWriter(fmt::BufferedFile &file) : file_(file) {}

  template <typename T>
  void Visit(int index, T value) { file_.print("{} {}\n", index, value); }
};

template <typename SuffixMap>
void WriteSuffixes(fmt::BufferedFile &file, const SuffixMap &suffixes) {
  for (typename SuffixMap::iterator
       i = suffixes.begin(), e = suffixes.end(); i != e; ++i) {
    const char *name = i->name();
    namespace suf = mp::suf;
    SuffixValueCounter counter;
    i->VisitValues(counter);
    file.print("suffix {} {} {} {} {}\n{}\n",
               i->kind() & (suf::MASK | suf::FLOAT | suf::IODECL),
               counter.num_values(), std::strlen(name) + 1, 0, 0, name);
    // TODO: write table
    SuffixValueWriter writer(file);
    i->VisitValues(writer);
  }
}
}  // namespace internal

// Writes a solution to a .sol file.
template <typename Solution>
void WriteSolFile(fmt::StringRef filename, const Solution &sol) {
  fmt::BufferedFile file(filename, "w");
  internal::WriteMessage(file, sol.message());
  // Write options.
  if (int num_options = sol.num_options()) {
    file.print("{}\n", num_options);
    for (int i = 0; i < num_options; ++i)
      file.print("{}\n", sol.option(i));
  }
  // TODO: check precision
  for (int i = 0, n = sol.num_values(); i < n; ++i)
    file.print("{}\n", sol.value(i));
  for (int i = 0, n = sol.num_dual_values(); i < n; ++i)
    file.print("{}\n", sol.dual_value(i));
  file.print("objno 0 0\n"); // TODO: solve codes for objectives
  //for (int suf_kind = 0; suf_kind < suf::NUM_KINDS; ++suf_kind)
    //internal::WriteSuffixes(file, sol.suffixes(suf_kind));
  // TODO: test
}
}  // namepace mp

#endif  // MP_SOL_H_
