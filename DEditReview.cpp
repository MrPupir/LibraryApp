// DEditReview.cpp: файл реализации
//

#include "pch.h"
#include "LibraryApp.h"
#include "afxdialogex.h"
#include "DEditReview.h"


// Диалоговое окно DEditReview

IMPLEMENT_DYNAMIC(DEditReview, CDialogEx)

DEditReview::DEditReview(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EDIT_REVIEW, pParent)
{

}

DEditReview::~DEditReview()
{
}

void DEditReview::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_RATING, m_comboRating);
	DDX_Control(pDX, IDC_EDIT_COMMENT, m_editComment);
}


BEGIN_MESSAGE_MAP(DEditReview, CDialogEx)
END_MESSAGE_MAP()


// Обработчики сообщений DEditReview

void DEditReview::SetInitialData(int rating, const CString& comment)
{
	m_rating = rating;
	if (m_rating < 1 || m_rating > 5) m_rating = 5;
	m_comment = comment;
}

BOOL DEditReview::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	for (int i = 1; i <= 5; ++i)
	{
		CString s;
		s.Format(_T("%d"), i);
		m_comboRating.AddString(s);
	}
	m_comboRating.SetCurSel(max(0, min(4, m_rating - 1)));
	m_editComment.SetWindowTextW(m_comment);
	return TRUE;
}

void DEditReview::OnOK()
{
	const int sel = m_comboRating.GetCurSel();
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
