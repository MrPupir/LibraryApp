// DEditCategory.cpp: файл реализации
//

#include "pch.h"
#include "LibraryApp.h"
#include "afxdialogex.h"
#include "DEditCategory.h"
#include "Database.h"

IMPLEMENT_DYNAMIC(DEditCategory, CDialogEx)

DEditCategory::DEditCategory(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EDIT_CATEGORY, pParent)
{
}

DEditCategory::~DEditCategory()
{
}

void DEditCategory::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_CATEGORY_SELECT, m_comboCategory);
	DDX_Control(pDX, IDC_EDIT_CATEGORY_NAME, m_editName);
}

BEGIN_MESSAGE_MAP(DEditCategory, CDialogEx)
	ON_CBN_SELCHANGE(IDC_COMBO_CATEGORY_SELECT, &DEditCategory::OnCbnSelchangeCategory)
END_MESSAGE_MAP()

BOOL DEditCategory::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	Database& db = Database::Instance();
	std::vector<CategoryRecord> categories = db.GetAllCategories();
	for (const auto& cat : categories) {
		int idx = m_comboCategory.AddString(cat.name);
		m_comboCategory.SetItemData(idx, cat.id);
	}
	if (m_comboCategory.GetCount() > 0) {
		m_comboCategory.SetCurSel(0);
		OnCbnSelchangeCategory();
	}

	return TRUE;
}

void DEditCategory::OnCbnSelchangeCategory()
{
	int sel = m_comboCategory.GetCurSel();
	if (sel != CB_ERR) {
		m_selectedCategoryId = (int)m_comboCategory.GetItemData(sel);
		CString name;
		m_comboCategory.GetLBText(sel, name);
		m_editName.SetWindowText(name);
	}
}

void DEditCategory::OnOK()
{
	int sel = m_comboCategory.GetCurSel();
	if (sel == CB_ERR) {
		AfxMessageBox(_T("Оберіть категорію для редагування!"), MB_ICONWARNING);
		return;
	}

	m_selectedCategoryId = (int)m_comboCategory.GetItemData(sel);
	m_editName.GetWindowText(m_categoryName);
	m_categoryName.Trim();

	if (m_categoryName.IsEmpty()) {
		AfxMessageBox(_T("Введіть нову назву категорії!"), MB_ICONWARNING);
		return;
	}
	if (m_categoryName.GetLength() < 2 || m_categoryName.GetLength() > 100) {
		AfxMessageBox(_T("Назва категорії має містити від 2 до 100 символів."), MB_ICONWARNING);
		return;
	}

	CDialogEx::OnOK();
}
