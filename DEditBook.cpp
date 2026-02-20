// DEditBook.cpp : implementation file

#include "pch.h"
#include "LibraryApp.h"
#include "afxdialogex.h"
#include "DEditBook.h"

IMPLEMENT_DYNAMIC(DEditBook, CDialogEx)

DEditBook::DEditBook(CWnd* pParent)
	: CDialogEx(IDD_EDIT_BOOK, pParent)
{
}

DEditBook::~DEditBook()
{
}

void DEditBook::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_BOOK_TITLE, m_title);
	DDX_Control(pDX, IDC_EDIT_BOOK_AUTHOR, m_author);
	DDX_Control(pDX, IDC_EDIT_BOOK_YEAR, m_year);
	DDX_Control(pDX, IDC_COMBO_BOOK_CATEGORY, m_categoryCombo);
	DDX_Control(pDX, IDC_EDIT_BOOK_TOTAL, m_total);
	DDX_Control(pDX, IDC_EDIT_BOOK_AVAILABLE, m_available);
	DDX_Control(pDX, IDC_EDIT_BOOK_DESC, m_desc);
	DDX_Control(pDX, IDC_EDIT_BOOK_COVER, m_coverPath);
}

BEGIN_MESSAGE_MAP(DEditBook, CDialogEx)
END_MESSAGE_MAP()

BOOL DEditBook::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	auto categories = Database::Instance().GetAllCategories();
	int selectedIdx = 0;
	for (int i = 0; i < (int)categories.size(); ++i)
	{
		int idx = m_categoryCombo.AddString(categories[i].name);
		m_categoryCombo.SetItemData(idx, categories[i].id);
		if (categories[i].id == m_book.categoryId || categories[i].name == m_book.category)
			selectedIdx = idx;
	}

	if (!categories.empty())
		m_categoryCombo.SetCurSel(selectedIdx);

	SetDlgItemText(IDC_EDIT_BOOK_TITLE, m_book.title);
	SetDlgItemText(IDC_EDIT_BOOK_AUTHOR, m_book.author);
	SetDlgItemInt(IDC_EDIT_BOOK_YEAR, m_book.year, FALSE);
	SetDlgItemInt(IDC_EDIT_BOOK_TOTAL, m_book.quantityTotal, FALSE);
	SetDlgItemInt(IDC_EDIT_BOOK_AVAILABLE, m_book.quantityAvailable, FALSE);
	SetDlgItemText(IDC_EDIT_BOOK_DESC, m_book.description);
	SetDlgItemText(IDC_EDIT_BOOK_COVER, m_book.coverPath);

	return TRUE;
}

void DEditBook::OnOK()
{
	m_title.GetWindowTextW(m_book.title);
	m_author.GetWindowTextW(m_book.author);
	m_desc.GetWindowTextW(m_book.description);
	m_coverPath.GetWindowTextW(m_book.coverPath);
	m_book.year = GetDlgItemInt(IDC_EDIT_BOOK_YEAR, NULL, FALSE);
	m_book.quantityTotal = GetDlgItemInt(IDC_EDIT_BOOK_TOTAL, NULL, FALSE);
	m_book.quantityAvailable = GetDlgItemInt(IDC_EDIT_BOOK_AVAILABLE, NULL, FALSE);

	int sel = m_categoryCombo.GetCurSel();
	if (sel != CB_ERR)
	{
		m_book.categoryId = (int)m_categoryCombo.GetItemData(sel);
		m_categoryCombo.GetLBText(sel, m_book.category);
	}

	m_book.title.Trim();
	m_book.author.Trim();
	m_book.category.Trim();
	m_book.description.Trim();
	m_book.coverPath.Trim();

	if (m_book.title.IsEmpty() || m_book.author.IsEmpty())
	{
		AfxMessageBox(_T("Заповніть назву та автора."));
		return;
	}

	if (m_book.title.GetLength() > 255 || m_book.author.GetLength() > 180)
	{
		AfxMessageBox(_T("Назва/автор перевищують допустиму довжину."));
		return;
	}

	if (m_book.description.GetLength() > 5000)
	{
		AfxMessageBox(_T("Опис не може бути довшим за 5000 символів."));
		return;
	}

	CTime now = CTime::GetCurrentTime();
	if (m_book.year < 1450 || m_book.year > now.GetYear() + 1)
	{
		AfxMessageBox(_T("Некоректний рік видання."));
		return;
	}

	if (m_book.categoryId <= 0)
	{
		AfxMessageBox(_T("Оберіть категорію."));
		return;
	}

	if (m_book.year <= 0 || m_book.quantityTotal < 0 || m_book.quantityAvailable < 0 || m_book.quantityAvailable > m_book.quantityTotal)
	{
		AfxMessageBox(_T("Некоректні числові значення."));
		return;
	}

	CDialogEx::OnOK();
}


