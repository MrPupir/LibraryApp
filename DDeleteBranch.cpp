// DDeleteBranch.cpp: файл реализации

#include "pch.h"
#include "LibraryApp.h"
#include "afxdialogex.h"
#include "DDeleteBranch.h"

IMPLEMENT_DYNAMIC(DDeleteBranch, CDialogEx)

DDeleteBranch::DDeleteBranch(CWnd* pParent)
	: CDialogEx(IDD_DELETE_BRANCH, pParent)
	, m_selectedId(0)
{
}

DDeleteBranch::~DDeleteBranch()
{
}

void DDeleteBranch::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_BRANCH_SELECT, m_comboBranch);
}

BEGIN_MESSAGE_MAP(DDeleteBranch, CDialogEx)
END_MESSAGE_MAP()

BOOL DDeleteBranch::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_branches = Database::Instance().GetAllBranches();
	for (const auto& br : m_branches)
	{
		CString label;
		label.Format(_T("[%d] %s — %s"), br.id, br.name.GetString(), br.address.GetString());
		m_comboBranch.AddString(label);
	}

	if (!m_branches.empty())
		m_comboBranch.SetCurSel(0);

	return TRUE;
}

void DDeleteBranch::OnOK()
{
	int sel = m_comboBranch.GetCurSel();
	if (sel < 0 || sel >= (int)m_branches.size())
	{
		AfxMessageBox(_T("Оберіть філію для видалення."));
		return;
	}

	m_selectedId = m_branches[sel].id;

	CString confirm;
	confirm.Format(_T("Видалити філію \"%s\"?\nЦю дію не можна скасувати!"), m_branches[sel].name.GetString());
	if (AfxMessageBox(confirm, MB_YESNO | MB_ICONWARNING) != IDYES)
		return;

	CDialogEx::OnOK();
}
