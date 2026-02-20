#pragma once
#include "afxdialogex.h"


// Диалоговое окно DAddReview

class DAddReview : public CDialogEx
{
	DECLARE_DYNAMIC(DAddReview)

public:
	DAddReview(CWnd* pParent = nullptr);   // стандартный конструктор
	virtual ~DAddReview();

	int GetRating() const { return m_rating; }
	CString GetComment() const { return m_comment; }

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ADD_REVIEW };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // поддержка DDX/DDV
	virtual BOOL OnInitDialog();
	virtual void OnOK();

private:
	int m_rating = 5;
	CString m_comment;
	CComboBox m_comboRating;
	CEdit m_editComment;

	DECLARE_MESSAGE_MAP()
};
