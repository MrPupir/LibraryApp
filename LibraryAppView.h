
// LibraryAppView.h: интерфейс класса CLibraryAppView
//

#pragma once
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#include <GL/GL.h>
#include <GL/GLU.h>
#include <atlimage.h>
#include <vector>
#include <string>
#include <windows.h>
#include "Database.h"
#include "Filter.h"

struct BookData {
    int id;
    int texIndex;
    CString title;
    CString description;
    CString year;
    CString coverPath;
    float r, g, b;

    float curX, curY, curZ;
    float curRotX, curRotY, curRotZ;

    float targetX, targetY, targetZ;
    float targetRotX, targetRotY, targetRotZ;
};

enum ViewScreenMode
{
    SCREEN_AUTH_REQUIRED = 0,
    SCREEN_BOOK_LIST = 1,
    SCREEN_PROFILE = 2,
    SCREEN_ANALYTICS = 3,
    SCREEN_BOOK_DETAILS = 4,
    SCREEN_3D_INSPECT = 5
};

class CLibraryAppView : public CView
{
protected:
    CLibraryAppView() noexcept;
    DECLARE_DYNCREATE(CLibraryAppView)

public:
    CLibraryAppDoc* GetDocument() const;
    virtual ~CLibraryAppView();

    HGLRC m_hRC;
    CDC* m_pDC;

    std::vector<BookData> m_books;
    std::vector<BookRecord> m_filteredBooks;
    std::vector<BookRecord> m_pageBooks;
    std::vector<GLuint> m_texCovers;
    std::vector<COLORREF> m_coverAvgColors;
    CRect m_tableRect;
    int m_tableHeaderH;
    int m_tableRowH;
    int m_tableHoverRow;

    CFont m_fontNormal;
    CFont m_fontBold;
	CFont m_fontTitle;

    int m_hoverIndex;
    int m_selectedIndex;
    bool m_bInspectMode;

    bool m_hoverBtnPrev;
    bool m_hoverBtnNext;

    float m_scrollOffset;
    float m_inspectRotX;
    float m_inspectRotY;
    float m_inspectDist;

    CPoint m_lastMousePos;
    bool m_isDragging;

    float m_bookAspect;
    GLuint m_listBase;
    float m_glTextScale3D;
    GLuint m_texPaper;
    GLYPHMETRICSFLOAT m_glyphMetrics[256];

    Database& m_db;

    ViewScreenMode m_screen;
    int m_currentPage;
    int m_pageSize;
    int m_totalPages;

    bool m_chartPieMode;
    bool m_chart3DMode;
    float m_chartRotX;
    float m_chartRotY;
    float m_chartZoom;
    bool m_chartDragging;
    int m_chartDataIndex;

    int m_contextBookId;

    int m_detailsBookId;
    float m_detailsScrollY;
    float m_profileScrollY;
    Filter m_filter;

    virtual void OnDraw(CDC* pDC);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnTimer(UINT_PTR nIDEvent);

    afx_msg void OnCmdLogin();
    afx_msg void OnCmdRegister();
    afx_msg void OnCmdLogout();
    afx_msg void OnCmdMainMenu();
    afx_msg void OnCmdProfile();
    afx_msg void OnCmdFilterSort();

    afx_msg void OnCmdAddBook();
    afx_msg void OnCmdEditBook();
    afx_msg void OnCmdDeleteBook();
    afx_msg void OnCmdUsersReservations();

    afx_msg void OnCmdAddBranch();
    afx_msg void OnCmdEditBranch();
    afx_msg void OnCmdDeleteBranch();

    afx_msg void OnCmdAddCategory();
    afx_msg void OnCmdEditCategory();
    afx_msg void OnCmdDeleteCategory();

    afx_msg void OnCmdAnalyticsView();
    afx_msg void OnCmdAnalyticsChartType();
    afx_msg void OnCmdAnalyticsDimension();
    afx_msg void OnCmdAnalyticsReset();

    afx_msg void OnContextReserve();
    afx_msg void OnContextCancelReservation();
    afx_msg void OnContextReview();
    afx_msg void OnContextDetails();
    afx_msg void OnContext3DInspect();
    afx_msg void OnContextProfileEditReview();
    afx_msg void OnContextProfileDeleteReview();
    afx_msg void OnContextProfileCancelReservation();
    afx_msg void OnContextProfileReportLost();
    afx_msg void OnContextProfileReportDamaged();
    int m_profileContextReservationId;
    int m_profileContextReviewId;
    CString m_profileContextReservationStatus;

protected:
    DECLARE_MESSAGE_MAP()
    BOOL SetupPixelFormat();
    void LoadTextures();
    void InitBooks();
    void DrawBookModel(float w, float h, float d, int texIdx, float r, float g, float b);
    void DrawBook(float w, float h, float d, int texIdx, float r, float g, float b);
    void DrawCharts();
    void DrawShelf(float width, float depth);
    void RenderScene(bool bPicking);
    int PickBook(CPoint point);
    void DrawInfoOverlay(CDC* pDC);
    void UpdateAnimations();
    void ReloadBooksPage();
    void RenderAnalyticsScene();
    void DrawOverlayCentered(CDC* pDC, const CString& text, int yOffset = 0);
    void DrawProfileOverlay(CDC* pDC);
    void DrawAnalyticsOverlay(CDC* pDC);
    void DrawBookDetailsScreen(CDC* pDC);
    void Draw3DInspectScene(bool bPicking);
    int HitTestTableRow(CPoint point) const;
    CString ResolveCoverPath(const CString& dbPath) const;
    GLuint LoadTextureFromFile(const CString& path, COLORREF* outAvgColor = nullptr);
    void ShowBookContextMenu(CPoint screenPoint);
    void ShowProfileContextMenu(CPoint screenPoint);
    void BeginOverlay2D();
    void EndOverlay2D();
    void DrawTextGL(int x, int y, const CString& text, float r, float g, float b);
    void DrawText3D(double x, double y, double z, const CString& text, float r, float g, float b, double scale = 1.0);
    float Measure3DTextWidth(const CString& text, double scale = 1.0);
    CSize MeasureTextGL(const CString& text) const;
    GLuint LoadPaperTexture();
    bool EnsureLoggedIn();
    bool EnsureAdmin();
};

#ifndef _DEBUG
inline CLibraryAppDoc* CLibraryAppView::GetDocument() const
{
    return reinterpret_cast<CLibraryAppDoc*>(m_pDocument);
}
#endif
