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

#ifndef QSyncthingTray_winUtils_hpp
#define QSyncthingTray_winUtils_hpp
#include <sstream>
#include <string>
#include <iostream>
#include <cstdio>
#include <windows.h>
#include <tlhelp32.h>
#include <comdef.h>

namespace mfk
{
namespace platforms
{
namespace windows
{
  struct WinUtils
  {

    char getPlatformDelimiter()
    {
      return '\\';
    }
    // stubbed out, does nothing on windows
    void showDockIcon(bool show) { }

    std::string getSSLLibraryText()
    {
      return std::string("In order to use HTTPS URLs on Windows, the "
        "<a href='http://slproweb.com/products/Win32OpenSSL.html'>OpenSSL "
        "Library</a> is required. Please install and restart QSyncthingTray.");
    }

    static bool isBinaryRunning(std::string binary)
    {
      const char *syncapp = binary.c_str();
      bool result = false;
      PROCESSENTRY32 entry;
      entry.dwSize = sizeof(PROCESSENTRY32);

      HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

      if (Process32First(snapshot, &entry))
      {
        while (Process32Next(snapshot, &entry))
        {
          _bstr_t cmpStr(entry.szExeFile);
          if (!_stricmp(cmpStr, syncapp))
          {
            result = true;
          }
        }
      }

      CloseHandle(snapshot);
      return result;

    }
  };
} // windows
} // platforms
} // mfk

#endif
