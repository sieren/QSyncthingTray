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

#pragma once

#ifdef _WIN32
#include "platforms/windows/winUtils.hpp"
#elif defined(__linux__) || defined (__OpenBSD__) || defined(__NetBSD__) || defined(__FreeBSD__)
#include "platforms/linux/posixUtils.hpp"
#elif (defined(__APPLE__) && defined(__MACH__))
#include "platforms/darwin/macUtils.hpp"
#endif

namespace qst
{
namespace sysutils
{
#ifdef _WIN32
using SystemUtility = platforms::windows::WinUtils;
#elif defined(__linux__) || defined (__OpenBSD__) || defined(__NetBSD__) || defined(__FreeBSD__)
using SystemUtility = platforms::gnu_linux::PosixUtils;
#elif (defined(__APPLE__) && defined(__MACH__))
using SystemUtility = platforms::darwin::MacUtils;
#endif
  
} // sysutils
} // qst

