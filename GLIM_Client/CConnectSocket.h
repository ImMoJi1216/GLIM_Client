#pragma once
#include <afxsock.h>
class CConnectSocket : public CSocket
{
public:
	void OnClose(int nErrorCode);
	void OnReceive(int nErrorCode);
	void SplitCString(const CString& input, CStringArray& output, TCHAR delimiter);
};

