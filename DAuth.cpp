#include "pch.h"
#include "LibraryApp.h"
#include "afxdialogex.h"
#include "DAuth.h"

IMPLEMENT_DYNAMIC(DAuth, CDialogEx)

DAuth::DAuth(BOOL isRegistrationMode, CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_AUTH, pParent)
    , m_isRegistrationMode(isRegistrationMode)
{
}

DAuth::~DAuth()
{
}

void DAuth::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_LOGIN, m_editLogin);
    DDX_Control(pDX, IDC_EDIT_PASSWORD, m_editPassword);
    DDX_Control(pDX, IDC_EDIT_EMAIL, m_editEmail);
    DDX_Control(pDX, IDC_EDIT_PHONE, m_editPhone);
    DDX_Control(pDX, IDC_EDIT_FULLNAME, m_editFullName);
}

BEGIN_MESSAGE_MAP(DAuth, CDialogEx)
END_MESSAGE_MAP()

BOOL DAuth::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    SetWindowTextW(m_isRegistrationMode ? _T("Реєстрація") : _T("Вхід"));
    if (CWnd* pOk = GetDlgItem(IDOK))
    {
        pOk->SetWindowTextW(m_isRegistrationMode ? _T("Зареєструватися") : _T("Увійти"));
    }

    const int show = m_isRegistrationMode ? SW_SHOW : SW_HIDE;
    GetDlgItem(IDC_EDIT_EMAIL)->ShowWindow(show);
    GetDlgItem(IDC_EDIT_PHONE)->ShowWindow(show);
    GetDlgItem(IDC_EDIT_FULLNAME)->ShowWindow(show);
    GetDlgItem(IDC_LBL_EMAIL)->ShowWindow(show);
    GetDlgItem(IDC_LBL_PHONE)->ShowWindow(show);
    GetDlgItem(IDC_LBL_FULLNAME)->ShowWindow(show);

    if (!m_isRegistrationMode)
    {
        CWnd* pPassword = GetDlgItem(IDC_EDIT_PASSWORD);
        CWnd* pOk = GetDlgItem(IDOK);
        CWnd* pCancel = GetDlgItem(IDCANCEL);

        if (pPassword && pOk && pCancel)
        {
            CRect rcPassword;
            CRect rcOk;
            CRect rcCancel;

            pPassword->GetWindowRect(&rcPassword);
            pOk->GetWindowRect(&rcOk);
            pCancel->GetWindowRect(&rcCancel);

            ScreenToClient(&rcPassword);
            ScreenToClient(&rcOk);
            ScreenToClient(&rcCancel);

            const int buttonsTop = rcPassword.bottom + 12;
            pOk->SetWindowPos(nullptr, rcOk.left, buttonsTop, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
            pCancel->SetWindowPos(nullptr, rcCancel.left, buttonsTop, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

            pOk->GetWindowRect(&rcOk);
            pCancel->GetWindowRect(&rcCancel);
            ScreenToClient(&rcOk);
            ScreenToClient(&rcCancel);

            const int buttonsBottom = (rcOk.bottom > rcCancel.bottom) ? rcOk.bottom : rcCancel.bottom;
            const int newClientHeight = buttonsBottom + 12;

            CRect rcClient;
            CRect rcWindow;
            GetClientRect(&rcClient);
            GetWindowRect(&rcWindow);

            const int frameHeight = rcWindow.Height() - rcClient.Height();
            SetWindowPos(nullptr, 0, 0, rcWindow.Width(), newClientHeight + frameHeight, SWP_NOMOVE | SWP_NOZORDER);
        }
    }

    return TRUE;
}

void DAuth::OnOK()
{
    m_editLogin.GetWindowTextW(m_login);
    m_editPassword.GetWindowTextW(m_password);
    m_editEmail.GetWindowTextW(m_email);
    m_editPhone.GetWindowTextW(m_phone);
    m_editFullName.GetWindowTextW(m_fullName);

    m_login.Trim();
    m_password.Trim();
    m_email.Trim();
    m_phone.Trim();
    m_fullName.Trim();

    if (!m_isRegistrationMode)
    {
        if (m_login.IsEmpty() || m_password.IsEmpty())
        {
            AfxMessageBox(_T("Невірний логін або пароль."));
            return;
        }

        CDialogEx::OnOK();
        return;
    }

    if (m_login.IsEmpty() || m_password.IsEmpty())
    {
        AfxMessageBox(_T("Введіть логін і пароль."));
        return;
    }

    if (m_login.GetLength() < 3 || m_login.GetLength() > 64)
    {
        AfxMessageBox(_T("Логін має містити від 3 до 64 символів."));
        return;
    }

    if (m_password.GetLength() < 6 || m_password.GetLength() > 255)
    {
        AfxMessageBox(_T("Пароль має містити від 6 до 255 символів."));
        return;
    }

    if (m_email.IsEmpty() || m_fullName.IsEmpty())
    {
        AfxMessageBox(_T("Для реєстрації заповніть email та ПІБ."));
        return;
    }

    int at = m_email.Find(_T('@'));
    int dot = m_email.ReverseFind(_T('.'));
    if (at <= 0 || dot <= at + 1 || dot >= m_email.GetLength() - 1 || m_email.Find(_T(' ')) >= 0)
    {
        AfxMessageBox(_T("Некоректний email."));
        return;
    }

    if (m_fullName.GetLength() < 3 || m_fullName.GetLength() > 150)
    {
        AfxMessageBox(_T("ПІБ має містити від 3 до 150 символів."));
        return;
    }

    if (!m_phone.IsEmpty())
    {
        int digits = 0;
        for (int i = 0; i < m_phone.GetLength(); ++i)
        {
            WCHAR ch = m_phone[i];
            if (ch >= _T('0') && ch <= _T('9')) digits++;
        }
        if (digits > 0 && digits < 7)
        {
            AfxMessageBox(_T("Телефон має містити щонайменше 7 цифр."));
            return;
        }
    }

    CDialogEx::OnOK();
}
