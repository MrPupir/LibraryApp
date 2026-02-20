#pragma once
#include "afxdialogex.h"


// Диалоговое окно DEditReview

class DEditReview : public CDialogEx
{
	DECLARE_DYNAMIC(DEditReview)

public:
	DEditReview(CWnd* pParent = nullptr);   // стандартный конструктор
	virtual ~DEditReview();

	void SetInitialData(int rating, const CString& comment);
	int GetRating() const { return m_rating; }
	CString GetComment() const { return m_comment; }

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EDIT_REVIEW };
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
