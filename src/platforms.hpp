/******************************************************************************
// QSyncthingTray
// Copyright (c) Matthias Frick, All rights reserved.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3.0 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library.
******************************************************************************/

#ifndef QSyncthingTray_systemUtils_hpp
#define QSyncthingTray_systemUtils_hpp

// CONCEPT
// These functions have to be offered by both classes for
// cross-platform maintainability.
//

namespace mfk
{
namespace sysutils
{
  template <typename T>
  struct SystemUtility
  {
    bool isBinaryRunning(std::string binary)
    {
      return T::isBinaryRunningImpl(binary);
    }
  };
} // sysutils
} // mfk
#endif
