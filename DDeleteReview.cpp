// DDeleteReview.cpp: файл реализации
//

#include "pch.h"
#include "LibraryApp.h"
#include "afxdialogex.h"
#include "DDeleteReview.h"


// Диалоговое окно DDeleteReview

IMPLEMENT_DYNAMIC(DDeleteReview, CDialogEx)

DDeleteReview::DDeleteReview(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DELETE_REVIEW, pParent)
{

}

DDeleteReview::~DDeleteReview()
{
}

void DDeleteReview::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(DDeleteReview, CDialogEx)
END_MESSAGE_MAP()


// Обработчики сообщений DDeleteReview

BOOL DDeleteReview::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	if (m_prompt.IsEmpty())
	{
		m_prompt = _T("Видалити цей відгук?");
	}
	CWnd* text = GetDlgItem(IDC_STATIC);
	if (text) text->SetWindowTextW(m_prompt);
	return TRUE;
}
