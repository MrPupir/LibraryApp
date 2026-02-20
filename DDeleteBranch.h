#pragma once
#include "afxdialogex.h"
#include "Database.h"
#include <vector>

class DDeleteBranch : public CDialogEx
{
	DECLARE_DYNAMIC(DDeleteBranch)

public:
	DDeleteBranch(CWnd* pParent = nullptr);
	virtual ~DDeleteBranch();

	int GetSelectedBranchId() const { return m_selectedId; }

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DELETE_BRANCH };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();

private:
	int m_selectedId;
	CComboBox m_comboBranch;
	std::vector<Database::BranchRecord> m_branches;

	DECLARE_MESSAGE_MAP()
};
