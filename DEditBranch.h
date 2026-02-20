#pragma once
#include "afxdialogex.h"
#include "Database.h"
#include <vector>

class DEditBranch : public CDialogEx
{
	DECLARE_DYNAMIC(DEditBranch)

public:
	DEditBranch(CWnd* pParent = nullptr);
	virtual ~DEditBranch();

	int GetSelectedBranchId() const { return m_selectedId; }
	CString GetBranchName() const { return m_name; }
	CString GetBranchAddress() const { return m_address; }

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EDIT_BRANCH };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnCbnSelChangeBranch();

private:
	int m_selectedId;
	CString m_name;
	CString m_address;
	CComboBox m_comboBranch;
	CEdit m_editName;
	CEdit m_editAddress;
	std::vector<Database::BranchRecord> m_branches;

	DECLARE_MESSAGE_MAP()
};
