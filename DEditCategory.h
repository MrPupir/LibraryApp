#pragma once
#include "afxdialogex.h"

class DEditCategory : public CDialogEx
{
	DECLARE_DYNAMIC(DEditCategory)

public:
	DEditCategory(CWnd* pParent = nullptr);
	virtual ~DEditCategory();
	int GetSelectedCategoryId() const { return m_selectedCategoryId; }
	CString GetCategoryName() const { return m_categoryName; }

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EDIT_CATEGORY };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnCbnSelchangeCategory();

private:
	int m_selectedCategoryId = 0;
	CString m_categoryName;
	CComboBox m_comboCategory;
	CEdit m_editName;

	DECLARE_MESSAGE_MAP()
};
