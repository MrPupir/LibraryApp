// DDeleteBook.cpp: файл реализации
//

#include "pch.h"
#include "LibraryApp.h"
#include "afxdialogex.h"
#include "DDeleteBook.h"


// Диалоговое окно DDeleteBook

IMPLEMENT_DYNAMIC(DDeleteBook, CDialogEx)

DDeleteBook::DDeleteBook(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DELETE_BOOK, pParent)
{

}

DDeleteBook::~DDeleteBook()
{
}

void DDeleteBook::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(DDeleteBook, CDialogEx)
END_MESSAGE_MAP()


// Обработчики сообщений DDeleteBook

BOOL DDeleteBook::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	if (m_bookId > 0)
	{
		SetDlgItemInt(IDC_EDIT_BOOK_ID, m_bookId, FALSE);
	}
	return TRUE;
}

void DDeleteBook::OnOK()
{
	m_bookId = GetDlgItemInt(IDC_EDIT_BOOK_ID, NULL, FALSE);
	if (m_bookId <= 0)
	{
		AfxMessageBox(_T("Вкажіть коректний ID книги (> 0)."));
		return;
	}
	CDialogEx::OnOK();
}
