#pragma once
#include "afxdialogex.h"


// Диалоговое окно DDeleteReview

class DDeleteReview : public CDialogEx
{
	DECLARE_DYNAMIC(DDeleteReview)

public:
	DDeleteReview(CWnd* pParent = nullptr);   // стандартный конструктор
	virtual ~DDeleteReview();
	void SetPrompt(const CString& prompt) { m_prompt = prompt; }

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DELETE_REVIEW };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // поддержка DDX/DDV
	virtual BOOL OnInitDialog();

private:
	CString m_prompt;

	DECLARE_MESSAGE_MAP()
};
