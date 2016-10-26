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

#include <platforms/darwin/macUtils.hpp>
#include <Cocoa/Cocoa.h>

namespace qst
{
namespace platforms
{
namespace darwin
{
void MacUtils::showDockIcon(bool show)
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
    dispatch_async(dispatch_get_main_queue(), ^{
      [NSApp activateIgnoringOtherApps:YES];
    });
  }
#pragma clang diagnostic pop
}
}
}
}
