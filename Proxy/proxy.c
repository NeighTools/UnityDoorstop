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
	__declspec(dllexport) ULONG name() \
	{ \
		return originalFunctions[i](); \
	}

FARPROC originalFunctions[63] = {0};

void loadFunctions(HMODULE dll)
{
	ADD_ORIGINAL(0, SvchostPushServiceGlobals);
	ADD_ORIGINAL(1, WinHttpAddRequestHeaders);
	ADD_ORIGINAL(2, WinHttpAutoProxySvcMain);
	ADD_ORIGINAL(3, WinHttpCheckPlatform);
	ADD_ORIGINAL(4, WinHttpCloseHandle);
	ADD_ORIGINAL(5, WinHttpConnect);
	ADD_ORIGINAL(6, WinHttpConnectionDeletePolicyEntries);
	ADD_ORIGINAL(7, WinHttpConnectionDeleteProxyInfo);
	ADD_ORIGINAL(8, WinHttpConnectionFreeNameList);
	ADD_ORIGINAL(9, WinHttpConnectionFreeProxyInfo);
	ADD_ORIGINAL(10, WinHttpConnectionFreeProxyList);
	ADD_ORIGINAL(11, WinHttpConnectionGetNameList);
	ADD_ORIGINAL(12, WinHttpConnectionGetProxyInfo);
	ADD_ORIGINAL(13, WinHttpConnectionGetProxyList);
	ADD_ORIGINAL(14, WinHttpConnectionSetPolicyEntries);
	ADD_ORIGINAL(15, WinHttpConnectionSetProxyInfo);
	ADD_ORIGINAL(16, WinHttpConnectionUpdateIfIndexTable);
	ADD_ORIGINAL(17, WinHttpCrackUrl);
	ADD_ORIGINAL(18, WinHttpCreateProxyResolver);
	ADD_ORIGINAL(19, WinHttpCreateUrl);
	ADD_ORIGINAL(20, WinHttpDetectAutoProxyConfigUrl);
	ADD_ORIGINAL(21, WinHttpFreeProxyResult);
	ADD_ORIGINAL(22, WinHttpFreeProxyResultEx);
	ADD_ORIGINAL(23, WinHttpFreeProxySettings);
	ADD_ORIGINAL(24, WinHttpGetDefaultProxyConfiguration);
	ADD_ORIGINAL(25, WinHttpGetIEProxyConfigForCurrentUser);
	ADD_ORIGINAL(26, WinHttpGetProxyForUrl);
	ADD_ORIGINAL(27, WinHttpGetProxyForUrlEx);
	ADD_ORIGINAL(28, WinHttpGetProxyForUrlEx2);
	ADD_ORIGINAL(29, WinHttpGetProxyForUrlHvsi);
	ADD_ORIGINAL(30, WinHttpGetProxyResult);
	ADD_ORIGINAL(31, WinHttpGetProxyResultEx);
	ADD_ORIGINAL(32, WinHttpGetProxySettingsVersion);
	ADD_ORIGINAL(33, WinHttpGetTunnelSocket);
	ADD_ORIGINAL(34, WinHttpOpen);
	ADD_ORIGINAL(35, WinHttpOpenRequest);
	ADD_ORIGINAL(36, WinHttpProbeConnectivity);
	ADD_ORIGINAL(37, WinHttpQueryAuthSchemes);
	ADD_ORIGINAL(38, WinHttpQueryDataAvailable);
	ADD_ORIGINAL(39, WinHttpQueryHeaders);
	ADD_ORIGINAL(40, WinHttpQueryOption);
	ADD_ORIGINAL(41, WinHttpReadData);
	ADD_ORIGINAL(42, WinHttpReadProxySettings);
	ADD_ORIGINAL(43, WinHttpReadProxySettingsHvsi);
	ADD_ORIGINAL(44, WinHttpReceiveResponse);
	ADD_ORIGINAL(45, WinHttpResetAutoProxy);
	ADD_ORIGINAL(46, WinHttpSaveProxyCredentials);
	ADD_ORIGINAL(47, WinHttpSendRequest);
	ADD_ORIGINAL(48, WinHttpSetCredentials);
	ADD_ORIGINAL(49, WinHttpSetDefaultProxyConfiguration);
	ADD_ORIGINAL(50, WinHttpSetOption);
	ADD_ORIGINAL(51, WinHttpSetStatusCallback);
	ADD_ORIGINAL(52, WinHttpSetTimeouts);
	ADD_ORIGINAL(53, WinHttpTimeFromSystemTime);
	ADD_ORIGINAL(54, WinHttpTimeToSystemTime);
	ADD_ORIGINAL(55, WinHttpWebSocketClose);
	ADD_ORIGINAL(56, WinHttpWebSocketCompleteUpgrade);
	ADD_ORIGINAL(57, WinHttpWebSocketQueryCloseStatus);
	ADD_ORIGINAL(58, WinHttpWebSocketReceive);
	ADD_ORIGINAL(59, WinHttpWebSocketSend);
	ADD_ORIGINAL(60, WinHttpWebSocketShutdown);
	ADD_ORIGINAL(61, WinHttpWriteData);
	ADD_ORIGINAL(62, WinHttpWriteProxySettings);
}

PROXY(0, SvchostPushServiceGlobals);
PROXY(1, WinHttpAddRequestHeaders);
PROXY(2, WinHttpAutoProxySvcMain);
PROXY(3, WinHttpCheckPlatform);
PROXY(4, WinHttpCloseHandle);
PROXY(5, WinHttpConnect);
PROXY(6, WinHttpConnectionDeletePolicyEntries);
PROXY(7, WinHttpConnectionDeleteProxyInfo);
PROXY(8, WinHttpConnectionFreeNameList);
PROXY(9, WinHttpConnectionFreeProxyInfo);
PROXY(10, WinHttpConnectionFreeProxyList);
PROXY(11, WinHttpConnectionGetNameList);
PROXY(12, WinHttpConnectionGetProxyInfo);
PROXY(13, WinHttpConnectionGetProxyList);
PROXY(14, WinHttpConnectionSetPolicyEntries);
PROXY(15, WinHttpConnectionSetProxyInfo);
PROXY(16, WinHttpConnectionUpdateIfIndexTable);
PROXY(17, WinHttpCrackUrl);
PROXY(18, WinHttpCreateProxyResolver);
PROXY(19, WinHttpCreateUrl);
PROXY(20, WinHttpDetectAutoProxyConfigUrl);
PROXY(21, WinHttpFreeProxyResult);
PROXY(22, WinHttpFreeProxyResultEx);
PROXY(23, WinHttpFreeProxySettings);
PROXY(24, WinHttpGetDefaultProxyConfiguration);
PROXY(25, WinHttpGetIEProxyConfigForCurrentUser);
PROXY(26, WinHttpGetProxyForUrl);
PROXY(27, WinHttpGetProxyForUrlEx);
PROXY(28, WinHttpGetProxyForUrlEx2);
PROXY(29, WinHttpGetProxyForUrlHvsi);
PROXY(30, WinHttpGetProxyResult);
PROXY(31, WinHttpGetProxyResultEx);
PROXY(32, WinHttpGetProxySettingsVersion);
PROXY(33, WinHttpGetTunnelSocket);
PROXY(34, WinHttpOpen);
PROXY(35, WinHttpOpenRequest);
PROXY(36, WinHttpProbeConnectivity);
PROXY(37, WinHttpQueryAuthSchemes);
PROXY(38, WinHttpQueryDataAvailable);
PROXY(39, WinHttpQueryHeaders);
PROXY(40, WinHttpQueryOption);
PROXY(41, WinHttpReadData);
PROXY(42, WinHttpReadProxySettings);
PROXY(43, WinHttpReadProxySettingsHvsi);
PROXY(44, WinHttpReceiveResponse);
PROXY(45, WinHttpResetAutoProxy);
PROXY(46, WinHttpSaveProxyCredentials);
PROXY(47, WinHttpSendRequest);
PROXY(48, WinHttpSetCredentials);
PROXY(49, WinHttpSetDefaultProxyConfiguration);
PROXY(50, WinHttpSetOption);
PROXY(51, WinHttpSetStatusCallback);
PROXY(52, WinHttpSetTimeouts);
PROXY(53, WinHttpTimeFromSystemTime);
PROXY(54, WinHttpTimeToSystemTime);
PROXY(55, WinHttpWebSocketClose);
PROXY(56, WinHttpWebSocketCompleteUpgrade);
PROXY(57, WinHttpWebSocketQueryCloseStatus);
PROXY(58, WinHttpWebSocketReceive);
PROXY(59, WinHttpWebSocketSend);
PROXY(60, WinHttpWebSocketShutdown);
PROXY(61, WinHttpWriteData);
PROXY(62, WinHttpWriteProxySettings);
