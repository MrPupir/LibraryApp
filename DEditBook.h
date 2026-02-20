#pragma once
#include "afxdialogex.h"
#include "Database.h"

class DEditBook : public CDialogEx
{
	DECLARE_DYNAMIC(DEditBook)

public:
	DEditBook(CWnd* pParent = nullptr);
	virtual ~DEditBook();
	void SetBook(const BookRecord& book) { m_book = book; }
	BookRecord GetBook() const { return m_book; }

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EDIT_BOOK };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();

private:
	BookRecord m_book;
	CEdit m_title;
	CEdit m_author;
	CEdit m_year;
	CComboBox m_categoryCombo;
	CEdit m_total;
	CEdit m_available;
	CEdit m_desc;
	CEdit m_coverPath;

	DECLARE_MESSAGE_MAP()
};
