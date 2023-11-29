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
    // ::�� �������� ������  strBuffer�� �迭�� 0���� ����
    
    if (Receive(strBuffer, sizeof(strBuffer)) > 0) // ���޵� ������(���ڿ�)�� ���� ���
    {
        TRACE(_T("Received data: %s\n"), strBuffer);
        CStringArray tokens;
        SplitCString(strBuffer, tokens, _T(','));
        if (tokens.GetSize() >= 2)
        {
            CString Type_Token = tokens.GetAt(0);
            CString Results_Token = tokens.GetAt(1);
            if (Type_Token == "Results")
            {
                if (Results_Token == "2")
                {
                    pMain->Send_Result_Arduino("2");
                }
                else if (Results_Token == "3")
                {
                    pMain->Send_Result_Arduino("3");
                    pMain->m_List.AddString(_T("RESULT LOG : �ҷ� ������"));
                }
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