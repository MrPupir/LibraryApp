// DEditBranch.cpp: файл реализации

#include "pch.h"
#include "LibraryApp.h"
#include "afxdialogex.h"
#include "DEditBranch.h"

IMPLEMENT_DYNAMIC(DEditBranch, CDialogEx)

DEditBranch::DEditBranch(CWnd* pParent)
	: CDialogEx(IDD_EDIT_BRANCH, pParent)
	, m_selectedId(0)
{
}

DEditBranch::~DEditBranch()
{
}

void DEditBranch::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_BRANCH_SELECT, m_comboBranch);
	DDX_Control(pDX, IDC_EDIT_BRANCH_NAME, m_editName);
	DDX_Control(pDX, IDC_EDIT_BRANCH_ADDRESS, m_editAddress);
}

BEGIN_MESSAGE_MAP(DEditBranch, CDialogEx)
	ON_CBN_SELCHANGE(IDC_COMBO_BRANCH_SELECT, &DEditBranch::OnCbnSelChangeBranch)
END_MESSAGE_MAP()

BOOL DEditBranch::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_branches = Database::Instance().GetAllBranches();
	for (const auto& br : m_branches)
	{
		CString label;
		label.Format(_T("[%d] %s"), br.id, br.name.GetString());
		m_comboBranch.AddString(label);
	}

	if (!m_branches.empty())
	{
		m_comboBranch.SetCurSel(0);
		OnCbnSelChangeBranch();
	}

	return TRUE;
}

void DEditBranch::OnCbnSelChangeBranch()
{
	int sel = m_comboBranch.GetCurSel();
	if (sel >= 0 && sel < (int)m_branches.size())
	{
		m_editName.SetWindowText(m_branches[sel].name);
		m_editAddress.SetWindowText(m_branches[sel].address);
	}
}

void DEditBranch::OnOK()
{
	int sel = m_comboBranch.GetCurSel();
	if (sel < 0 || sel >= (int)m_branches.size())
	{
		AfxMessageBox(_T("Оберіть філію."));
		return;
	}

	m_selectedId = m_branches[sel].id;
	m_editName.GetWindowText(m_name);
	m_editAddress.GetWindowText(m_address);
	m_name.Trim();
	m_address.Trim();

	if (m_name.IsEmpty())
	{
		AfxMessageBox(_T("Введіть назву філії."));
		return;
	}
	if (m_address.IsEmpty())
	{
		AfxMessageBox(_T("Введіть адресу філії."));
		return;
	}
	if (m_name.GetLength() > 120 || m_address.GetLength() > 255)
	{
		AfxMessageBox(_T("Назва/адреса філії перевищують допустиму довжину."));
		return;
	}

	CDialogEx::OnOK();
}
