/*
   CEED - Unified CEGUI asset editor

   Copyright (C) 2011-2017   Martin Preisler <martin@preisler.me>
                             and contributing authors (see AUTHORS file)

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CEED_paths_
#define CEED_paths_

#include "CEEDBase.h"

/**This module contains means to get various CEED related paths in various
environments.
*/

#include "fake.h"
#include "version.h"

#include <QDir>

namespace CEED {
namespace paths {

extern bool FROZEN;
extern QString PACKAGE_DIR;
extern QString DATA_DIR;
extern QString SYSTEM_DATA_DIR;
extern bool SYSTEM_DATA_DIR_EXISTS;
extern QString DOC_DIR;
extern QString SYSTEM_DOC_DIR;
extern bool SYSTEM_DOC_DIR_EXISTS;
extern QString UI_DIR;
extern QString SYSTEM_PIXMAPS_DIR;
extern QString SYSTEM_APPLICATIONS_DIR;
extern QString SYSTEM_APPDATA_DIR;

void init();

} // namespace paths
} // namespace CEED

#endif
