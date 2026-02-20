#pragma once
#include "afxdialogex.h"


// Диалоговое окно DReservation

class DReservation : public CDialogEx
{
	DECLARE_DYNAMIC(DReservation)

public:
	DReservation(CWnd* pParent = nullptr);   // стандартный конструктор
	virtual ~DReservation();

	CString GetBranch() const { return m_branch; }
	int GetDays() const { return m_days; }

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_RESERVATION };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // поддержка DDX/DDV
	virtual BOOL OnInitDialog();
	virtual void OnOK();

private:
	CString m_branch;
	int m_days = 7;
	CComboBox m_comboBranch;
	CEdit m_editDays;

	DECLARE_MESSAGE_MAP()
};
