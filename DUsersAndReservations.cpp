// DUsersAndReservations.cpp: файл реализации

#include "pch.h"
#include "LibraryApp.h"
#include "afxdialogex.h"
#include "DUsersAndReservations.h"
#include "Database.h"

IMPLEMENT_DYNAMIC(DUsersAndReservations, CDialogEx)

namespace
{
struct StatusItem { UINT id; LPCTSTR raw; LPCTSTR label; };

const StatusItem kStatusItems[] = {
	{ 60001, _T("reserved"), _T("Зарезервовано") },
	{ 60002, _T("ready_for_pickup"), _T("Готово до видачі") },
	{ 60003, _T("issued"), _T("Видано") },
	{ 60004, _T("overdue"), _T("Прострочено") },
	{ 60005, _T("return_requested"), _T("Запит на повернення") },
	{ 60006, _T("returned"), _T("Повернуто") },
	{ 60007, _T("lost"), _T("Втрачено") },
	{ 60008, _T("damaged"), _T("Пошкоджено") },
	{ 60009, _T("cancelled"), _T("Скасовано") },
	{ 60010, _T("expired"), _T("Термін минув") }
};

CString StatusLabelByRaw(const CString& raw)
{
	CString s = raw;
	s.MakeLower();
	for (const auto& item : kStatusItems)
	{
		if (s == item.raw) return item.label;
	}
	return raw;
}
}

DUsersAndReservations::DUsersAndReservations(CWnd* pParent)
	: CDialogEx(IDD_USERS_AND_RESERVATIONS, pParent)
{
}

DUsersAndReservations::~DUsersAndReservations()
{
}

void DUsersAndReservations::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_USERS_RES, m_reservationList);
	DDX_Control(pDX, IDC_BUTTON_CHANGE_STATUS, m_btnChangeStatus);
}


BEGIN_MESSAGE_MAP(DUsersAndReservations, CDialogEx)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_USERS_RES, &DUsersAndReservations::OnNMDblClkListReservation)
	ON_BN_CLICKED(IDC_BUTTON_CHANGE_STATUS, &DUsersAndReservations::OnBnClickedButtonChangeStatus)
	ON_WM_SIZE()
END_MESSAGE_MAP()


BOOL DUsersAndReservations::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	m_reservationList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_reservationList.InsertColumn(0, _T("Користувач"), LVCFMT_LEFT, 110);
	m_reservationList.InsertColumn(1, _T("Книга"), LVCFMT_LEFT, 130);
	m_reservationList.InsertColumn(2, _T("Філія"), LVCFMT_LEFT, 95);
	m_reservationList.InsertColumn(3, _T("Дата броні"), LVCFMT_LEFT, 70);
	m_reservationList.InsertColumn(4, _T("Повернення"), LVCFMT_LEFT, 70);
	m_reservationList.InsertColumn(5, _T("Статус"), LVCFMT_LEFT, 90);
	m_reservationList.InsertColumn(6, _T("raw"), LVCFMT_LEFT, 0);

	for (int i = 0; i < (int)m_rows.size(); ++i)
	{
		const CString& row = m_rows[i];
		CString cols[8];
		int pos = 0;
		int colIdx = 0;
		while (colIdx < 8 && pos < row.GetLength())
		{
			int delimPos = row.Find(_T("|"), pos);
			if (delimPos == -1) delimPos = row.GetLength();
			cols[colIdx] = row.Mid(pos, delimPos - pos);
			cols[colIdx].Trim();
			pos = delimPos + 1;
			colIdx++;
		}

		if (colIdx < 7)
		{
			continue;
		}
		if (colIdx < 8)
		{
			cols[7] = cols[6];
		}

		int idx = m_reservationList.InsertItem(i, cols[1]);
		m_reservationList.SetItemData(idx, (DWORD_PTR)_ttoi(cols[0]));
		m_reservationList.SetItemText(idx, 1, cols[2]);
		m_reservationList.SetItemText(idx, 2, cols[3]);
		m_reservationList.SetItemText(idx, 3, cols[4]);
		m_reservationList.SetItemText(idx, 4, cols[5]);
		m_reservationList.SetItemText(idx, 5, cols[7]);
		m_reservationList.SetItemText(idx, 6, cols[6]);
	}

	AdjustColumns();

	if (m_reservationList.GetItemCount() > 0)
	{
		m_reservationList.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		m_reservationList.SetSelectionMark(0);
	}
	
	return TRUE;
}

void DUsersAndReservations::OnNMDblClkListReservation(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = pNMIA ? pNMIA->iItem : m_reservationList.GetSelectionMark();
	if (nItem >= 0 && nItem < m_reservationList.GetItemCount())
	{
		CString userDisplay = m_reservationList.GetItemText(nItem, 0);
		UserRecord user;
		if (Database::Instance().GetUserByDisplayName(userDisplay, user))
		{
			CString info;
			info.Format(
				_T("Логін: %s\nПІБ: %s\nЕ-пошта: %s\nТелефон: %s\nРоль: %s"),
				user.login.GetString(),
				user.fullName.GetString(),
				user.email.GetString(),
				user.phone.GetString(),
				user.role.GetString());
			AfxMessageBox(info);
		}
		else
		{
			AfxMessageBox(_T("Не вдалося відкрити профіль користувача."));
		}
	}
	*pResult = 0;
}

void DUsersAndReservations::OnBnClickedButtonChangeStatus()
{
	int nItem = m_reservationList.GetSelectionMark();
	if (nItem >= 0 && nItem < m_reservationList.GetItemCount())
	{
		CString currentRaw = m_reservationList.GetItemText(nItem, 6);
		CString newStatusDb = PickStatus(currentRaw);
		if (newStatusDb.IsEmpty()) return;

		const int reservationId = (int)m_reservationList.GetItemData(nItem);
		if (reservationId <= 0)
		{
			AfxMessageBox(_T("Некоректний ID бронювання."));
			return;
		}

		Database::Instance().UpdateReservationStatus(reservationId, newStatusDb);
		CString refreshed = StatusLabelByRaw(newStatusDb);

		m_reservationList.SetItemText(nItem, 6, newStatusDb);
		m_reservationList.SetItemText(nItem, 5, refreshed);
		AfxMessageBox(_T("Статус змінено."));
	}
	else {
		AfxMessageBox(_T("Будь ласка, виберіть запис"));
	}
}

void DUsersAndReservations::AdjustColumns()
{
	if (!::IsWindow(m_reservationList.GetSafeHwnd())) return;
	CRect rc;
	m_reservationList.GetClientRect(&rc);
	int total = max(120, rc.Width() - 6);
	int w0 = (total * 18) / 100;
	int w1 = (total * 26) / 100;
	int w2 = (total * 14) / 100;
	int w3 = (total * 14) / 100;
	int w4 = (total * 14) / 100;
	int w5 = total - (w0 + w1 + w2 + w3 + w4);
	m_reservationList.SetColumnWidth(0, w0);
	m_reservationList.SetColumnWidth(1, w1);
	m_reservationList.SetColumnWidth(2, w2);
	m_reservationList.SetColumnWidth(3, w3);
	m_reservationList.SetColumnWidth(4, w4);
	m_reservationList.SetColumnWidth(5, max(90, w5));
	m_reservationList.SetColumnWidth(6, 0);
}

CString DUsersAndReservations::PickStatus(const CString& currentRawStatus)
{
	CMenu menu;
	menu.CreatePopupMenu();
	CString current = currentRawStatus;
	current.MakeLower();

	for (const auto& item : kStatusItems)
	{
		UINT flags = MF_STRING;
		if (current == item.raw) flags |= MF_CHECKED;
		CString text;
		text.Format(_T("%s  [%s]"), item.label, item.raw);
		menu.AppendMenu(flags, item.id, text);
	}

	CPoint pt;
	::GetCursorPos(&pt);
	UINT cmd = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, this);
	for (const auto& item : kStatusItems)
	{
		if (item.id == cmd) return item.raw;
	}
	return CString();
}

void DUsersAndReservations::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	if (!::IsWindow(m_reservationList.GetSafeHwnd()) || !::IsWindow(m_btnChangeStatus.GetSafeHwnd())) return;

	const int margin = 6;
	const int btnH = 22;
	const int bottomY = max(margin, cy - btnH - margin);
	m_reservationList.MoveWindow(margin, margin, max(120, cx - margin * 2), max(120, bottomY - margin));
	m_btnChangeStatus.MoveWindow(margin, bottomY, 160, btnH);
	CWnd* closeBtn = GetDlgItem(IDOK);
	if (closeBtn)
	{
		closeBtn->MoveWindow(max(margin, cx - 90 - margin), bottomY, 90, btnH);
	}
	AdjustColumns();
}