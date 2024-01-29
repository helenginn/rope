// vagabond
// Copyright (C) 2022 Helen Ginn
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Please email: vagabond @ hginn.co.uk for more details.

#ifndef __vagabond__AreaMeasurer__
#define __vagabond__AreaMeasurer__

#include "AtomPosMap.h"
#include "vagabond/core/Fibonacci.h"
#include <set>

class SurfaceAreaHandler;
class ContactSheet;
struct Job;

class AreaMeasurer
{
public:
  AreaMeasurer(SurfaceAreaHandler *handler = nullptr, int n_points = 400);
  ~AreaMeasurer();
  
  void setJob(Job *j)
  {
    _job = j;
  }
  
  Job *job() const
  {
    return _job;
  }

  void copyAtomMap(AtomPosMap &posMap)
  {
    _posMap = posMap;
  }
  
  const AtomPosMap &atomPosMap() const
  {
    return _posMap;
  }

  /** set probe radius used for calculating solvent accesible surface*/
  void setProbeRadius(float radius)
  {
    _probeRadius = radius;
  }
  
  /** calculates surface area of previously copied atom map in Angstroms. */
  float surfaceArea(const AtomPosMap &posMap);

  /** calculates exposure of atom, i.e. the percentage of points not in 
   * the overlap with the other atoms in the 'atomsNear' set */
  float fibExposureSingleAtom(const std::set<Atom *> &atomsNearPosmap,
                              Atom *atom, const float radius);
  
  /** more efficiently calculates exposure of atom,
   *  i.e. the percentage of points not in the overlap with other atoms
   *  within a specified radius */
  float fibExposureSingleAtomZSlice(const AtomPosMap &posMap, Atom *atom,
                                    const float radius);

private:
  Job *_job = nullptr;
  AtomPosMap _posMap;
  Fibonacci _lattice;
  float _probeRadius = 1.4; // water molecule radius estimation
  const float _maxVdWRadius = 3.48; //Fr (Francium) radius in gemmi library

  SurfaceAreaHandler *_handler = nullptr;
  ContactSheet *_contacts = nullptr;
};

/** calculates exposed area from atom's lattice exposure, vdw & probe radius */
float areaFromExposure(float exposure, float radius, double probeRadius = 1.4);

/** gets van der Waals radius from an atom */
float getVdWRadius(Atom *atom);

/** handy function to get squared length of a vector efficiently */
inline float sqlength(glm::vec3 a)
{
  return glm::dot(a, a);
}

/** singleton class to time surface area calculations */
class TimerSurfaceArea
{
public:
  static TimerSurfaceArea &getInstance()
  {
    static TimerSurfaceArea instance;
    return instance;
  }
  
  void reset() // reset timer
  {
    times.clear();
    loops = 0;
  }
  
  void start()  // start timer
  {
    _start = std::chrono::high_resolution_clock::now();
  }

  void end() // end timer
  {
    _end = std::chrono::high_resolution_clock::now();
    times.push_back(_end - _start);
  }
  
  std::vector<std::chrono::duration<float>> times;
  bool timing = false;
  int loops = 0;
private:
  TimerSurfaceArea() {}
  TimerSurfaceArea(TimerSurfaceArea const&) = delete;
  void operator=(TimerSurfaceArea const&) = delete;
  
  std::chrono::high_resolution_clock::time_point _start;
  std::chrono::high_resolution_clock::time_point _end;
};

#endif