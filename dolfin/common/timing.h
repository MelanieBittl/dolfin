// Copyright (C) 2005-2010 Anders Logg
//
// This file is part of DOLFIN.
//
// DOLFIN is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// DOLFIN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with DOLFIN. If not, see <http://www.gnu.org/licenses/>.
//
// First added:  2005-12-21
// Last changed: 2012-11-01

#ifndef __TIMING_H
#define __TIMING_H

#include <string>
#include <cstdint>
#include <dolfin/log/Table.h>

namespace dolfin
{
  /// Parameter specifying whthert to clear timing(s)
  enum class TimingClear : bool { keep = false, clear = true };

  /// Timing type: wall-clock time, user (cpu) time, system (kernel) time.
  /// Precision of wall is around 1 microsecond, user and system are around
  /// 10 millisecond (on Linux).
  enum class TimingType : int32_t { wall = 0, user = 1, system = 2 };

  /// Start timing (should not be used internally in DOLFIN!)
  void tic();

  /// Return elapsed wall time (should not be used internally in DOLFIN!)
  double toc();

  /// Return wall time elapsed since some implementation dependent epoch
  double time();

  /// Return a summary of timings and tasks in a Table, optionally clearing
  /// stored timings
  Table timings(TimingClear clear, std::set<TimingType> type);

  /// DEPRECATED: List a summary of timings and tasks, optionally clearing
  /// stored timings. MPI_AVG reduction is printed. Collective on
  /// MPI_COMM_WORLD. Only wall time is printed.
  void list_timings(bool reset=false);

  /// List a summary of timings and tasks, optionally clearing stored
  /// timings. MPI_AVG reduction is printed. Collective on MPI_COMM_WORLD.
  void list_timings(TimingClear clear, std::set<TimingType> type);

  /// Dump a summary of timings and tasks to XML file, optionally clearing
  /// stored timings. MPI_MAX, MPI_MIN and MPI_AVG reductions are stored.
  /// Collective on MPI_COMM_WORLD.
  void dump_timings_to_xml(std::string filename, TimingClear clear);

  /// Return timing (count, total wall time, total user time,
  /// total system time) for given task, optionally clearing
  /// all timings for the task
  std::tuple<std::size_t, double, double, double>
    timing(std::string task, TimingClear clear);

}

#endif
