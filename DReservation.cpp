// DReservation.cpp: файл реализації

#include "pch.h"
#include "LibraryApp.h"
#include "afxdialogex.h"
#include "DReservation.h"
#include "Database.h"

IMPLEMENT_DYNAMIC(DReservation, CDialogEx)

DReservation::DReservation(CWnd* pParent)
	: CDialogEx(IDD_RESERVATION, pParent)
{
}

DReservation::~DReservation()
{
}

void DReservation::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_BRANCH, m_comboBranch);
	DDX_Control(pDX, IDC_EDIT_DAYS, m_editDays);
}

BEGIN_MESSAGE_MAP(DReservation, CDialogEx)
END_MESSAGE_MAP()

BOOL DReservation::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Load branches from DB
	auto branches = Database::Instance().GetBranches();
	for (const auto& b : branches) {
		m_comboBranch.AddString(b);
	}

	if (branches.empty()) {
		m_comboBranch.AddString(_T("(немає філій)"));
	}

	m_comboBranch.SetCurSel(0);
	SetDlgItemInt(IDC_EDIT_DAYS, m_days);

	return TRUE;
}

void DReservation::OnOK()
{
	int sel = m_comboBranch.GetCurSel();
	if (sel == CB_ERR)
	{
		AfxMessageBox(_T("Оберіть філію."));
		return;
	}

	m_comboBranch.GetLBText(sel, m_branch);
	m_days = GetDlgItemInt(IDC_EDIT_DAYS, NULL, FALSE);
	if (m_days <= 0)
	{
		AfxMessageBox(_T("Кількість днів має бути більшою за 0."));
		return;
	}
	if (m_days > 30)
	{
		AfxMessageBox(_T("Максимум 30 днів."));
		return;
	}

	CDialogEx::OnOK();
}
