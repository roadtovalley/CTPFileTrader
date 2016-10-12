#ifndef _WIN32_WINNT // Allow use of features specific to Windows NT 4 or later.
  #define _WIN32_WINNT 0x0501 // Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINDOWS // Allow use of features specific to Windows 98 or later.
  #define _WIN32_WINDOWS 0x0501 // Change this to the appropriate value to target Windows Me or later.
#endif

#if !defined(AFX_STDAFX_H__492A505C_688D_43E1_AA53_B158C70CDF35__INCLUDED_)
  #define AFX_STDAFX_H__492A505C_688D_43E1_AA53_B158C70CDF35__INCLUDED_

  #if _MSC_VER > 1000
    #pragma once
  #endif // _MSC_VER > 1000

  #define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

  #include <Windows.h>
  #include <stdio.h>
#endif // !defined(AFX_STDAFX_H__492A505C_688D_43E1_AA53_B158C70CDF35__INCLUDED_)
