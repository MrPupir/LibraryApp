#pragma once
#include "afxdialogex.h"
#include <vector>

class DUsersAndReservations : public CDialogEx
{
	DECLARE_DYNAMIC(DUsersAndReservations)

public:
	DUsersAndReservations(CWnd* pParent = nullptr);
	virtual ~DUsersAndReservations();
	void SetRows(const std::vector<CString>& rows) { m_rows = rows; }

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_USERS_AND_RESERVATIONS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg void OnNMDblClkListReservation(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedButtonChangeStatus();
	afx_msg void OnSize(UINT nType, int cx, int cy);

private:
	void AdjustColumns();
	CString PickStatus(const CString& currentRawStatus);
	std::vector<CString> m_rows;
	CListCtrl m_reservationList;
	CButton m_btnChangeStatus;
	DECLARE_MESSAGE_MAP()
};