#pragma once
#include "afxdialogex.h"

class DDeleteCategory : public CDialogEx
{
	DECLARE_DYNAMIC(DDeleteCategory)

public:
	DDeleteCategory(CWnd* pParent = nullptr);
	virtual ~DDeleteCategory();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DELETE_CATEGORY };
#endif

	int GetSelectedCategoryId() const;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	int m_selectedCategoryId = 0;
	CComboBox m_comboCategory;

	DECLARE_MESSAGE_MAP()
};
