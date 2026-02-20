#pragma once
#include "afxdialogex.h"

class DAddBranch : public CDialogEx
{
	DECLARE_DYNAMIC(DAddBranch)

public:
	DAddBranch(CWnd* pParent = nullptr);
	virtual ~DAddBranch();

	CString GetBranchName() const { return m_name; }
	CString GetBranchAddress() const { return m_address; }

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ADD_BRANCH };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();

private:
	CString m_name;
	CString m_address;
	CEdit m_editName;
	CEdit m_editAddress;

	DECLARE_MESSAGE_MAP()
};
