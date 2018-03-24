/*
  This file is part of the Fairytale project

  Copyright (C) 2018 Márcio Pais

  This library is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TRANSFORMFACTORY_H
#define TRANSFORMFACTORY_H

#include "block.h"
#include "transforms/zlibtransform.h"

class TransformFactory {
public:
	static Transform* createTransform(const BlockType type) {
		switch (type) {
			case BlockType::DEFLATE: return new zLibTransform();
			default: return nullptr;
		}
	}
};

#endif
