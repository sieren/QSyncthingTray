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
#include <sstream>
#include <string>
#include <iostream>
#include <memory>
#include <QProcessEnvironment>
#include <QString>
#define UNUSED(x) (void)(x)

namespace qst
{
namespace platforms
{
namespace gnu_linux
{
  struct PosixUtils
  {
    char getPlatformDelimiter()
    {
      return '/';
    }

    // stubbed out, does nothing on linux
    void showDockIcon(bool show) 
    {
    UNUSED(show);
    }

    std::string getSSLLibraryText()
    {
      return std::string("In order to use HTTPS URLs on Linux, please \
        install OpenSSL");
    }

    static bool isBinaryRunning(std::string binary)
    {
      const char* someapp = binary.c_str();
      std::stringstream cmd;

      cmd << "pgrep -x \"" << someapp << "\"";

      FILE* app = popen(cmd.str().c_str(), "r");
      char instances = '0';
      
      if (app)
      {
        size_t x = fread(&instances, sizeof(instances), 1, app);
        UNUSED(x);
        pclose(app);
      }
      bool result = instances == '0' ? false : true;
      return result;
    }

    static QString getDefaultSyncthingConfig()
    {
      QString path = QProcessEnvironment::systemEnvironment().value("HOME", "~");
      path.append("/.config/syncthing/config.xml");
      return{path};
    }

    
    static auto getDefaultSyncthingLocation() -> QString
    {
      return{"/usr/local/bin/syncthing"};
    }
    
    static auto getDefaultSyncthingINotifyLocation() -> QString
    {
      return{"/usr/local/bin/syncthing-inotify"};
    }

    template<typename U, typename T>
    void doubleClicked(U&& func, T ref)
    {
      std::function<void()> function = std::bind(func, ref);
      function();
    }
  };
} // posix
} // sysutils
} // qst

#endif
