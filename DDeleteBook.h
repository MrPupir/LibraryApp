#pragma once
#include "afxdialogex.h"


// Диалоговое окно DDeleteBook

class DDeleteBook : public CDialogEx
{
	DECLARE_DYNAMIC(DDeleteBook)

public:
	DDeleteBook(CWnd* pParent = nullptr);   // стандартный конструктор
	virtual ~DDeleteBook();
	int GetBookId() const { return m_bookId; }
	void SetBookId(int id) { m_bookId = id; }

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DELETE_BOOK };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // поддержка DDX/DDV
	virtual BOOL OnInitDialog();
	virtual void OnOK();

private:
	int m_bookId = 0;

	DECLARE_MESSAGE_MAP()
};
