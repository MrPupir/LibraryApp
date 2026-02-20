// DDeleteCategory.cpp: файл реализации
//

#include "pch.h"
#include "LibraryApp.h"
#include "afxdialogex.h"
#include "DDeleteCategory.h"
#include "Database.h"

IMPLEMENT_DYNAMIC(DDeleteCategory, CDialogEx)

DDeleteCategory::DDeleteCategory(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DELETE_CATEGORY, pParent)
{
}

DDeleteCategory::~DDeleteCategory()
{
}

void DDeleteCategory::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_CATEGORY_SELECT, m_comboCategory);
}

BEGIN_MESSAGE_MAP(DDeleteCategory, CDialogEx)
END_MESSAGE_MAP()

BOOL DDeleteCategory::OnInitDialog()
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
	}

	return TRUE;
}

void DDeleteCategory::OnOK()
{
	int sel = m_comboCategory.GetCurSel();
	if (sel == CB_ERR) {
		AfxMessageBox(_T("Оберіть категорію для видалення!"), MB_ICONWARNING);
		return;
	}

	m_selectedCategoryId = (int)m_comboCategory.GetItemData(sel);

	if (AfxMessageBox(_T("Ви впевнені, що хочете видалити цю категорію?\n(Категорію з книгами видалити не можна.)"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
		CDialogEx::OnOK();
	}
}

int DDeleteCategory::GetSelectedCategoryId() const
{
	return m_selectedCategoryId;
}
