/* ==================================
 * COMPUTER GENERATED -- DO NOT EDIT
 * ==================================
 * 
 * This file contains the definitions for all proxy functions this DLL supports.
 * 
 * The proxies are very simple functions that should be optimizied into a 
 * single JMP instruction without editing the stack at all.
 * 
 * NOTE: While this works, this is a somewhat hackish approach that is based on how 
 * the compiler optimizes the code. That said, the proxy will not work on Debug build currently
 * (that can be fixed by changing the appropriate compile flag that I am yet to locate).
 */

#include <windows.h>

#define ADD_ORIGINAL(i, name) originalFunctions[i] = GetProcAddress(dll, #name)

#define PROXY(i, name) \
	ULONG name() \
	{ \
		return originalFunctions[i](); \
	}

FARPROC originalFunctions[83] = {0};

void loadFunctions(HMODULE dll)
{
ADD_ORIGINAL(0, WinHttpGetIEProxyConfigForCurrentUser);
ADD_ORIGINAL(1, glIsTexture);
ADD_ORIGINAL(2, glGetTexParameteriv);
ADD_ORIGINAL(3, glTexSubImage2D);
ADD_ORIGINAL(4, glPixelStorei);
ADD_ORIGINAL(5, glCopyTexSubImage2D);
ADD_ORIGINAL(6, glDrawBuffer);
ADD_ORIGINAL(7, glReadBuffer);
ADD_ORIGINAL(8, glDrawArrays);
ADD_ORIGINAL(9, wglDeleteContext);
ADD_ORIGINAL(10, glBegin);
ADD_ORIGINAL(11, glVertex3f);
ADD_ORIGINAL(12, glNormal3f);
ADD_ORIGINAL(13, glColor4f);
ADD_ORIGINAL(14, glEnd);
ADD_ORIGINAL(15, glHint);
ADD_ORIGINAL(16, glLightModelf);
ADD_ORIGINAL(17, glLoadIdentity);
ADD_ORIGINAL(18, glMaterialfv);
ADD_ORIGINAL(19, glMaterialf);
ADD_ORIGINAL(20, glReadPixels);
ADD_ORIGINAL(21, glFinish);
ADD_ORIGINAL(22, glFogi);
ADD_ORIGINAL(23, glFogf);
ADD_ORIGINAL(24, glFogfv);
ADD_ORIGINAL(25, glLightModelfv);
ADD_ORIGINAL(26, glLightf);
ADD_ORIGINAL(27, glLightfv);
ADD_ORIGINAL(28, glTexGeni);
ADD_ORIGINAL(29, glTexGenfv);
ADD_ORIGINAL(30, glTexEnvfv);
ADD_ORIGINAL(31, glScissor);
ADD_ORIGINAL(32, glViewport);
ADD_ORIGINAL(33, glColor4fv);
ADD_ORIGINAL(34, glLightModeli);
ADD_ORIGINAL(35, glColorMaterial);
ADD_ORIGINAL(36, glGetFloatv);
ADD_ORIGINAL(37, glMultMatrixf);
ADD_ORIGINAL(38, glMatrixMode);
ADD_ORIGINAL(39, glPolygonMode);
ADD_ORIGINAL(40, glFrontFace);
ADD_ORIGINAL(41, glClearColor);
ADD_ORIGINAL(42, glClearDepth);
ADD_ORIGINAL(43, glClearStencil);
ADD_ORIGINAL(44, glClear);
ADD_ORIGINAL(45, glIsEnabled);
ADD_ORIGINAL(46, glStencilFunc);
ADD_ORIGINAL(47, glStencilOp);
ADD_ORIGINAL(48, glStencilMask);
ADD_ORIGINAL(49, glDepthFunc);
ADD_ORIGINAL(50, glDepthMask);
ADD_ORIGINAL(51, glCullFace);
ADD_ORIGINAL(52, glPolygonOffset);
ADD_ORIGINAL(53, glColorMask);
ADD_ORIGINAL(54, glDisable);
ADD_ORIGINAL(55, glBlendFunc);
ADD_ORIGINAL(56, glEnable);
ADD_ORIGINAL(57, glAlphaFunc);
ADD_ORIGINAL(58, glTexEnvi);
ADD_ORIGINAL(59, glTexEnvf);
ADD_ORIGINAL(60, glDisableClientState);
ADD_ORIGINAL(61, glEnableClientState);
ADD_ORIGINAL(62, glColorPointer);
ADD_ORIGINAL(63, glVertexPointer);
ADD_ORIGINAL(64, glNormalPointer);
ADD_ORIGINAL(65, glTexCoordPointer);
ADD_ORIGINAL(66, glDrawElements);
ADD_ORIGINAL(67, glGetString);
ADD_ORIGINAL(68, glGetError);
ADD_ORIGINAL(69, glDeleteTextures);
ADD_ORIGINAL(70, glGenTextures);
ADD_ORIGINAL(71, glBindTexture);
ADD_ORIGINAL(72, glTexParameteri);
ADD_ORIGINAL(73, glTexImage2D);
ADD_ORIGINAL(74, wglGetProcAddress);
ADD_ORIGINAL(75, glGetIntegerv);
ADD_ORIGINAL(76, wglShareLists);
ADD_ORIGINAL(77, wglGetCurrentDC);
ADD_ORIGINAL(78, wglGetCurrentContext);
ADD_ORIGINAL(79, wglMakeCurrent);
ADD_ORIGINAL(80, wglCreateContext);
ADD_ORIGINAL(81, glLoadMatrixf);
ADD_ORIGINAL(82, GetIpAddrTable);

}

PROXY(0, WinHttpGetIEProxyConfigForCurrentUser);
PROXY(1, glIsTexture);
PROXY(2, glGetTexParameteriv);
PROXY(3, glTexSubImage2D);
PROXY(4, glPixelStorei);
PROXY(5, glCopyTexSubImage2D);
PROXY(6, glDrawBuffer);
PROXY(7, glReadBuffer);
PROXY(8, glDrawArrays);
PROXY(9, wglDeleteContext);
PROXY(10, glBegin);
PROXY(11, glVertex3f);
PROXY(12, glNormal3f);
PROXY(13, glColor4f);
PROXY(14, glEnd);
PROXY(15, glHint);
PROXY(16, glLightModelf);
PROXY(17, glLoadIdentity);
PROXY(18, glMaterialfv);
PROXY(19, glMaterialf);
PROXY(20, glReadPixels);
PROXY(21, glFinish);
PROXY(22, glFogi);
PROXY(23, glFogf);
PROXY(24, glFogfv);
PROXY(25, glLightModelfv);
PROXY(26, glLightf);
PROXY(27, glLightfv);
PROXY(28, glTexGeni);
PROXY(29, glTexGenfv);
PROXY(30, glTexEnvfv);
PROXY(31, glScissor);
PROXY(32, glViewport);
PROXY(33, glColor4fv);
PROXY(34, glLightModeli);
PROXY(35, glColorMaterial);
PROXY(36, glGetFloatv);
PROXY(37, glMultMatrixf);
PROXY(38, glMatrixMode);
PROXY(39, glPolygonMode);
PROXY(40, glFrontFace);
PROXY(41, glClearColor);
PROXY(42, glClearDepth);
PROXY(43, glClearStencil);
PROXY(44, glClear);
PROXY(45, glIsEnabled);
PROXY(46, glStencilFunc);
PROXY(47, glStencilOp);
PROXY(48, glStencilMask);
PROXY(49, glDepthFunc);
PROXY(50, glDepthMask);
PROXY(51, glCullFace);
PROXY(52, glPolygonOffset);
PROXY(53, glColorMask);
PROXY(54, glDisable);
PROXY(55, glBlendFunc);
PROXY(56, glEnable);
PROXY(57, glAlphaFunc);
PROXY(58, glTexEnvi);
PROXY(59, glTexEnvf);
PROXY(60, glDisableClientState);
PROXY(61, glEnableClientState);
PROXY(62, glColorPointer);
PROXY(63, glVertexPointer);
PROXY(64, glNormalPointer);
PROXY(65, glTexCoordPointer);
PROXY(66, glDrawElements);
PROXY(67, glGetString);
PROXY(68, glGetError);
PROXY(69, glDeleteTextures);
PROXY(70, glGenTextures);
PROXY(71, glBindTexture);
PROXY(72, glTexParameteri);
PROXY(73, glTexImage2D);
PROXY(74, wglGetProcAddress);
PROXY(75, glGetIntegerv);
PROXY(76, wglShareLists);
PROXY(77, wglGetCurrentDC);
PROXY(78, wglGetCurrentContext);
PROXY(79, wglMakeCurrent);
PROXY(80, wglCreateContext);
PROXY(81, glLoadMatrixf);
PROXY(82, GetIpAddrTable);
