#include "pch.h"
#include "LibraryApp.h"
#include "afxdialogex.h"
#include "DAddCategory.h"

IMPLEMENT_DYNAMIC(DAddCategory, CDialogEx)

DAddCategory::DAddCategory(CWnd* pParent)
	: CDialogEx(IDD_ADD_CATEGORY, pParent)
{
}

DAddCategory::~DAddCategory()
{
}

void DAddCategory::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_CATEGORY_NAME, m_editName);
}

BEGIN_MESSAGE_MAP(DAddCategory, CDialogEx)
END_MESSAGE_MAP()

BOOL DAddCategory::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_editName.SetFocus();
	return FALSE;
}

void DAddCategory::OnOK()
{
	m_editName.GetWindowTextW(m_categoryName);
	m_categoryName.Trim();

	if (m_categoryName.IsEmpty())
	{
		AfxMessageBox(_T("Введіть назву категорії."));
		return;
	}
	if (m_categoryName.GetLength() < 2 || m_categoryName.GetLength() > 100)
	{
		AfxMessageBox(_T("Назва категорії має містити від 2 до 100 символів."));
		return;
	}

	CDialogEx::OnOK();
}
