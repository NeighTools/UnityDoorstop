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

FARPROC originalFunctions[64] = {0};

void loadFunctions(HMODULE dll)
{
ADD_ORIGINAL(0, GetIpAddrTable);
ADD_ORIGINAL(1, SvchostPushServiceGlobals);
ADD_ORIGINAL(2, WinHttpAddRequestHeaders);
ADD_ORIGINAL(3, WinHttpAutoProxySvcMain);
ADD_ORIGINAL(4, WinHttpCheckPlatform);
ADD_ORIGINAL(5, WinHttpCloseHandle);
ADD_ORIGINAL(6, WinHttpConnect);
ADD_ORIGINAL(7, WinHttpConnectionDeletePolicyEntries);
ADD_ORIGINAL(8, WinHttpConnectionDeleteProxyInfo);
ADD_ORIGINAL(9, WinHttpConnectionFreeNameList);
ADD_ORIGINAL(10, WinHttpConnectionFreeProxyInfo);
ADD_ORIGINAL(11, WinHttpConnectionFreeProxyList);
ADD_ORIGINAL(12, WinHttpConnectionGetNameList);
ADD_ORIGINAL(13, WinHttpConnectionGetProxyInfo);
ADD_ORIGINAL(14, WinHttpConnectionGetProxyList);
ADD_ORIGINAL(15, WinHttpConnectionSetPolicyEntries);
ADD_ORIGINAL(16, WinHttpConnectionSetProxyInfo);
ADD_ORIGINAL(17, WinHttpConnectionUpdateIfIndexTable);
ADD_ORIGINAL(18, WinHttpCrackUrl);
ADD_ORIGINAL(19, WinHttpCreateProxyResolver);
ADD_ORIGINAL(20, WinHttpCreateUrl);
ADD_ORIGINAL(21, WinHttpDetectAutoProxyConfigUrl);
ADD_ORIGINAL(22, WinHttpFreeProxyResult);
ADD_ORIGINAL(23, WinHttpFreeProxyResultEx);
ADD_ORIGINAL(24, WinHttpFreeProxySettings);
ADD_ORIGINAL(25, WinHttpGetDefaultProxyConfiguration);
ADD_ORIGINAL(26, WinHttpGetIEProxyConfigForCurrentUser);
ADD_ORIGINAL(27, WinHttpGetProxyForUrl);
ADD_ORIGINAL(28, WinHttpGetProxyForUrlEx);
ADD_ORIGINAL(29, WinHttpGetProxyForUrlEx2);
ADD_ORIGINAL(30, WinHttpGetProxyForUrlHvsi);
ADD_ORIGINAL(31, WinHttpGetProxyResult);
ADD_ORIGINAL(32, WinHttpGetProxyResultEx);
ADD_ORIGINAL(33, WinHttpGetProxySettingsVersion);
ADD_ORIGINAL(34, WinHttpGetTunnelSocket);
ADD_ORIGINAL(35, WinHttpOpen);
ADD_ORIGINAL(36, WinHttpOpenRequest);
ADD_ORIGINAL(37, WinHttpProbeConnectivity);
ADD_ORIGINAL(38, WinHttpQueryAuthSchemes);
ADD_ORIGINAL(39, WinHttpQueryDataAvailable);
ADD_ORIGINAL(40, WinHttpQueryHeaders);
ADD_ORIGINAL(41, WinHttpQueryOption);
ADD_ORIGINAL(42, WinHttpReadData);
ADD_ORIGINAL(43, WinHttpReadProxySettings);
ADD_ORIGINAL(44, WinHttpReadProxySettingsHvsi);
ADD_ORIGINAL(45, WinHttpReceiveResponse);
ADD_ORIGINAL(46, WinHttpResetAutoProxy);
ADD_ORIGINAL(47, WinHttpSaveProxyCredentials);
ADD_ORIGINAL(48, WinHttpSendRequest);
ADD_ORIGINAL(49, WinHttpSetCredentials);
ADD_ORIGINAL(50, WinHttpSetDefaultProxyConfiguration);
ADD_ORIGINAL(51, WinHttpSetOption);
ADD_ORIGINAL(52, WinHttpSetStatusCallback);
ADD_ORIGINAL(53, WinHttpSetTimeouts);
ADD_ORIGINAL(54, WinHttpTimeFromSystemTime);
ADD_ORIGINAL(55, WinHttpTimeToSystemTime);
ADD_ORIGINAL(56, WinHttpWebSocketClose);
ADD_ORIGINAL(57, WinHttpWebSocketCompleteUpgrade);
ADD_ORIGINAL(58, WinHttpWebSocketQueryCloseStatus);
ADD_ORIGINAL(59, WinHttpWebSocketReceive);
ADD_ORIGINAL(60, WinHttpWebSocketSend);
ADD_ORIGINAL(61, WinHttpWebSocketShutdown);
ADD_ORIGINAL(62, WinHttpWriteData);
ADD_ORIGINAL(63, WinHttpWriteProxySettings);

}

PROXY(0, GetIpAddrTable);
PROXY(1, SvchostPushServiceGlobals);
PROXY(2, WinHttpAddRequestHeaders);
PROXY(3, WinHttpAutoProxySvcMain);
PROXY(4, WinHttpCheckPlatform);
PROXY(5, WinHttpCloseHandle);
PROXY(6, WinHttpConnect);
PROXY(7, WinHttpConnectionDeletePolicyEntries);
PROXY(8, WinHttpConnectionDeleteProxyInfo);
PROXY(9, WinHttpConnectionFreeNameList);
PROXY(10, WinHttpConnectionFreeProxyInfo);
PROXY(11, WinHttpConnectionFreeProxyList);
PROXY(12, WinHttpConnectionGetNameList);
PROXY(13, WinHttpConnectionGetProxyInfo);
PROXY(14, WinHttpConnectionGetProxyList);
PROXY(15, WinHttpConnectionSetPolicyEntries);
PROXY(16, WinHttpConnectionSetProxyInfo);
PROXY(17, WinHttpConnectionUpdateIfIndexTable);
PROXY(18, WinHttpCrackUrl);
PROXY(19, WinHttpCreateProxyResolver);
PROXY(20, WinHttpCreateUrl);
PROXY(21, WinHttpDetectAutoProxyConfigUrl);
PROXY(22, WinHttpFreeProxyResult);
PROXY(23, WinHttpFreeProxyResultEx);
PROXY(24, WinHttpFreeProxySettings);
PROXY(25, WinHttpGetDefaultProxyConfiguration);
PROXY(26, WinHttpGetIEProxyConfigForCurrentUser);
PROXY(27, WinHttpGetProxyForUrl);
PROXY(28, WinHttpGetProxyForUrlEx);
PROXY(29, WinHttpGetProxyForUrlEx2);
PROXY(30, WinHttpGetProxyForUrlHvsi);
PROXY(31, WinHttpGetProxyResult);
PROXY(32, WinHttpGetProxyResultEx);
PROXY(33, WinHttpGetProxySettingsVersion);
PROXY(34, WinHttpGetTunnelSocket);
PROXY(35, WinHttpOpen);
PROXY(36, WinHttpOpenRequest);
PROXY(37, WinHttpProbeConnectivity);
PROXY(38, WinHttpQueryAuthSchemes);
PROXY(39, WinHttpQueryDataAvailable);
PROXY(40, WinHttpQueryHeaders);
PROXY(41, WinHttpQueryOption);
PROXY(42, WinHttpReadData);
PROXY(43, WinHttpReadProxySettings);
PROXY(44, WinHttpReadProxySettingsHvsi);
PROXY(45, WinHttpReceiveResponse);
PROXY(46, WinHttpResetAutoProxy);
PROXY(47, WinHttpSaveProxyCredentials);
PROXY(48, WinHttpSendRequest);
PROXY(49, WinHttpSetCredentials);
PROXY(50, WinHttpSetDefaultProxyConfiguration);
PROXY(51, WinHttpSetOption);
PROXY(52, WinHttpSetStatusCallback);
PROXY(53, WinHttpSetTimeouts);
PROXY(54, WinHttpTimeFromSystemTime);
PROXY(55, WinHttpTimeToSystemTime);
PROXY(56, WinHttpWebSocketClose);
PROXY(57, WinHttpWebSocketCompleteUpgrade);
PROXY(58, WinHttpWebSocketQueryCloseStatus);
PROXY(59, WinHttpWebSocketReceive);
PROXY(60, WinHttpWebSocketSend);
PROXY(61, WinHttpWebSocketShutdown);
PROXY(62, WinHttpWriteData);
PROXY(63, WinHttpWriteProxySettings);
