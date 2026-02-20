#pragma once
#include "afxdialogex.h"

class DAddCategory : public CDialogEx
{
	DECLARE_DYNAMIC(DAddCategory)

public:
	DAddCategory(CWnd* pParent = nullptr);
	virtual ~DAddCategory();
	CString GetCategoryName() const { return m_categoryName; }

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ADD_CATEGORY };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();

private:
	CString m_categoryName;
	CEdit m_editName;

	DECLARE_MESSAGE_MAP()
};
