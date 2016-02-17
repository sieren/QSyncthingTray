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

#ifndef QSyncthingTray_posixUtils_hpp
#define QSyncthingTray_posixUtils_hpp
#pragma once
#include <sstream>
#include <string>
#include <iostream>
#include <Carbon/Carbon.h>
#define UNUSED(x) (void)(x)

namespace qst
{
namespace platforms
{
namespace darwin
{
  static int windowCounter = 0;

  struct MacUtils
  {
    char getPlatformDelimiter()
    {
      return '/';
    }

    void showDockIcon(bool show)
    {
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
      ProcessSerialNumber psn;
      windowCounter += show ? 1 : -1;
      UInt32 transformState = show ? kProcessTransformToForegroundApplication :
        kProcessTransformToUIElementApplication;
      const bool shouldHideLastWindow =
        (transformState == kProcessTransformToUIElementApplication &&
         windowCounter == 0) ? true : false;
      const bool shouldShowWindow =
        (transformState == kProcessTransformToForegroundApplication &&
         windowCounter > 0) ? true : false;
      if (GetCurrentProcess(&psn) == noErr && (shouldShowWindow || shouldHideLastWindow))
      {
        TransformProcessType(&psn,
          transformState);
      }
    #pragma clang diagnostic pop
    }

    std::string getSSLLibraryText()
    {
      return std::string("HTTPS is not supported on this platform. "
        "This can happen when OpenSSL is missing.");
    }

    static bool isBinaryRunning(std::string binary)
    {
      const char* someapp = binary.c_str();
      std::stringstream cmd;
      
      cmd << "pgrep -x " << someapp;
      
      FILE* app = popen(cmd.str().c_str(), "r");
      char instances = '0';
      
      if (app)
      {
        fread(&instances, sizeof(instances), 1, app);
        pclose(app);
      }
      bool result = instances == '0' ? false : true;
      return result;
    }

    template<typename U, typename T>
    void doubleClicked(U&& func, T ref)
    {
      // stubbed out, no action on Mac
    }

  };
} // posix
} // sysutils
} // qst

#endif
