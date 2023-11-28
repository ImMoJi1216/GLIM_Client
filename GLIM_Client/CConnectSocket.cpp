#include "pch.h"
#include "CConnectSocket.h"
#include "GLIM_ClientDlg.h"

void CConnectSocket::OnClose(int nErrorCode)
{
    ShutDown();
    Close();

    CSocket::OnClose(nErrorCode);

    AfxMessageBox(_T("ERROR:Disconnected from server!"));
    ::PostQuitMessage(0);
}

void CConnectSocket::OnReceive(int nErrorCode)
{
    CGLIMClientDlg* pMain = (CGLIMClientDlg*)AfxGetMainWnd();
    TCHAR strBuffer[1024];
    ::ZeroMemory(strBuffer, sizeof(strBuffer));
    // ::은 범위지정 연산자  strBuffer의 배열을 0으로 지정
    if (Receive(strBuffer, sizeof(strBuffer)) > 0) // 전달된 데이터(문자열)가 있을 경우
    {
        AfxMessageBox(_T("씨발련아"));
        TRACE(_T("Received data: %s\n"), strBuffer);
        CStringArray tokens;
        SplitCString(strBuffer, tokens, _T(','));
        if (tokens.GetSize() >= 2)
        {
            CString Type_Token = tokens.GetAt(0);
            if (Type_Token == "Results")
            {
                pMain->m_List.AddString(_T("씨발아"));
            }
        }
    }
    CSocket::OnReceive(nErrorCode);

}

void CConnectSocket::SplitCString(const CString & input, CStringArray & output, TCHAR delimiter)
{
    int start = 0;
    int end = 0;

    while ((end = input.Find(delimiter, start)) != -1)
    {
        CString token = input.Mid(start, end - start);
        output.Add(token);
        start = end + 1;
    }

    CString lastToken = input.Mid(start);
    output.Add(lastToken);
}