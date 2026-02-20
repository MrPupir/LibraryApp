// DAddReview.cpp: файл реализации
//

#include "pch.h"
#include "LibraryApp.h"
#include "afxdialogex.h"
#include "DAddReview.h"


// Диалоговое окно DAddReview

IMPLEMENT_DYNAMIC(DAddReview, CDialogEx)

DAddReview::DAddReview(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ADD_REVIEW, pParent)
{

}

DAddReview::~DAddReview()
{
}

void DAddReview::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_RATING, m_comboRating);
	DDX_Control(pDX, IDC_EDIT_COMMENT, m_editComment);
}


BEGIN_MESSAGE_MAP(DAddReview, CDialogEx)
END_MESSAGE_MAP()


// Обработчики сообщений DAddReview

BOOL DAddReview::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	for (int i = 1; i <= 5; ++i)
	{
		CString s;
		s.Format(_T("%d"), i);
		m_comboRating.AddString(s);
	}
	m_comboRating.SetCurSel(4);
	return TRUE;
}

void DAddReview::OnOK()
{
	int sel = m_comboRating.GetCurSel();
	if (sel == CB_ERR)
	{
		AfxMessageBox(_T("Оберіть оцінку."));
		return;
	}
	CString s;
	m_comboRating.GetLBText(sel, s);
	m_rating = _ttoi(s);
	m_editComment.GetWindowTextW(m_comment);
	m_comment.Trim();
	if (m_comment.IsEmpty())
	{
		AfxMessageBox(_T("Введіть текст відгуку."));
		return;
	}
	CDialogEx::OnOK();
}
