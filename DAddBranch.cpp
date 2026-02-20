#include "pch.h"
#include "LibraryApp.h"
#include "afxdialogex.h"
#include "DAddBranch.h"

IMPLEMENT_DYNAMIC(DAddBranch, CDialogEx)

DAddBranch::DAddBranch(CWnd* pParent)
	: CDialogEx(IDD_ADD_BRANCH, pParent)
{
}

DAddBranch::~DAddBranch()
{
}

void DAddBranch::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_BRANCH_NAME, m_editName);
	DDX_Control(pDX, IDC_EDIT_BRANCH_ADDRESS, m_editAddress);
}

BEGIN_MESSAGE_MAP(DAddBranch, CDialogEx)
END_MESSAGE_MAP()

BOOL DAddBranch::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	return TRUE;
}

void DAddBranch::OnOK()
{
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
