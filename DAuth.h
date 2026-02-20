#pragma once
#include "afxdialogex.h"


// Диалоговое окно DAuth

class DAuth : public CDialogEx
{
	DECLARE_DYNAMIC(DAuth)

public:
	DAuth(BOOL isRegistrationMode, CWnd* pParent = nullptr);   // стандартный конструктор
	virtual ~DAuth();

	CString GetLogin() const { return m_login; }
	CString GetPassword() const { return m_password; }
	CString GetEmail() const { return m_email; }
	CString GetPhone() const { return m_phone; }
	CString GetFullName() const { return m_fullName; }

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_AUTH };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // поддержка DDX/DDV
	virtual BOOL OnInitDialog();
	virtual void OnOK();

private:
	BOOL m_isRegistrationMode = FALSE;
	CString m_login;
	CString m_password;
	CString m_email;
	CString m_phone;
	CString m_fullName;

	CEdit m_editLogin;
	CEdit m_editPassword;
	CEdit m_editEmail;
	CEdit m_editPhone;
	CEdit m_editFullName;

	DECLARE_MESSAGE_MAP()
};
