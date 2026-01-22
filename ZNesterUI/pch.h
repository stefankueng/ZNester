// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

#include "targetver.h"
// Windows Header Files
#include <windows.h>

#include <windowsx.h>
#define _USE_MATH_DEFINES
// C RunTime Header Files
#include <Objbase.h>
#include <commctrl.h>
#include <malloc.h>
#include <memory.h>
#include <stdlib.h>

#pragma comment( lib, "comctl32.lib" )
#pragma comment( lib, "shlwapi.lib" )

#pragma comment( \
	linker,      \
	"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"" )
