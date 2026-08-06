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

#pragma once

class Rod;
class Particle;

class ForceSource {
public:
  virtual ~ForceSource() = default;

  virtual bool contains(const Rod *rod) const = 0;
  virtual bool contains(const Particle *Particle) const = 0;
};

class RodSource : public ForceSource {
public:
  RodSource(const Rod *rod) : _rod(rod) {}

  bool contains(const Rod *rod) const override;
  bool contains(const Particle *particle) const override;

private:
  const Rod *_rod{};
};

// TODO: implement this at some point
//
// class AngleSource : public ForceSource {
// public:
//   AngleSource(const Rod *left, const Rod *right);
//
//   bool contains(const Rod *rod) const override;
//   bool contains(const Particle *particle) const override;
//
// private:
//   const Rod *_left{};
//   const Rod *_right{};
// };
