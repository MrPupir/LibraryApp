#include "pch.h"
#include "framework.h"

#ifndef SHARED_HANDLERS
#include "LibraryApp.h"
#endif

#include "LibraryAppDoc.h"
#include "LibraryAppView.h"
#include "DAddBook.h"
#include "DAuth.h"
#include "DDeleteBook.h"
#include "DEditBook.h"
#include "DReservation.h"
#include "DAddReview.h"
#include "DEditReview.h"
#include "DDeleteReview.h"
#include "DUsersAndReservations.h"
#include "DAddBranch.h"
#include "DEditBranch.h"
#include "DDeleteBranch.h"
#include "DAddCategory.h"
#include "DEditCategory.h"
#include "DDeleteCategory.h"
#include <algorithm>
#include <cmath>
#include <float.h>
#include <set>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define GLF_START_LIST 1000

enum {
    ID_CONTEXT_RESERVE = 50001, ID_CONTEXT_REVIEW = 50002, ID_CONTEXT_DETAILS = 50003, ID_CONTEXT_3D_INSPECT = 50004, ID_CONTEXT_CANCEL_RESERVATION = 50005
};

enum {
    ID_CONTEXT_PROFILE_EDIT_REVIEW = 50021,
    ID_CONTEXT_PROFILE_DELETE_REVIEW = 50022,
    ID_CONTEXT_PROFILE_CANCEL_RESERVATION = 50023,
    ID_CONTEXT_PROFILE_REPORT_LOST = 50024,
    ID_CONTEXT_PROFILE_REPORT_DAMAGED = 50025
};

enum {
    ID_FILTER_PREV_CATEGORY = 51001,
    ID_FILTER_NEXT_CATEGORY = 51002,
    ID_FILTER_NEXT_SORT = 51003,
    ID_FILTER_TOGGLE_DIR = 51004,
    ID_FILTER_RESET = 51005
};

IMPLEMENT_DYNCREATE(CLibraryAppView, CView)

BEGIN_MESSAGE_MAP(CLibraryAppView, CView)
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_LBUTTONUP()
    ON_WM_RBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_MOUSEWHEEL()
    ON_WM_KEYDOWN()
    ON_WM_TIMER()
    ON_WM_CONTEXTMENU()
    ON_COMMAND(ID_32771, &CLibraryAppView::OnCmdLogin)
    ON_COMMAND(ID_32772, &CLibraryAppView::OnCmdRegister)
    ON_COMMAND(ID_32773, &CLibraryAppView::OnCmdLogout)
    ON_COMMAND(ID_32775, &CLibraryAppView::OnCmdMainMenu)
    ON_COMMAND(ID_32776, &CLibraryAppView::OnCmdProfile)
    ON_COMMAND(ID_32777, &CLibraryAppView::OnCmdAddBook)
    ON_COMMAND(ID_32778, &CLibraryAppView::OnCmdEditBook)
    ON_COMMAND(ID_32779, &CLibraryAppView::OnCmdDeleteBook)
    ON_COMMAND(ID_32780, &CLibraryAppView::OnCmdUsersReservations)
    ON_COMMAND(ID_32784, &CLibraryAppView::OnCmdAnalyticsView)
    ON_COMMAND(ID_32781, &CLibraryAppView::OnCmdAnalyticsChartType)
    ON_COMMAND(ID_32782, &CLibraryAppView::OnCmdAnalyticsDimension)
    ON_COMMAND(ID_32783, &CLibraryAppView::OnCmdAnalyticsReset)
    ON_COMMAND(ID_CONTEXT_RESERVE, &CLibraryAppView::OnContextReserve)
    ON_COMMAND(ID_CONTEXT_CANCEL_RESERVATION, &CLibraryAppView::OnContextCancelReservation)
    ON_COMMAND(ID_CONTEXT_REVIEW, &CLibraryAppView::OnContextReview)
    ON_COMMAND(ID_CONTEXT_DETAILS, &CLibraryAppView::OnContextDetails)
    ON_COMMAND(ID_CONTEXT_3D_INSPECT, &CLibraryAppView::OnContext3DInspect)
    ON_COMMAND(ID_CONTEXT_PROFILE_EDIT_REVIEW, &CLibraryAppView::OnContextProfileEditReview)
    ON_COMMAND(ID_CONTEXT_PROFILE_DELETE_REVIEW, &CLibraryAppView::OnContextProfileDeleteReview)
    ON_COMMAND(ID_CONTEXT_PROFILE_CANCEL_RESERVATION, &CLibraryAppView::OnContextProfileCancelReservation)
    ON_COMMAND(ID_CONTEXT_PROFILE_REPORT_LOST, &CLibraryAppView::OnContextProfileReportLost)
    ON_COMMAND(ID_CONTEXT_PROFILE_REPORT_DAMAGED, &CLibraryAppView::OnContextProfileReportDamaged)
    ON_COMMAND(ID_32785, &CLibraryAppView::OnCmdAddBranch)
    ON_COMMAND(ID_32786, &CLibraryAppView::OnCmdEditBranch)
    ON_COMMAND(ID_32787, &CLibraryAppView::OnCmdDeleteBranch)
    ON_COMMAND(ID_32788, &CLibraryAppView::OnCmdAddCategory)
    ON_COMMAND(ID_32789, &CLibraryAppView::OnCmdEditCategory)
    ON_COMMAND(ID_32790, &CLibraryAppView::OnCmdDeleteCategory)
END_MESSAGE_MAP()

CLibraryAppView::CLibraryAppView() noexcept : m_db(Database::Instance()) {
    m_hRC = NULL;
    m_pDC = NULL;
    m_hoverIndex = -1;
    m_selectedIndex = -1;
    m_bInspectMode = false;
    m_hoverBtnPrev = false;
    m_hoverBtnNext = false;
    m_scrollOffset = 0.0f;
    m_inspectRotX = 0.0f;
    m_inspectRotY = 0.0f;
    m_inspectDist = -4.0f;
    m_isDragging = false;
    m_bookAspect = 0.75f;
    m_screen = SCREEN_AUTH_REQUIRED;
    m_currentPage = 0;
    m_pageSize = 3;
    m_totalPages = 1;
    m_chartPieMode = false;
    m_chart3DMode = true;
    m_chartRotX = 20.0f;
    m_chartRotY = -20.0f;
    m_chartZoom = -12.0f;
    m_chartDragging = false;
    m_contextBookId = 0;
    m_profileContextReservationId = 0;
    m_profileContextReviewId = 0;

    m_tableHeaderH = 40;
    m_tableRowH = 30;
    m_tableHoverRow = -1;

    m_listBase = 0;
    m_glTextScale3D = 0.18f;
    m_texPaper = 0;
    memset(m_glyphMetrics, 0, sizeof(m_glyphMetrics));
    m_chartDataIndex = 0;

    m_detailsBookId = 0;
    m_detailsScrollY = 0.0f;
    m_profileScrollY = 0.0f;
    m_detailsCardW = 860;
    m_detailsCardH = 560;
}

CLibraryAppView::~CLibraryAppView() {}

BOOL CLibraryAppView::PreCreateWindow(CREATESTRUCT& cs) {
    cs.style |= CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    return CView::PreCreateWindow(cs);
}

int CLibraryAppView::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CView::OnCreate(lpCreateStruct) == -1) return -1;
    m_pDC = new CClientDC(this);
    if (!SetupPixelFormat()) return -1;
    m_hRC = wglCreateContext(m_pDC->GetSafeHdc());
    wglMakeCurrent(m_pDC->GetSafeHdc(), m_hRC);

    m_fontNormal.CreateFont(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, RUSSIAN_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FF_SWISS, _T("Segoe UI"));
    m_fontBold.CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, RUSSIAN_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FF_SWISS, _T("Segoe UI"));
    m_fontTitle.CreateFont(40, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, RUSSIAN_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FF_SWISS, _T("Segoe UI"));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_listBase = GLF_START_LIST;

    HDC hDC = m_pDC->GetSafeHdc();

    CFont font;
    font.CreateFont(
        -12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        RUSSIAN_CHARSET,
        OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY, FF_SWISS,
        _T("Arial"));

    CFont* pOldFont = m_pDC->SelectObject(&font);

    BOOL bResult = wglUseFontOutlinesA(
        hDC,
        0,
        256,
        m_listBase,
        0.0f,
        0.2f,
        WGL_FONT_POLYGONS,
        m_glyphMetrics);

    m_pDC->SelectObject(pOldFont);

    m_texPaper = LoadPaperTexture();
    m_filter.SetCategories(m_db.GetAllCategories());
    LoadTextures();
    ReloadBooksPage();
    SetTimer(1, 16, NULL);
    return 0;
}

BOOL CLibraryAppView::SetupPixelFormat() {
    static PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0,
        0,
        0,
        0,
        0,
        0,
        8,
        0,
        0,
        0,
        0,
        0,
        0,
        24,
        8,
        0,
        PFD_MAIN_PLANE,
        0,
        0,
        0,
        0
    };

    int pf = ChoosePixelFormat(m_pDC->GetSafeHdc(), &pfd);
    if (pf == 0) return FALSE;
    return SetPixelFormat(m_pDC->GetSafeHdc(), pf, &pfd);
}

void CLibraryAppView::OnDestroy() {
    for (GLuint tex : m_texCovers)
        if (tex) glDeleteTextures(1, &tex);
    if (m_texPaper) {
        glDeleteTextures(1, &m_texPaper);
        m_texPaper = 0;
    }
    KillTimer(1);
    m_fontNormal.DeleteObject();
    m_fontBold.DeleteObject();
    m_fontTitle.DeleteObject();
    if (wglGetCurrentContext() != NULL) wglMakeCurrent(NULL, NULL);
    if (m_hRC) {
        wglDeleteContext(m_hRC);
        m_hRC = NULL;
    }
    if (m_pDC) {
        delete m_pDC;
        m_pDC = NULL;
    }
    CView::OnDestroy();
}

void CLibraryAppView::OnSize(UINT nType, int cx, int cy) {
    CView::OnSize(nType, cx, cy);
    if (cy == 0) cy = 1;
    if (m_pDC && m_hRC)
    {
        wglMakeCurrent(m_pDC->GetSafeHdc(), m_hRC);
    }
    glViewport(0, 0, cx, cy);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)cx / (GLfloat)cy, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    int tableW = cx - 20;
    int tableX = 10;
    m_tableRect = CRect(tableX, 36, tableX + tableW, cy - 44);
    
    int tableHeight = m_tableRect.Height();
    int availableHeight = tableHeight - m_tableHeaderH - 4;
    int calculatedPageSize = max(1, availableHeight / m_tableRowH);
    
    if (calculatedPageSize != m_pageSize) {
        m_pageSize = calculatedPageSize;
        m_currentPage = 0;
        ReloadBooksPage();
    }

    m_detailsCardW = min(max(860, (int)(cx * 0.8)), cx - 24);
    m_detailsCardH = min(max(400, (int)(cy * 0.8)), cy - 24);
}

BOOL CLibraryAppView::OnEraseBkgnd(CDC* pDC) {
    (void)pDC;
    return TRUE;
}

CString CLibraryAppView::ResolveCoverPath(const CString& dbPath) const {

    CString path(dbPath);
    path.Trim();
    if (path.IsEmpty()) return CString();

    CString lower(path);
    lower.MakeLower();
    bool isUrl = (lower.Left(7) == _T("http://") || lower.Left(8) == _T("https://"));

    if (isUrl)
    {
        TCHAR exeBuf[MAX_PATH] = { 0 };
        ::GetModuleFileName(nullptr, exeBuf, MAX_PATH);
        CString appDir(exeBuf);
        int s = appDir.ReverseFind(_T('\\'));
        if (s > 0) appDir = appDir.Left(s);

        CString cacheDir = appDir + _T("\\books\\cache");
        ::CreateDirectory(appDir + _T("\\books"), nullptr);
        ::CreateDirectory(cacheDir, nullptr);

        unsigned int hash = 0;
        for (int i = 0; i < path.GetLength(); ++i)
            hash = hash * 31 + (unsigned int)path[i];

        CString cachedFile;
        cachedFile.Format(_T("%s\\%08X.jpg"), cacheDir.GetString(), hash);

        if (::GetFileAttributes(cachedFile) != INVALID_FILE_ATTRIBUTES)
            return cachedFile;

        try
        {
            CInternetSession session(_T("LibraryApp"));
            CStdioFile* pFile = session.OpenURL(path, 1, INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_RELOAD);
            if (pFile)
            {
                CFile local;
                if (local.Open(cachedFile, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary))
                {
                    BYTE buf[4096];
                    UINT n;
                    while ((n = pFile->Read(buf, sizeof(buf))) > 0)
                        local.Write(buf, n);
                    local.Close();
                }
                pFile->Close();
                delete pFile;
            }
            session.Close();
        }
        catch (CInternetException* e) { e->Delete(); }
        catch (CFileException* e) { e->Delete(); }

        if (::GetFileAttributes(cachedFile) != INVALID_FILE_ATTRIBUTES)
            return cachedFile;
        return CString();
    }

    if (::GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES) return path;

    TCHAR exeBuf[MAX_PATH] = { 0 };
    ::GetModuleFileName(nullptr, exeBuf, MAX_PATH);
    CString appDir(exeBuf);
    int slash = appDir.ReverseFind(_T('\\'));
    if (slash > 0) appDir = appDir.Left(slash);

    CString normalized = path;
    normalized.Replace(_T("/"), _T("\\"));

    CString prefixes[] = { _T("\\"), _T("\\..\\"), _T("\\..\\..\\"), _T("\\..\\..\\..\\") };
    for (const CString& p : prefixes)
    {
        CString full = appDir + p + normalized;
        if (::GetFileAttributes(full) != INVALID_FILE_ATTRIBUTES) return full;
    }
    return CString();
}

GLuint CLibraryAppView::LoadTextureFromFile(const CString& path, COLORREF* outAvgColor)
{
    if (outAvgColor) *outAvgColor = RGB(120, 135, 160);

    CString absPath = ResolveCoverPath(path);
    if (absPath.IsEmpty()) return 0;

    CImage image;
    if (FAILED(image.Load(absPath))) return 0;

    GLuint texID = 0;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    GLenum format = (image.GetBPP() == 32) ? GL_BGRA_EXT : GL_BGR_EXT;
    BYTE* pData = (BYTE*)image.GetBits();
    int w = image.GetWidth();
    int h = image.GetHeight();
    int pitch = image.GetPitch();

    if (pitch < 0) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, format, GL_UNSIGNED_BYTE, pData + (h - 1) * pitch);
    }
    else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, format, GL_UNSIGNED_BYTE, pData);
    }

    if (outAvgColor) {
        long sumR = 0, sumG = 0, sumB = 0, pixels = 0;
        int regionW = min(16, w);
        int regionH = min(16, h);
        for (int y = 0; y < regionH; ++y) {
            for (int x = 0; x < regionW; ++x) {
                COLORREF c = image.GetPixel(x, y);
                if (c == CLR_INVALID) continue;
                sumR += GetRValue(c); sumG += GetGValue(c); sumB += GetBValue(c); pixels++;
            }
        }
        if (pixels > 0) *outAvgColor = RGB(sumR / pixels, sumG / pixels, sumB / pixels);
    }

    return texID;
}

GLuint CLibraryAppView::LoadPaperTexture()
{
    CBitmap bmp;
    if (!bmp.LoadBitmap(IDB_PAPER)) return 0;

    BITMAP bi;
    bmp.GetBitmap(&bi);
    int w = bi.bmWidth;
    int h = bi.bmHeight;

    std::vector<BYTE> bits(w * h * 4);
    BITMAPINFOHEADER bih = {};
    bih.biSize = sizeof(bih);
    bih.biWidth = w;
    bih.biHeight = h;
    bih.biPlanes = 1;
    bih.biBitCount = 32;
    bih.biCompression = BI_RGB;

    CDC* pDC = GetDC();
    ::GetDIBits(pDC->GetSafeHdc(), (HBITMAP)bmp, 0, h, &bits[0], (BITMAPINFO*)&bih, DIB_RGB_COLORS);
    ReleaseDC(pDC);

    for (int i = 0; i < w * h; ++i)
    {
        BYTE tmp = bits[i * 4];
        bits[i * 4] = bits[i * 4 + 2];
        bits[i * 4 + 2] = tmp;
        bits[i * 4 + 3] = 255;
    }

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &bits[0]);
    return tex;
}

void CLibraryAppView::LoadTextures() {
    for (GLuint tex : m_texCovers)
        if (tex != 0) glDeleteTextures(1, &tex);
    m_texCovers.clear();
    m_coverAvgColors.clear();
    for (int i = 0; i < (int)m_pageBooks.size(); ++i) {
        COLORREF avg = RGB(110, 128, 150);
        m_texCovers.push_back(LoadTextureFromFile(m_pageBooks[i].coverPath, &avg));
        m_coverAvgColors.push_back(avg);
    }
}

void CLibraryAppView::ReloadBooksPage() {
    m_books.clear();
    m_filteredBooks.clear();
    m_pageBooks.clear();

    std::vector<BookRecord> allBooks = m_db.GetAllBooks();
    for (const auto& b : allBooks)
    {
        if (m_filter.Accept(b))
        {
            m_filteredBooks.push_back(b);
        }
    }
    m_filter.Sort(m_filteredBooks);

    int total = (int)m_filteredBooks.size();
    m_totalPages = (total <= 0) ? 1 : (total + m_pageSize - 1) / m_pageSize;
    if (m_currentPage < 0) m_currentPage = 0;
    if (m_currentPage >= m_totalPages) m_currentPage = m_totalPages - 1;

    const int start = m_currentPage * m_pageSize;
    const int end = min(start + m_pageSize, total);
    for (int i = start; i < end; ++i)
    {
        m_pageBooks.push_back(m_filteredBooks[i]);
    }

    LoadTextures();
    for (int i = 0; i < (int)m_pageBooks.size(); ++i) {
        BookData b;
        b.id = m_pageBooks[i].id;
        b.texIndex = i;
        b.title = m_pageBooks[i].title;
        b.description = m_pageBooks[i].description;
        b.coverPath = m_pageBooks[i].coverPath;
        b.year.Format(_T("%d"), m_pageBooks[i].year);
        COLORREF avg = (i < (int)m_coverAvgColors.size()) ? m_coverAvgColors[i] : RGB(110, 128, 150);
        b.r = (float)GetRValue(avg) / 255.0f;
        b.g = (float)GetGValue(avg) / 255.0f;
        b.b = (float)GetBValue(avg) / 255.0f;
        b.curX = b.curY = -10.0f;
        b.curZ = -10.0f;
        b.curRotX = b.curRotY = b.curRotZ = 0.0f;
        b.targetX = b.targetY = b.targetZ = 0.0f;
        b.targetRotX = b.targetRotY = b.targetRotZ = 0.0f;
        m_books.push_back(b);
    }
    m_hoverIndex = -1;
    m_selectedIndex = -1;
    m_bInspectMode = false;
    m_scrollOffset = 0.0f;
    m_tableHoverRow = -1;
}

void CLibraryAppView::InitBooks() {
    ReloadBooksPage();
}

void CLibraryAppView::OnTimer(UINT_PTR nIDEvent) {
    bool needRedraw = false;
    if (m_screen == SCREEN_BOOK_LIST) {
        UpdateAnimations();
        needRedraw = true;
    }
    else if (m_screen == SCREEN_ANALYTICS && m_chartDragging) {
        needRedraw = true;
    }
    if (needRedraw) Invalidate(FALSE);
    CView::OnTimer(nIDEvent);
}

void CLibraryAppView::BeginOverlay2D() {
    CRect rc;
    GetClientRect(&rc);
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, rc.Width(), rc.Height(), 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

void CLibraryAppView::EndOverlay2D() {
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
}

void CLibraryAppView::DrawTextGL(int x, int y, const CString& text, float r, float g, float b)
{
    if (text.IsEmpty()) return;

    const float scale = 16.0f;
    const int fontHeight = -16;
    int lineHeight = 20;
    int baselineOffset = 15;

    CClientDC dc(this);
    CFont font;
    if (font.CreateFont(
        fontHeight, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        RUSSIAN_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY, FF_SWISS, _T("Arial"))) {
        CFont* pOldFont = dc.SelectObject(&font);
        TEXTMETRIC tm;
        if (dc.GetTextMetrics(&tm)) {
            lineHeight = tm.tmHeight;
            baselineOffset = tm.tmAscent;
        }
        dc.SelectObject(pOldFont);
    }

    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LINE_BIT | GL_POLYGON_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor3f(r, g, b);

    int lineStart = 0;
    int lineIndex = 0;

    for (int i = 0; i <= text.GetLength(); ++i) {
        if (i == text.GetLength() || text[i] == _T('\n')) {
            CString line = text.Mid(lineStart, i - lineStart);

            glPushMatrix();

            glTranslatef((float)x, (float)(y + baselineOffset + lineIndex * lineHeight), 0.0f);
            glScalef(scale, -scale, 1.0f);
            glListBase(m_listBase);

            int len = line.GetLength();
            if (len > 0) {
                char* buf = new char[len + 1];
                WideCharToMultiByte(1251, 0, line, -1, buf, len + 1, NULL, NULL);

                glCallLists(strlen(buf), GL_UNSIGNED_BYTE, buf);

                delete[] buf;
            }

            glPopMatrix();

            lineStart = i + 1;
            lineIndex++;
        }
    }
    glPopAttrib();
}

void CLibraryAppView::DrawText3D(double x, double y, double z, const CString& text, float r, float g, float b, double scale)
{
    if (text.IsEmpty()) return;

    int len = text.GetLength();
    char* ansiBuf = new char[len + 1];
    WideCharToMultiByte(1251, 0, text, -1, ansiBuf, len + 1, NULL, NULL);

    double s = m_glTextScale3D * scale;
    glPushMatrix();
    glTranslated(x, y, z);
    glScaled(s, s, s);
    glColor3f(r, g, b);
    glListBase(m_listBase);
    glCallLists(strlen(ansiBuf), GL_UNSIGNED_BYTE, ansiBuf);

    glPopMatrix();

    delete[] ansiBuf;
}

float CLibraryAppView::Measure3DTextWidth(const CString& text, double scale)
{
    if (text.IsEmpty()) return 0.0f;
    int len = text.GetLength();
    char* buf = new char[len + 1];
    WideCharToMultiByte(1251, 0, text, -1, buf, len + 1, NULL, NULL);
    float w = 0.0f;
    for (int i = 0; buf[i]; ++i)
        w += m_glyphMetrics[(unsigned char)buf[i]].gmfCellIncX;
    delete[] buf;
    return w * (float)(m_glTextScale3D * scale);
}

CSize CLibraryAppView::MeasureTextGL(const CString& text) const
{
    if (text.IsEmpty()) return CSize(0, 0);

    const int fontHeight = -16;

    CClientDC dc(const_cast<CLibraryAppView*>(this));

    CFont font;
    font.CreateFont(
        fontHeight, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        RUSSIAN_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY, FF_SWISS, _T("Arial"));

    CFont* pOldFont = dc.SelectObject(&font);

    TEXTMETRIC tm;
    int lineHeight = 20;
    if (dc.GetTextMetrics(&tm)) lineHeight = tm.tmHeight;

    int maxWidth = 0;
    int lines = 0;
    int lineStart = 0;
    for (int i = 0; i <= text.GetLength(); ++i) {
        if (i == text.GetLength() || text[i] == _T('\n')) {
            CString line = text.Mid(lineStart, i - lineStart);
            CSize sz = dc.GetTextExtent(line);
            if (sz.cx > maxWidth) maxWidth = sz.cx;
            lineStart = i + 1;
            lines++;
        }
    }

    dc.SelectObject(pOldFont);

    return CSize(maxWidth, max(1, lines) * lineHeight);
}

int CLibraryAppView::HitTestTableRow(CPoint point) const {
    if (!m_tableRect.PtInRect(point)) return -1;
    if (point.y < m_tableRect.top + m_tableHeaderH) return -1;
    int row = (point.y - (m_tableRect.top + m_tableHeaderH)) / m_tableRowH;
    if (row < 0 || row >= (int)m_pageBooks.size()) return -1;
    return row;
}

void CLibraryAppView::DrawOverlayCentered(CDC* pDC,
    const CString& text, int yOffset) {
    (void)pDC;
    CRect r;
    GetClientRect(&r);

    glColor4f(0.0f, 0.0f, 0.0f, 0.3f);
    glBegin(GL_QUADS);
    glVertex2i(0, 0);
    glVertex2i(r.Width(), 0);
    glVertex2i(r.Width(), r.Height());
    glVertex2i(0, r.Height());
    glEnd();

    CString drawText = text;
    const int maxTextW = max(80, r.Width() - 80);
    while (!drawText.IsEmpty() && MeasureTextGL(drawText).cx > maxTextW) {
        drawText = drawText.Left(drawText.GetLength() - 1);
    }
    if (drawText != text && drawText.GetLength() > 3) drawText = drawText.Left(drawText.GetLength() - 3) + _T("...");

    CSize sz = MeasureTextGL(drawText);
    int x = (r.Width() - sz.cx) / 2;
    int y = (r.Height() - sz.cy) / 2 + yOffset;

    int pad = 12;
    x = max(pad, min(x, r.Width() - pad - sz.cx));
    y = max(pad, min(y, r.Height() - pad - sz.cy));

    glColor4f(1.0f, 1.0f, 1.0f, 0.95f);
    glBegin(GL_QUADS);
    glVertex2i(x - pad, y - pad - 6);
    glVertex2i(x + sz.cx + pad, y - pad - 6);
    glVertex2i(x + sz.cx + pad, y + sz.cy + pad - 2);
    glVertex2i(x - pad, y + sz.cy + pad - 2);
    glEnd();

    DrawTextGL(x, y, drawText, 0.0f, 0.47f, 0.84f);
}

void CLibraryAppView::DrawProfileOverlay(CDC* pDC) {
    (void)pDC;

    CRect rc;
    GetClientRect(&rc);
    int cardW = min(560, rc.Width() - 40);
    int cardH = min(320, rc.Height() - 40);
    int cardX = (rc.Width() - cardW) / 2;
    int cardY = (rc.Height() - cardH) / 2;
    CRect card(cardX, cardY, cardX + cardW, cardY + cardH);

    glColor4f(0.0f, 0.0f, 0.0f, 0.08f);
    glBegin(GL_QUADS);
    glVertex2i(card.left + 4, card.top + 4);
    glVertex2i(card.right + 4, card.top + 4);
    glVertex2i(card.right + 4, card.bottom + 4);
    glVertex2i(card.left + 4, card.bottom + 4);
    glEnd();

    glColor4f(1.0f, 1.0f, 1.0f, 0.98f);
    glBegin(GL_QUADS);
    glVertex2i(card.left, card.top);
    glVertex2i(card.right, card.top);
    glVertex2i(card.right, card.bottom);
    glVertex2i(card.left, card.bottom);
    glEnd();

    glColor4f(0.84f, 0.86f, 0.90f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2i(card.left, card.top);
    glVertex2i(card.right, card.top);
    glVertex2i(card.right, card.bottom);
    glVertex2i(card.left, card.bottom);
    glEnd();

    DrawTextGL(card.left + 16, card.top + 24, _T("Профіль"), 0.0f, 0.40f, 0.75f);

    CRect grpInfo(card.left + 16, card.top + 52, card.left + cardW / 2 - 8, card.bottom - 16);
    CRect grpStat(card.left + cardW / 2 + 8, card.top + 52, card.right - 16, card.bottom - 16);

    glColor4f(0.97f, 0.98f, 0.99f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2i(grpInfo.left, grpInfo.top);
    glVertex2i(grpInfo.right, grpInfo.top);
    glVertex2i(grpInfo.right, grpInfo.bottom);
    glVertex2i(grpInfo.left, grpInfo.bottom);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2i(grpStat.left, grpStat.top);
    glVertex2i(grpStat.right, grpStat.top);
    glVertex2i(grpStat.right, grpStat.bottom);
    glVertex2i(grpStat.left, grpStat.bottom);
    glEnd();

    glColor4f(0.83f, 0.86f, 0.90f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2i(grpInfo.left, grpInfo.top);
    glVertex2i(grpInfo.right, grpInfo.top);
    glVertex2i(grpInfo.right, grpInfo.bottom);
    glVertex2i(grpInfo.left, grpInfo.bottom);
    glEnd();
    glBegin(GL_LINE_LOOP);
    glVertex2i(grpStat.left, grpStat.top);
    glVertex2i(grpStat.right, grpStat.top);
    glVertex2i(grpStat.right, grpStat.bottom);
    glVertex2i(grpStat.left, grpStat.bottom);
    glEnd();

    DrawTextGL(grpInfo.left + 10, grpInfo.top + 20, _T("Дані"), 0.0f, 0.40f, 0.75f);
    DrawTextGL(grpStat.left + 10, grpStat.top + 20, _T("Статистика"), 0.0f, 0.40f, 0.75f);

    CString roleUa = m_db.IsAdmin() ? _T("Адмін") : _T("Читач");
    CString info;
    info.Format(
        _T("Логін: %s\n\nПІБ: %s\n\nЕ-пошта: %s\n\nТел: %s"),
        m_db.GetCurrentLogin().GetString(),
        m_db.GetCurrentFullName().GetString(),
        m_db.GetCurrentEmail().GetString(),
        m_db.GetCurrentPhone().GetString());
    DrawTextGL(grpInfo.left + 10, grpInfo.top + 46, info, 0.15f, 0.18f, 0.22f);

    int totalBooks = m_db.GetBooksCount();
    int totalCategories = (int)m_db.GetCategoryStats().size();
    int activeReservations = 0;
    std::vector < CString > rows = m_db.GetUsersReservationsRows();
    if (!rows.empty()) {
        bool onlyInfo = (rows.size() == 1) &&
            (rows[0].Find(_T("немає")) >= 0 || rows[0].Find(_T("Немає")) >= 0 || rows[0].Find(_T("Помилка")) >= 0);
        activeReservations = onlyInfo ? 0 : (int)rows.size();
    }

    CString stats;
    stats.Format(
        _T("Роль: %s\n\nКниг: %d\n\nКатегорій: %d\n\nБроней: %d"),
        roleUa.GetString(),
        totalBooks,
        totalCategories,
        activeReservations);
    DrawTextGL(grpStat.left + 10, grpStat.top + 46, stats, 0.15f, 0.18f, 0.22f);
}

void CLibraryAppView::DrawAnalyticsOverlay(CDC* pDC) {
    (void)pDC;
    CString dataset;
    switch (m_chartDataIndex) {
    case 0: dataset = _T("Книги за категоріями"); break;
    case 1: dataset = _T("Доступність книг"); break;
    case 2: dataset = _T("Активність"); break;
    case 3: dataset = _T("Топ за рейтингом"); break;
    case 4: dataset = _T("Популярні книги"); break;
    case 5: dataset = _T("Стани бронювань"); break;
    case 6: dataset = _T("Оцінки відгуків"); break;
    case 7: dataset = _T("Книги за десятиліттями"); break;
    case 8: dataset = _T("Бронювання по філіях"); break;
    case 9: dataset = _T("Доступні примірники"); break;
    case 10: dataset = _T("Користувачі за ролями"); break;
    case 11: dataset = _T("Топ авторів"); break;
    case 12: dataset = _T("Бронювання по місяцях"); break;
    case 13: dataset = _T("Відгуки по місяцях"); break;
    case 14: dataset = _T("Книги за рейтингом"); break;
    default: dataset = _T("Загальна статистика"); break;
    }

    CString mode;
    mode.Format(_T("%s | %s | %s  [%d/16]"), dataset.GetString(),
        m_chartPieMode ? _T("Кругова") : _T("Гістограма"),
        m_chart3DMode ? _T("3D") : _T("2D"),
        m_chartDataIndex + 1);
    DrawTextGL(14, 18, mode, 0.0f, 0.40f, 0.75f);
    DrawTextGL(14, 38, _T("WASD/\u043c\u0438\u0448\u0430: \u043e\u0431\u0435\u0440\u0442\u0430\u043d\u043d\u044f  |  \u041a\u043e\u043b\u0435\u0441\u043e: \u043c\u0430\u0441\u0448\u0442\u0430\u0431  |  \u0421\u0442\u0440\u0456\u043b\u043a\u0438: \u043d\u0430\u0431\u0456\u0440 \u0434\u0430\u043d\u0438\u0445"), 0.45f, 0.50f, 0.55f);
}

void CLibraryAppView::OnDraw(CDC* pDC) {
    CRect rc;
    GetClientRect(&rc);

    if (m_screen == SCREEN_ANALYTICS || m_screen == SCREEN_3D_INSPECT || (m_screen == SCREEN_BOOK_LIST && m_bInspectMode)) {
        wglMakeCurrent(m_pDC->GetSafeHdc(), m_hRC);
        glViewport(0, 0, rc.Width(), rc.Height());

        if (m_bInspectMode && m_selectedIndex >= 0 && m_selectedIndex < (int)m_books.size()) {
            float br = m_books[m_selectedIndex].r, bg = m_books[m_selectedIndex].g, bb = m_books[m_selectedIndex].b;
            glClearColor(0.08f + br * 0.1f, 0.09f + bg * 0.1f, 0.12f + bb * 0.12f, 1.0f);
        }
        else if (m_screen == SCREEN_3D_INSPECT) {
            glClearColor(0.15f, 0.16f, 0.20f, 1.0f);
        }
        else {
            glClearColor(0.96f, 0.96f, 0.97f, 1.0f);
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        GLfloat l0[] = { 10.0f, 15.0f, 10.0f, 1.0f };
        GLfloat l_dif[] = { 1.0f, 0.95f, 0.9f, 1.0f };
        glLightfv(GL_LIGHT0, GL_POSITION, l0);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, l_dif);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        if (m_screen == SCREEN_ANALYTICS) DrawCharts();
        else if (m_screen == SCREEN_3D_INSPECT) {
            Draw3DInspectScene(false);
            BeginOverlay2D();
            DrawTextGL(rc.Width() / 2 - 80, rc.Height() - 18, _T("ESC \u2192 \u043d\u0430\u0437\u0430\u0434"), 0.55f, 0.55f, 0.55f);
            EndOverlay2D();
        }
        else RenderScene(false);

        SwapBuffers(m_pDC->GetSafeHdc());
        return;
    }

    wglMakeCurrent(NULL, NULL);

    CDC memDC;
    memDC.CreateCompatibleDC(pDC);
    CBitmap bmp;
    bmp.CreateCompatibleBitmap(pDC, rc.Width(), rc.Height());
    CBitmap* pOldBmp = memDC.SelectObject(&bmp);

    memDC.FillSolidRect(&rc, RGB(245, 245, 247));
    memDC.SetBkMode(TRANSPARENT);

    if (m_screen == SCREEN_AUTH_REQUIRED) {
        CRect r1 = rc; r1.bottom = rc.Height() / 2;
        memDC.SetTextColor(RGB(38, 46, 56));
        CFont* old = memDC.SelectObject(&m_fontTitle);
        memDC.DrawText(_T("Вхід не виконано"), &r1, DT_CENTER | DT_BOTTOM | DT_SINGLELINE);
        CRect r2 = rc; r2.top = rc.Height() / 2 + 4;
        memDC.SelectObject(&m_fontNormal);
        memDC.SetTextColor(RGB(130, 130, 130));
        memDC.DrawText(_T("Меню \u2192 Бібліотека \u2192 Вхід / Реєстрація"), &r2, DT_CENTER | DT_TOP | DT_SINGLELINE);
        memDC.SelectObject(old);
    }
    else if (m_screen == SCREEN_PROFILE) {
        auto reservations = m_db.GetCurrentUserReservations();
        auto reviews = m_db.GetCurrentUserReviews();

        std::set<CString> readBooksSet;
        for (const auto& rec : reservations)
        {
            CString status = rec.status;
            status.MakeLower();
            if (status == _T("returned") && !rec.bookTitle.IsEmpty())
                readBooksSet.insert(rec.bookTitle);
        }
        std::vector<CString> readBooks(readBooksSet.begin(), readBooksSet.end());

        auto StatusLabel = [](const CString& raw) {
            CString s = raw; s.MakeLower();
            if (s == _T("reserved")) return CString(_T("Зарезервовано"));
            if (s == _T("ready_for_pickup")) return CString(_T("Готово до видачі"));
            if (s == _T("issued")) return CString(_T("Видано"));
            if (s == _T("overdue")) return CString(_T("Прострочено"));
            if (s == _T("return_requested")) return CString(_T("Запит на повернення"));
            if (s == _T("returned")) return CString(_T("Повернуто"));
            if (s == _T("lost")) return CString(_T("Втрачено"));
            if (s == _T("damaged")) return CString(_T("Пошкоджено"));
            if (s == _T("cancelled")) return CString(_T("Скасовано"));
            if (s == _T("expired")) return CString(_T("Термін минув"));
            return raw;
        };

        CRect panel(12, 12, rc.right - 12, rc.bottom - 12);
        memDC.FillSolidRect(&panel, RGB(255, 255, 255));
        memDC.Draw3dRect(&panel, RGB(213, 220, 230), RGB(213, 220, 230));

        CFont* old = memDC.SelectObject(&m_fontBold);
        memDC.SetTextColor(RGB(0, 100, 190));
        CRect titleRect(panel.left + 12, panel.top + 10, panel.right - 12, panel.top + 34);
        memDC.DrawText(_T("Профіль користувача"), &titleRect, DT_LEFT | DT_TOP | DT_SINGLELINE);

        memDC.SelectObject(&m_fontNormal);
        memDC.SetTextColor(RGB(85, 96, 108));
        CString role = m_db.IsAdmin() ? _T("Адмін") : _T("Читач");
        CString headInfo;
        headInfo.Format(
            _T("%s | %s | %s | Роль: %s"),
            m_db.GetCurrentLogin().GetString(),
            m_db.GetCurrentFullName().GetString(),
            m_db.GetCurrentEmail().GetString(),
            role.GetString());
        CRect headRect(panel.left + 12, panel.top + 34, panel.right - 12, panel.top + 56);
        memDC.DrawText(headInfo, &headRect, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS);

        CRect scrollRect(panel.left + 10, panel.top + 58, panel.right - 10, panel.bottom - 10);
        memDC.FillSolidRect(&scrollRect, RGB(248, 251, 255));
        memDC.Draw3dRect(&scrollRect, RGB(226, 232, 240), RGB(226, 232, 240));

        int contentH = 0;
        contentH += 34 + max(26, (int)reservations.size() * 52);
        contentH += 30 + max(26, (int)readBooks.size() * 26);
        contentH += 30 + max(26, (int)reviews.size() * 54);
        contentH += 20;

        int minScroll = min(0, scrollRect.Height() - contentH);
        if (m_profileScrollY > 0.0f) m_profileScrollY = 0.0f;
        if (m_profileScrollY < (float)minScroll) m_profileScrollY = (float)minScroll;

        int saved = memDC.SaveDC();
        memDC.IntersectClipRect(&scrollRect);

        int y = scrollRect.top + 10 + (int)m_profileScrollY;

        memDC.SetTextColor(RGB(0, 100, 190));
        memDC.SelectObject(&m_fontBold);
        memDC.DrawText(_T("Бронювання"), &CRect(scrollRect.left + 10, y, scrollRect.right - 10, y + 22), DT_LEFT | DT_TOP | DT_SINGLELINE);
        y += 24;

        memDC.SelectObject(&m_fontNormal);
        if (reservations.empty())
        {
            memDC.SetTextColor(RGB(120, 126, 134));
            memDC.DrawText(_T("Немає активних або історичних бронювань."), &CRect(scrollRect.left + 12, y, scrollRect.right - 12, y + 24), DT_LEFT | DT_TOP);
            y += 30;
        }
        else
        {
            for (int i = 0; i < (int)reservations.size(); ++i)
            {
                CRect item(scrollRect.left + 8, y, scrollRect.right - 8, y + 46);
                memDC.FillSolidRect(&item, (i % 2 == 0) ? RGB(255, 255, 255) : RGB(252, 253, 255));
                memDC.Draw3dRect(&item, RGB(230, 235, 242), RGB(230, 235, 242));

                CString row1;
                row1.Format(_T("%s  |  %s"), reservations[i].bookTitle.GetString(), StatusLabel(reservations[i].status).GetString());
                memDC.SetTextColor(RGB(50, 58, 68));
                memDC.DrawText(row1, &CRect(item.left + 8, item.top + 6, item.right - 8, item.top + 22), DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS);

                CString row2;
                row2.Format(_T("%s | %s → %s"), reservations[i].branchName.GetString(), reservations[i].reservationDate.GetString(), reservations[i].returnDate.GetString());
                memDC.SetTextColor(RGB(102, 112, 124));
                memDC.DrawText(row2, &CRect(item.left + 8, item.top + 24, item.right - 8, item.bottom - 4), DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS);
                y += 52;
            }
        }

        y += 4;
        memDC.SetTextColor(RGB(0, 100, 190));
        memDC.SelectObject(&m_fontBold);
        memDC.DrawText(_T("Прочитані книги (успішно повернуті)"), &CRect(scrollRect.left + 10, y, scrollRect.right - 10, y + 22), DT_LEFT | DT_TOP | DT_SINGLELINE);
        y += 24;
        memDC.SelectObject(&m_fontNormal);
        if (readBooks.empty())
        {
            memDC.SetTextColor(RGB(120, 126, 134));
            memDC.DrawText(_T("Поки що немає повернутих книг."), &CRect(scrollRect.left + 12, y, scrollRect.right - 12, y + 22), DT_LEFT | DT_TOP | DT_SINGLELINE);
            y += 28;
        }
        else
        {
            for (int i = 0; i < (int)readBooks.size(); ++i)
            {
                CString row;
                row.Format(_T("• %s"), readBooks[i].GetString());
                memDC.SetTextColor(RGB(64, 72, 82));
                memDC.DrawText(row, &CRect(scrollRect.left + 12, y, scrollRect.right - 12, y + 22), DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS);
                y += 24;
            }
        }

        y += 2;
        memDC.SetTextColor(RGB(0, 100, 190));
        memDC.SelectObject(&m_fontBold);
        memDC.DrawText(_T("Мої відгуки"), &CRect(scrollRect.left + 10, y, scrollRect.right - 10, y + 22), DT_LEFT | DT_TOP | DT_SINGLELINE);
        y += 24;
        memDC.SelectObject(&m_fontNormal);
        if (reviews.empty())
        {
            memDC.SetTextColor(RGB(120, 126, 134));
            memDC.DrawText(_T("Відгуки ще не залишалися."), &CRect(scrollRect.left + 12, y, scrollRect.right - 12, y + 22), DT_LEFT | DT_TOP | DT_SINGLELINE);
            y += 28;
        }
        else
        {
            for (int i = 0; i < (int)reviews.size(); ++i)
            {
                CRect item(scrollRect.left + 8, y, scrollRect.right - 8, y + 48);
                memDC.FillSolidRect(&item, RGB(255, 255, 255));
                memDC.Draw3dRect(&item, RGB(230, 235, 242), RGB(230, 235, 242));

                CString h;
                h.Format(_T("★ %d  %s  (%s)"), reviews[i].rating, reviews[i].userName.GetString(), reviews[i].createdAt.GetString());
                memDC.SetTextColor(RGB(58, 66, 76));
                memDC.DrawText(h, &CRect(item.left + 8, item.top + 5, item.right - 8, item.top + 21), DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS);

                CString c = reviews[i].comment;
                if (c.GetLength() > 150) c = c.Left(147) + _T("...");
                memDC.SetTextColor(RGB(95, 104, 114));
                memDC.DrawText(c, &CRect(item.left + 8, item.top + 22, item.right - 8, item.bottom - 4), DT_LEFT | DT_TOP | DT_WORDBREAK | DT_END_ELLIPSIS);
                y += 54;
            }
        }

        memDC.RestoreDC(saved);

        memDC.SelectObject(old);
    }
    else if (m_screen == SCREEN_BOOK_DETAILS) {
        DrawBookDetailsScreen(&memDC);
    }
    else if (m_screen == SCREEN_BOOK_LIST) {
        CRect rc;
        GetClientRect(&rc);

        CRect rTitle = rc;
        rTitle.top = 4;
        rTitle.bottom = 24;
        rTitle.left += 10;
        memDC.SetTextColor(RGB(0, 100, 190));
        CFont* old = memDC.SelectObject(&m_fontBold);
        CString title;
        title.Format(_T("Каталог  |  %d книг"), (int)m_filteredBooks.size());
        memDC.DrawText(title, &rTitle, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        memDC.SelectObject(&m_fontNormal);

        CString filterInfo;
        filterInfo.Format(
            _T("Категорія: %s  |  Сортування: %s (%s)"),
            m_filter.GetCategoryLabel().GetString(),
            m_filter.GetSortLabel().GetString(),
            m_filter.IsSortAscending() ? _T("за зростанням") : _T("за спаданням"));
        CRect rFilter = rc;
        rFilter.top = 26;
        rFilter.bottom = 44;
        rFilter.left += 10;
        memDC.SetTextColor(RGB(90, 100, 110));
        memDC.DrawText(filterInfo, &rFilter, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

        int tableW = rc.Width() - 20;
        int tableX = 10;
        int tableY = 48;
        int tableH = rc.Height() - tableY - 48;
        m_tableRect = CRect(tableX, tableY, tableX + tableW, tableY + tableH);

        memDC.FillSolidRect(&m_tableRect, RGB(255, 255, 255));
        CPen borderPen(PS_SOLID, 2, RGB(180, 190, 200));
        CPen* oldPen = memDC.SelectObject(&borderPen);
        memDC.MoveTo(m_tableRect.left, m_tableRect.top);
        memDC.LineTo(m_tableRect.right, m_tableRect.top);
        memDC.LineTo(m_tableRect.right, m_tableRect.bottom);
        memDC.LineTo(m_tableRect.left, m_tableRect.bottom);
        memDC.LineTo(m_tableRect.left, m_tableRect.top);
        memDC.SelectObject(oldPen);

        CRect rH = m_tableRect;
        rH.bottom = rH.top + m_tableHeaderH;
        
        COLORREF headerColor = RGB(230, 240, 250);
        memDC.FillSolidRect(&rH, headerColor);
        
        CPen headerBorderPen(PS_SOLID, 1, RGB(200, 210, 220));
        oldPen = memDC.SelectObject(&headerBorderPen);
        memDC.MoveTo(rH.left, rH.bottom);
        memDC.LineTo(rH.right, rH.bottom);
        memDC.SelectObject(oldPen);

        memDC.SelectObject(&m_fontBold);
        memDC.SetTextColor(RGB(0, 120, 214));

        int x = rH.left;
        float cw[] = { 0.38f, 0.20f, 0.08f, 0.18f, 0.16f };
        CString cols[] = { _T("Назва"), _T("Автор"), _T("Рік"), _T("Категорія"), _T("Рейтинг") };

        int sortCol = 0;
        switch (m_filter.GetSortMode())
        {
        case Filter::SORT_TITLE: sortCol = 0; break;
        case Filter::SORT_AUTHOR: sortCol = 1; break;
        case Filter::SORT_YEAR: sortCol = 2; break;
        case Filter::SORT_CATEGORY: sortCol = 3; break;
        case Filter::SORT_RATING: sortCol = 4; break;
        default: sortCol = 0; break;
        }
        
        for (int i = 0; i < 5; i++) {
            int w = (int)(m_tableRect.Width() * cw[i]);
            CRect rcCell(x + 8, rH.top + 4, x + w, rH.bottom - 4);
            CString headerText = cols[i];
            if (i == sortCol)
            {
                headerText += m_filter.IsSortAscending() ? _T(" ▲") : _T(" ▼");
            }
            memDC.DrawText(headerText, &rcCell, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
            x += w;
        }

        memDC.SelectObject(&m_fontNormal);
        memDC.SetTextColor(RGB(60, 60, 60));
        
        CPen rowPen(PS_SOLID, 1, RGB(240, 240, 245));
        oldPen = memDC.SelectObject(&rowPen);

        int y = rH.bottom;
        for (int i = 0; i < (int)m_pageBooks.size(); i++) {
            if (y + m_tableRowH > m_tableRect.bottom) break;
            CRect rRow(m_tableRect.left, y, m_tableRect.right, y + m_tableRowH);

            if (i == m_selectedIndex) {
                memDC.FillSolidRect(&rRow, RGB(188, 224, 255));
            } else if (i == m_tableHoverRow) {
                memDC.FillSolidRect(&rRow, RGB(220, 240, 255));
            } else if (i % 2 == 0) {
                memDC.FillSolidRect(&rRow, RGB(250, 250, 252));
            }

            x = rRow.left;
            CString vals[] = { m_pageBooks[i].title, m_pageBooks[i].author, _T(""), m_pageBooks[i].category, _T("") };
            vals[2].Format(_T("%d"), m_pageBooks[i].year);
            vals[4].Format(_T("★ %.1f"), m_pageBooks[i].rating);

            memDC.SetTextColor(RGB(60, 60, 60));
            for (int c = 0; c < 5; c++) {
                int w = (int)(m_tableRect.Width() * cw[c]);
                CRect rcCell(x + 8, y + 3, x + w - 4, y + m_tableRowH - 3);
                
                if (c == 4) {
                    memDC.SetTextColor(RGB(230, 130, 0));
                }

                memDC.DrawText(vals[c], &rcCell, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

                if (c == 4) {
                    memDC.SetTextColor(RGB(60, 60, 60));
                }

                x += w;
            }

            memDC.MoveTo(rRow.left, rRow.bottom);
            memDC.LineTo(rRow.right, rRow.bottom);
            y += m_tableRowH;
        }
        memDC.SelectObject(oldPen);

        CRect rFooter(m_tableRect.left, m_tableRect.bottom + 4, m_tableRect.right, m_tableRect.bottom + 22);
        CString pageInfo;
        pageInfo.Format(_T("Стор. %d / %d"), m_currentPage + 1, m_totalPages);
        memDC.SetTextColor(RGB(100, 100, 100));
        memDC.DrawText(pageInfo, &rFooter, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        memDC.SetTextColor(RGB(0, 120, 214));
        memDC.SelectObject(&m_fontBold);
        CString hint = _T("ЛКМ: обрати книгу | ПКМ: дії з книгою | ↑/↓: категорія | ←/→: сторінка | Клік по заголовку: сортування");
        CRect rHint(m_tableRect.left, m_tableRect.bottom + 26, m_tableRect.right, m_tableRect.bottom + 44);
        memDC.SetTextColor(RGB(160, 160, 160));
        memDC.SelectObject(&m_fontNormal);
        memDC.DrawText(hint, &rHint, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        memDC.SelectObject(old);
    }

    pDC->BitBlt(0, 0, rc.Width(), rc.Height(), &memDC, 0, 0, SRCCOPY);
    memDC.SelectObject(pOldBmp);
}

void CLibraryAppView::RenderAnalyticsScene() {
    DrawCharts();
}

void CLibraryAppView::DrawCharts() {
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glClearColor(0.94f, 0.95f, 0.97f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    CRect rc;
    GetClientRect(&rc);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (m_chart3DMode) {
        gluPerspective(45.0f, rc.Height() ? (GLfloat)rc.Width() / (GLfloat)rc.Height() : 1.0f, 0.1f, 200.0f);
    } else {
        float aspect = rc.Height() ? (GLfloat)rc.Width() / (GLfloat)rc.Height() : 1.0f;
        float orthoH = -m_chartZoom * 0.5f;
        orthoH = max(4.0f, min(18.0f, orthoH));
        glOrtho(-orthoH * aspect, orthoH * aspect, -orthoH, orthoH, -50.0, 50.0);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    if (m_chart3DMode) {
        float cameraDist = -m_chartZoom;
        cameraDist = max(5.0f, min(35.0f, cameraDist));
        gluLookAt(0.0, 1.5, cameraDist, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    } else {
        gluLookAt(0.0, 0.0, 10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    }

    GLfloat lightPos0[] = { 10.0f, 16.0f, 14.0f, 1.0f };
    GLfloat lightPos1[] = { -8.0f, 8.0f, -6.0f, 1.0f };
    GLfloat lightDif0[] = { 1.0f, 0.98f, 0.95f, 1.0f };
    GLfloat lightDif1[] = { 0.30f, 0.35f, 0.45f, 1.0f };
    GLfloat lightAmb[] = { 0.18f, 0.18f, 0.22f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDif0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
    glLightfv(GL_LIGHT1, GL_POSITION, lightPos1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDif1);

    std::vector<CategoryStat> stats;
    CString datasetTitle = _T("Книги за категоріями");
    const int NUM_DATASETS = 16;

    if (m_chartDataIndex == 0) {
        stats = m_db.GetCategoryStats();
    }
    else if (m_chartDataIndex == 1) {
        datasetTitle = _T("Стан доступності книг");
        int available = 0, issued = 0;
        m_db.GetAvailabilityStats(available, issued);
        CategoryStat s1; s1.name = _T("Доступні"); s1.count = available; stats.push_back(s1);
        CategoryStat s2; s2.name = _T("Видані");    s2.count = issued;    stats.push_back(s2);
    }
    else if (m_chartDataIndex == 2) {
        datasetTitle = _T("Активність користувачів");
        stats = m_db.GetUserActivityStats();
    }
    else if (m_chartDataIndex == 3) {
        datasetTitle = _T("Топ книг за рейтингом");
        auto books = m_db.GetTopRatedBooks(8);
        for (int bi = 0; bi < (int)books.size(); ++bi) {
            CategoryStat s;
            s.name = books[bi].title;
            s.count = (int)(books[bi].rating * 10 + 0.5f);
            stats.push_back(s);
        }
    }
    else if (m_chartDataIndex == 4) {
        datasetTitle = _T("Найпопулярніші книги");
        auto books = m_db.GetMostReservedBooks(8);
        for (int bi = 0; bi < (int)books.size(); ++bi) {
            CategoryStat s;
            s.name = books[bi].title;
            s.count = books[bi].quantityTotal;
            stats.push_back(s);
        }
    }
    else if (m_chartDataIndex == 5) {
        datasetTitle = _T("Стани бронювань");
        stats = m_db.GetReservationStatusStats();
    }
    else if (m_chartDataIndex == 6) {
        datasetTitle = _T("Оцінки відгуків");
        stats = m_db.GetRatingDistributionStats();
        for (int i = 0; i < (int)stats.size(); ++i) {
            CString label;
            label.Format(_T("★%s"), stats[i].name.GetString());
            stats[i].name = label;
        }
    }
    else if (m_chartDataIndex == 7) {
        datasetTitle = _T("Книги за десятиліттями");
        stats = m_db.GetBooksByDecadeStats();
    }
    else if (m_chartDataIndex == 8) {
        datasetTitle = _T("Бронювання по філіях");
        stats = m_db.GetBranchReservationStats(10);
    }
    else if (m_chartDataIndex == 9) {
        datasetTitle = _T("Доступні примірники");
        stats = m_db.GetCategoryAvailabilityStats();
    }
    else if (m_chartDataIndex == 10) {
        datasetTitle = _T("Користувачі за ролями");
        stats = m_db.GetUsersByRoleStats();
    }
    else if (m_chartDataIndex == 11) {
        datasetTitle = _T("Топ авторів");
        stats = m_db.GetBooksByAuthorStats(10);
    }
    else if (m_chartDataIndex == 12) {
        datasetTitle = _T("Бронювання по місяцях");
        stats = m_db.GetReservationsByMonthStats(12);
    }
    else if (m_chartDataIndex == 13) {
        datasetTitle = _T("Відгуки по місяцях");
        stats = m_db.GetReviewsByMonthStats(12);
    }
    else if (m_chartDataIndex == 14) {
        datasetTitle = _T("Книги за рейтингом");
        stats = m_db.GetBooksByRatingBucketStats();
    }
    else {
        datasetTitle = _T("Загальна статистика");
        AnalyticsData ad = m_db.GetAnalytics();
        CategoryStat s;
        s.name = _T("Книги");        s.count = ad.totalBooks;         stats.push_back(s);
        s.name = _T("Категорії");    s.count = ad.totalCategories;    stats.push_back(s);
        s.name = _T("Користувачі"); s.count = ad.totalUsers;         stats.push_back(s);
        s.name = _T("Філії");        s.count = ad.totalBranches;      stats.push_back(s);
        s.name = _T("Бронювання");  s.count = ad.activeReservations; stats.push_back(s);
        s.name = _T("Повернення");  s.count = ad.returnedBooks;      stats.push_back(s);
        s.name = _T("Відгуки");      s.count = ad.totalReviews;       stats.push_back(s);
    }

    if (stats.empty()) {
        CategoryStat s; s.name = _T("Немає даних"); s.count = 1;
        stats.push_back(s);
    }

    const float palette[][3] = {
        {0.22f, 0.56f, 0.92f},
        {0.18f, 0.72f, 0.53f},
        {0.95f, 0.62f, 0.16f},
        {0.85f, 0.28f, 0.35f},
        {0.55f, 0.40f, 0.85f},
        {0.16f, 0.70f, 0.82f},
        {0.65f, 0.75f, 0.18f},
        {0.82f, 0.38f, 0.68f},
        {0.40f, 0.65f, 0.28f},
        {0.92f, 0.78f, 0.20f}
    };
    const int PAL = 10;

    glPushMatrix();
    if (m_chart3DMode) {
        glRotatef(m_chartRotX, 1, 0, 0);
        glRotatef(m_chartRotY, 0, 1, 0);
    }

    if (!m_chartPieMode) {
        int maxCount = 1;
        for (int i = 0; i < (int)stats.size(); ++i)
            if (stats[i].count > maxCount) maxCount = stats[i].count;

        int n = (int)stats.size();
        float gap = n <= 3 ? 2.4f : (n <= 6 ? 1.8f : (n <= 8 ? 1.45f : 1.2f));
        float totalWidth = (float)(n - 1) * gap;
        float startX = -totalWidth * 0.5f;
        float depth = m_chart3DMode ? 1.0f : 0.0f;
        float baseY = -3.0f;
        float maxBarH = 5.5f;
        float barW = gap * 0.62f;
        if (barW > 1.2f) barW = 1.2f;
        float zHalf = depth * 0.5f;
        float frontZ = zHalf + 0.01f;

        glDisable(GL_TEXTURE_2D);

        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        int gridN = 5;
        for (int gi = 1; gi <= gridN; ++gi) {
            float gy = baseY + maxBarH * ((float)gi / (float)gridN);
            int gridVal = (int)(maxCount * ((float)gi / (float)gridN) + 0.5f);
            glColor4f(0.72f, 0.74f, 0.78f, 0.45f);
            glLineWidth(1.0f);
            glBegin(GL_LINES);
            glVertex3f(startX - 1.2f, gy, frontZ);
            glVertex3f(startX + totalWidth + 1.2f, gy, frontZ);
            glEnd();
            CString gLabel;
            if (m_chartDataIndex == 3) {
                gLabel.Format(_T("%.1f"), gridVal / 10.0f);
            } else {
                gLabel.Format(_T("%d"), gridVal);
            }
            DrawText3D(startX - 1.8f, gy - 0.08f, frontZ, gLabel, 0.50f, 0.54f, 0.62f);
        }

        glColor4f(0.55f, 0.58f, 0.65f, 0.7f);
        glLineWidth(1.5f);
        glBegin(GL_LINES);
        glVertex3f(startX - 1.2f, baseY, frontZ);
        glVertex3f(startX + totalWidth + 1.2f, baseY, frontZ);
        glVertex3f(startX - 1.2f, baseY, frontZ);
        glVertex3f(startX - 1.2f, baseY + maxBarH + 0.3f, frontZ);
        glEnd();
        glEnable(GL_LIGHTING);

        for (int i = 0; i < n; ++i) {
            float normalized = (float)stats[i].count / (float)maxCount;
            float barH = max(0.12f, normalized * maxBarH);
            float x = startX + gap * (float)i;
            float cr = palette[i % PAL][0];
            float cg = palette[i % PAL][1];
            float cb = palette[i % PAL][2];
            float w2 = barW * 0.5f;
            float y0 = baseY;
            float y1 = y0 + barH;

            glColor3f(cr, cg, cb);
            glBegin(GL_QUADS);

            glNormal3f(0, 0, 1);
            glVertex3f(x - w2, y0, zHalf);
            glVertex3f(x + w2, y0, zHalf);
            glVertex3f(x + w2, y1, zHalf);
            glVertex3f(x - w2, y1, zHalf);

            glNormal3f(0, 0, -1);
            glVertex3f(x + w2, y0, -zHalf);
            glVertex3f(x - w2, y0, -zHalf);
            glVertex3f(x - w2, y1, -zHalf);
            glVertex3f(x + w2, y1, -zHalf);

            glNormal3f(1, 0, 0);
            glVertex3f(x + w2, y0, -zHalf);
            glVertex3f(x + w2, y0, zHalf);
            glVertex3f(x + w2, y1, zHalf);
            glVertex3f(x + w2, y1, -zHalf);

            glNormal3f(-1, 0, 0);
            glVertex3f(x - w2, y0, zHalf);
            glVertex3f(x - w2, y0, -zHalf);
            glVertex3f(x - w2, y1, -zHalf);
            glVertex3f(x - w2, y1, zHalf);

            glNormal3f(0, -1, 0);
            glVertex3f(x - w2, y0, -zHalf);
            glVertex3f(x + w2, y0, -zHalf);
            glVertex3f(x + w2, y0, zHalf);
            glVertex3f(x - w2, y0, zHalf);
            glEnd();

            glColor3f(min(1.0f, cr * 1.3f), min(1.0f, cg * 1.3f), min(1.0f, cb * 1.3f));
            glBegin(GL_QUADS);
            glNormal3f(0, 1, 0);
            glVertex3f(x - w2, y1, zHalf);
            glVertex3f(x + w2, y1, zHalf);
            glVertex3f(x + w2, y1, -zHalf);
            glVertex3f(x - w2, y1, -zHalf);
            glEnd();
        }

        glDisable(GL_LIGHTING);
        for (int i = 0; i < n; ++i) {
            float normalized = (float)stats[i].count / (float)maxCount;
            float barH = max(0.12f, normalized * maxBarH);
            float x = startX + gap * (float)i;
            CString name = stats[i].name;
            float maxLabelW = gap * 0.9f;
            float estW = Measure3DTextWidth(name);
            double lblScale = (estW > maxLabelW && estW > 0.01f) ? (double)(maxLabelW / estW) : 1.0;
            if (lblScale < 0.35) lblScale = 0.35;
            CString val;
            if (m_chartDataIndex == 3) {
                val.Format(_T("%.1f"), stats[i].count / 10.0f);
            } else {
                val.Format(_T("%d"), stats[i].count);
            }
            float nameW = Measure3DTextWidth(name, lblScale);
            DrawText3D(x - nameW * 0.5f, baseY - 0.55f, frontZ + 0.1f, name, 0.20f, 0.24f, 0.32f, lblScale);
            float valW = Measure3DTextWidth(val);
            DrawText3D(x - valW * 0.5f, baseY + barH + 0.15f, frontZ + 0.1f, val,
                palette[i % PAL][0] * 0.7f, palette[i % PAL][1] * 0.7f, palette[i % PAL][2] * 0.7f);
        }
        glEnable(GL_LIGHTING);
    }
    else {
        int total = 0;
        for (int i = 0; i < (int)stats.size(); ++i) total += max(0, stats[i].count);
        total = max(1, total);

        float radius = 3.2f;
        float h = m_chart3DMode ? 1.1f : 0.0f;
        float halfH = h * 0.5f;
        float angleStart = 0.0f;
        float explodeDist = 0.18f;

        for (int i = 0; i < (int)stats.size(); ++i) {
            if (stats[i].count <= 0) continue;
            float sweep = 360.0f * ((float)stats[i].count / (float)total);
            float cr = palette[i % PAL][0];
            float cg = palette[i % PAL][1];
            float cb = palette[i % PAL][2];

            float midA = (angleStart + sweep * 0.5f) * 3.1415926f / 180.0f;
            float ex = cosf(midA) * explodeDist;
            float ey = sinf(midA) * explodeDist;

            glPushMatrix();
            glTranslatef(ex, ey, 0.0f);

            glColor3f(cr, cg, cb);
            glBegin(GL_TRIANGLE_FAN);
            glNormal3f(0, 0, 1);
            glVertex3f(0.0f, 0.0f, halfH);
            for (int s = 0; s <= 48; ++s) {
                float a = (angleStart + sweep * ((float)s / 48.0f)) * 3.1415926f / 180.0f;
                glVertex3f(cosf(a) * radius, sinf(a) * radius, halfH);
            }
            glEnd();

            glColor3f(cr * 0.8f, cg * 0.8f, cb * 0.8f);
            glBegin(GL_TRIANGLE_FAN);
            glNormal3f(0, 0, -1);
            glVertex3f(0.0f, 0.0f, -halfH);
            for (int s = 48; s >= 0; --s) {
                float a = (angleStart + sweep * ((float)s / 48.0f)) * 3.1415926f / 180.0f;
                glVertex3f(cosf(a) * radius, sinf(a) * radius, -halfH);
            }
            glEnd();

            glColor3f(cr * 0.88f, cg * 0.88f, cb * 0.88f);
            glBegin(GL_QUAD_STRIP);
            for (int s = 0; s <= 48; ++s) {
                float a = (angleStart + sweep * ((float)s / 48.0f)) * 3.1415926f / 180.0f;
                float nx = cosf(a);
                float ny = sinf(a);
                glNormal3f(nx, ny, 0.0f);
                glVertex3f(nx * radius, ny * radius, -halfH);
                glVertex3f(nx * radius, ny * radius, halfH);
            }
            glEnd();

            float a0 = angleStart * 3.1415926f / 180.0f;
            float a1 = (angleStart + sweep) * 3.1415926f / 180.0f;
            glBegin(GL_QUADS);
            glNormal3f(sinf(a0), -cosf(a0), 0.0f);
            glVertex3f(0.0f, 0.0f, -halfH);
            glVertex3f(cosf(a0) * radius, sinf(a0) * radius, -halfH);
            glVertex3f(cosf(a0) * radius, sinf(a0) * radius, halfH);
            glVertex3f(0.0f, 0.0f, halfH);
            glNormal3f(-sinf(a1), cosf(a1), 0.0f);
            glVertex3f(0.0f, 0.0f, -halfH);
            glVertex3f(0.0f, 0.0f, halfH);
            glVertex3f(cosf(a1) * radius, sinf(a1) * radius, halfH);
            glVertex3f(cosf(a1) * radius, sinf(a1) * radius, -halfH);
            glEnd();

            glPopMatrix();
            angleStart += sweep;
        }

        glDisable(GL_LIGHTING);
        float legendX = radius + 1.8f;
        float legendTop = ((float)(stats.size()) * 0.45f) * 0.5f;
        int legendIdx = 0;
        angleStart = 0.0f;

        for (int i = 0; i < (int)stats.size(); ++i) {
            if (stats[i].count <= 0) continue;
            float sweep = 360.0f * ((float)stats[i].count / (float)total);
            int pct = (int)((stats[i].count * 100.0f / (float)total) + 0.5f);
            float cr = palette[i % PAL][0];
            float cg = palette[i % PAL][1];
            float cb = palette[i % PAL][2];

            float ly = legendTop - legendIdx * 0.55f;
            glColor3f(cr, cg, cb);
            glBegin(GL_QUADS);
            glVertex3f(legendX, ly, halfH + 0.05f);
            glVertex3f(legendX + 0.35f, ly, halfH + 0.05f);
            glVertex3f(legendX + 0.35f, ly - 0.35f, halfH + 0.05f);
            glVertex3f(legendX, ly - 0.35f, halfH + 0.05f);
            glEnd();

            CString name = stats[i].name;
            CString label;
            label.Format(_T("%s %d%%"), name.GetString(), pct);
            float estLW = Measure3DTextWidth(label);
            float maxLegW = 3.5f;
            double legScale = (estLW > maxLegW && estLW > 0.01f) ? (double)(maxLegW / estLW) : 1.0;
            if (legScale < 0.4) legScale = 0.4;
            DrawText3D(legendX + 0.5f, ly - 0.30f, halfH + 0.08f, label, 0.18f, 0.22f, 0.30f, legScale);

            if (pct >= 6) {
                float midA = (angleStart + sweep * 0.5f) * 3.1415926f / 180.0f;
                float eLbl = explodeDist;
                float lx = cosf(midA) * (radius * 0.6f) + cosf(midA) * eLbl;
                float lyp = sinf(midA) * (radius * 0.6f) + sinf(midA) * eLbl;
                CString pctLabel;
                pctLabel.Format(_T("%d%%"), pct);
                DrawText3D(lx - 0.15f, lyp - 0.08f, halfH + 0.06f, pctLabel, 1.0f, 1.0f, 1.0f);
            }

            angleStart += sweep;
            legendIdx++;
        }
        glEnable(GL_LIGHTING);
    }

    glDisable(GL_LIGHTING);
    float titleY = m_chartPieMode ? 4.2f : 3.2f;
    float titleW = Measure3DTextWidth(datasetTitle, 1.8);
    DrawText3D(-titleW * 0.5f, titleY, 0.5f, datasetTitle, 0.05f, 0.32f, 0.68f, 1.8);
    glEnable(GL_LIGHTING);

    glPopMatrix();
}

void CLibraryAppView::UpdateAnimations() {
    float lerp = 0.1f;
    float shelfZ = -9.0f, rowH = 3.5f, bw = 1.8f, gap = 0.4f, startY = 3.8f;
    int total = (int)m_books.size();
    for (int i = 0; i < total; i++) {
        BookData& b = m_books[i];
        if (m_bInspectMode && i == m_selectedIndex) {
            b.targetX = 0.0f;
            b.targetY = 0.0f;
            b.targetZ = m_inspectDist;
            b.targetRotX = m_inspectRotX;
            b.targetRotY = m_inspectRotY;
            b.targetRotZ = 0.0f;
        }
        else if (m_bInspectMode) {
            b.targetY = -25.0f;
        }
        else {
            int row = i / 3, col = i % 3, inRow = 3;
            if (row == (total - 1) / 3) {
                int rem = total % 3;
                if (rem != 0) inRow = rem;
            }
            float totalW = inRow * bw + (inRow - 1) * gap, rowStartX = -totalW / 2.0f + bw / 2.0f;
            b.targetX = rowStartX + col * (bw + gap);
            b.targetY = startY - row * rowH + m_scrollOffset;
            b.targetZ = shelfZ;
            if (i == m_hoverIndex) {
                b.targetZ += 0.8f;
                b.targetRotY = -15.0f;
            }
            else {
                b.targetRotX = b.targetRotY = b.targetRotZ = 0.0f;
            }
        }
        b.curX += (b.targetX - b.curX) * lerp;
        b.curY += (b.targetY - b.curY) * lerp;
        b.curZ += (b.targetZ - b.curZ) * lerp;
        b.curRotX += (b.targetRotX - b.curRotX) * lerp;
        b.curRotY += (b.targetRotY - b.curRotY) * lerp;
        b.curRotZ += (b.targetRotZ - b.curRotZ) * lerp;
    }
}

void CLibraryAppView::DrawInfoOverlay(CDC* pDC) {
    (void)pDC;
    CRect rect;
    GetClientRect(&rect);
    int left = rect.Width() / 2 + 30;
    int top = rect.Height() / 2 - 100;

    DrawTextGL(left, top, m_books[m_selectedIndex].title, 1.0f, 1.0f, 1.0f);

    CString desc = m_books[m_selectedIndex].description;

    if (desc.GetLength() > 200) desc = desc.Left(197) + _T("...");

    CString meta;
    meta.Format(_T("Рік: %s"), m_books[m_selectedIndex].year);

    DrawTextGL(left, top + 26, meta, 0.7f, 0.7f, 0.8f);
    DrawTextGL(left, top + 48, desc, 0.8f, 0.8f, 0.8f);

    auto reviews = m_db.GetReviewsByBook(m_books[m_selectedIndex].id);
    if (!reviews.empty()) {
        int y = top + 140;
        DrawTextGL(left, y, _T("Відгуки:"), 0.5f, 0.8f, 1.0f);
        int take = min(2, (int)reviews.size());
        for (int i = (int)reviews.size() - 1; i >= (int)reviews.size() - take; --i) {
            CString r;
            r.Format(_T("\u2605 %d  %s"), reviews[i].rating, reviews[i].comment.GetString());
            y += 22;
            DrawTextGL(left, y, r, 0.7f, 0.7f, 0.7f);
        }
    }
}

void CLibraryAppView::DrawShelf(float width, float depth) {

    float h = 0.15f, x = width / 2.0f, z = depth / 2.0f, y = h / 2.0f;
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.4f, 0.25f, 0.15f);

    glPushMatrix();
    glScalef(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glVertex3f(-x, y, z);
    glVertex3f(x, y, z);
    glVertex3f(x, y, -z);
    glVertex3f(-x, y, -z);
    glNormal3f(0, 0, 1);
    glVertex3f(-x, -y, z);
    glVertex3f(x, -y, z);
    glVertex3f(x, y, z);
    glVertex3f(-x, y, z);
    glEnd();
    glPopMatrix();
}

void CLibraryAppView::RenderScene(bool bPicking) {
    if (m_bInspectMode && !bPicking) {
        glDisable(GL_TEXTURE_2D);

        glBegin(GL_QUADS);
        glNormal3f(0, 0, 1);
        glColor3f(0.11f, 0.12f, 0.16f);
        glVertex3f(-22, -10, -18);
        glVertex3f(22, -10, -18);
        glColor3f(0.06f, 0.07f, 0.10f);
        glVertex3f(22, 14, -18);
        glVertex3f(-22, 14, -18);
        glEnd();

        glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        glColor3f(0.10f, 0.11f, 0.13f);
        glVertex3f(-22, -10, -20);
        glVertex3f(22, -10, -20);
        glColor3f(0.07f, 0.08f, 0.10f);
        glVertex3f(22, -10, 12);
        glVertex3f(-22, -10, 12);
        glEnd();

        glColor3f(0.24f, 0.26f, 0.30f);
        glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        glVertex3f(-2.0f, -1.7f, -5.2f);
        glVertex3f(2.0f, -1.7f, -5.2f);
        glVertex3f(2.0f, -1.7f, -2.8f);
        glVertex3f(-2.0f, -1.7f, -2.8f);
        glEnd();
    }

    if (!m_bInspectMode && !bPicking) {
        int rows = ((int)m_books.size() + 2) / 3;
        for (int r = 0; r < rows; r++) {
            glPushMatrix();
            float y = 3.8f - r * 3.5f + m_scrollOffset - 1.25f;
            if (y > -12.0f && y < 12.0f) {
                glTranslatef(0.0f, y, -9.0f);
                DrawShelf(8.0f, 2.5f);
            }
            glPopMatrix();
        }
    }

    for (int i = 0; i < (int)m_books.size(); i++) {
        BookData& b = m_books[i];
        if (bPicking) glPushName(i);
        glPushMatrix();
        glTranslatef(b.curX, b.curY, b.curZ);
        glRotatef(b.curRotX, 1, 0, 0);
        glRotatef(b.curRotY, 0, 1, 0);
        glRotatef(b.curRotZ, 0, 0, 1);
        DrawBookModel(1.8f, 2.5f, 0.45f, b.texIndex, b.r, b.g, b.b);
        glPopMatrix();
        if (bPicking) glPopName();
    }
}

void CLibraryAppView::DrawBookModel(float w, float h, float d, int texIdx, float r, float g, float b) {
    DrawBook(w, h, d, texIdx, r, g, b);
}

void CLibraryAppView::DrawBook(float w, float h, float d, int texIdx, float r, float g, float b) {
    const float x = w * 0.5f;
    const float y = h * 0.5f;
    const float z = d * 0.5f;

    const bool hasCoverTex = (texIdx >= 0 && texIdx < (int)m_texCovers.size() && m_texCovers[texIdx] != 0);
    const bool hasPaperTex = (m_texPaper != 0);

    GLfloat specStrong[] = {
        0.34f,
        0.34f,
        0.34f,
        1.0f
    };
    GLfloat specSoft[] = {
        0.05f,
        0.05f,
        0.05f,
        1.0f
    };
    GLfloat shinCover[] = {
        48.0f
    };
    GLfloat shinPaper[] = {
        8.0f
    };

    glEnable(GL_TEXTURE_2D);

    if (hasCoverTex) {
        glBindTexture(GL_TEXTURE_2D, m_texCovers[texIdx]);
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    else {
        glDisable(GL_TEXTURE_2D);
        glColor3f(r, g, b);
    }

    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specStrong);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shinCover);

    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glTexCoord2f(0, 0);
    glVertex3f(-x, -y, z);
    glTexCoord2f(1, 0);
    glVertex3f(x, -y, z);
    glTexCoord2f(1, 1);
    glVertex3f(x, y, z);
    glTexCoord2f(0, 1);
    glVertex3f(-x, y, z);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glColor3f(r * 0.82f, g * 0.82f, b * 0.82f);
    glBegin(GL_QUADS);
    glNormal3f(0, 0, -1);
    glVertex3f(-x, -y, -z);
    glVertex3f(-x, y, -z);
    glVertex3f(x, y, -z);
    glVertex3f(x, -y, -z);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glColor3f(r * 0.58f, g * 0.58f, b * 0.58f);
    glBegin(GL_QUADS);
    glNormal3f(-1, 0, 0);
    glVertex3f(-x, -y, -z);
    glVertex3f(-x, -y, z);
    glVertex3f(-x, y, z);
    glVertex3f(-x, y, -z);
    glEnd();

    if (hasPaperTex) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_texPaper);
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    else {
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.95f, 0.95f, 0.92f);
    }

    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specSoft);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shinPaper);

    glBegin(GL_QUADS);
    glNormal3f(1, 0, 0);
    glTexCoord2f(0, 0);
    glVertex3f(x, -y, -z);
    glTexCoord2f(1, 0);
    glVertex3f(x, y, -z);
    glTexCoord2f(1, 1);
    glVertex3f(x, y, z);
    glTexCoord2f(0, 1);
    glVertex3f(x, -y, z);
    glEnd();

    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glTexCoord2f(0, 0);
    glVertex3f(-x, y, -z);
    glTexCoord2f(1, 0);
    glVertex3f(x, y, -z);
    glTexCoord2f(1, 1);
    glVertex3f(x, y, z);
    glTexCoord2f(0, 1);
    glVertex3f(-x, y, z);
    glEnd();

    glBegin(GL_QUADS);
    glNormal3f(0, -1, 0);
    glTexCoord2f(0, 1);
    glVertex3f(-x, -y, -z);
    glTexCoord2f(1, 1);
    glVertex3f(x, -y, -z);
    glTexCoord2f(1, 0);
    glVertex3f(x, -y, z);
    glTexCoord2f(0, 0);
    glVertex3f(-x, -y, z);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

bool CLibraryAppView::EnsureLoggedIn() {
    if (m_db.IsLoggedIn()) return true;
    AfxMessageBox(_T("Спочатку увійдіть."));
    return false;
}
bool CLibraryAppView::EnsureAdmin() {
    if (!EnsureLoggedIn()) return false;
    if (m_db.IsAdmin()) return true;
    AfxMessageBox(_T("Доступ тільки для адміністратора."));
    return false;
}

void CLibraryAppView::OnLButtonDown(UINT nFlags, CPoint point) {
    SetFocus();
    if (m_screen == SCREEN_ANALYTICS) {
        if (m_chart3DMode) {
            m_chartDragging = true;
            m_lastMousePos = point;
            SetCapture();
        }
        CView::OnLButtonDown(nFlags, point);
        return;
    }
    if (m_screen == SCREEN_3D_INSPECT) {
        m_isDragging = true;
        m_lastMousePos = point;
        SetCapture();
        CView::OnLButtonDown(nFlags, point);
        return;
    }
    if (m_screen == SCREEN_BOOK_DETAILS) {
        CView::OnLButtonDown(nFlags, point);
        return;
    }
    if (m_screen != SCREEN_BOOK_LIST || !m_db.IsLoggedIn()) {
        CView::OnLButtonDown(nFlags, point);
        return;
    }

    CRect rc;
    GetClientRect(&rc);

    if (!m_bInspectMode) {
        CRect headerRect(m_tableRect.left, m_tableRect.top, m_tableRect.right, m_tableRect.top + m_tableHeaderH);
        if (headerRect.PtInRect(point))
        {
            float cw[] = { 0.38f, 0.20f, 0.08f, 0.18f, 0.16f };
            int x = m_tableRect.left;
            int hitCol = -1;
            for (int i = 0; i < 5; ++i)
            {
                int w = (int)(m_tableRect.Width() * cw[i]);
                CRect rcCol(x, headerRect.top, x + w, headerRect.bottom);
                if (rcCol.PtInRect(point))
                {
                    hitCol = i;
                    break;
                }
                x += w;
            }

            Filter::SortMode newMode = m_filter.GetSortMode();
            bool sortable = true;
            switch (hitCol)
            {
            case 0: newMode = Filter::SORT_TITLE; break;
            case 1: newMode = Filter::SORT_AUTHOR; break;
            case 2: newMode = Filter::SORT_YEAR; break;
            case 3: newMode = Filter::SORT_CATEGORY; break;
            case 4: newMode = Filter::SORT_RATING; break;
            default: sortable = false; break;
            }

            if (sortable)
            {
                if (newMode == m_filter.GetSortMode())
                {
                    m_filter.ToggleSortDirection();
                }
                else
                {
                    m_filter.SetSortMode(newMode);
                }
                m_currentPage = 0;
                ReloadBooksPage();
                Invalidate(FALSE);
                CView::OnLButtonDown(nFlags, point);
                return;
            }
        }

        int row = HitTestTableRow(point);
        if (row >= 0 && row < (int)m_books.size()) {
            m_selectedIndex = row;
            m_contextBookId = m_books[row].id;
        }
        CView::OnLButtonDown(nFlags, point);
        return;
    }

    CView::OnLButtonDown(nFlags, point);
}

void CLibraryAppView::OnLButtonDblClk(UINT nFlags, CPoint point) {
    if (m_screen == SCREEN_BOOK_LIST && !m_bInspectMode && m_db.IsLoggedIn()) {
        int row = HitTestTableRow(point);
        if (row >= 0 && row < (int)m_books.size()) {
            m_detailsBookId = m_books[row].id;
            m_detailsScrollY = 0.0f;
            m_screen = SCREEN_BOOK_DETAILS;
            Invalidate(FALSE);
            CView::OnLButtonDblClk(nFlags, point);
            return;
        }
    }
    CView::OnLButtonDblClk(nFlags, point);
}

void CLibraryAppView::OnLButtonUp(UINT nFlags, CPoint point) {
    if (m_isDragging || m_chartDragging) {
        m_isDragging = false;
        m_chartDragging = false;
        ReleaseCapture();
    }
    CView::OnLButtonUp(nFlags, point);
}

void CLibraryAppView::OnRButtonUp(UINT nFlags, CPoint point) {
    CPoint screenPoint = point;
    ClientToScreen(&screenPoint);
    if (m_screen == SCREEN_PROFILE)
        ShowProfileContextMenu(screenPoint);
    else
        ShowBookContextMenu(screenPoint);
    CView::OnRButtonUp(nFlags, point);
}

void CLibraryAppView::ShowBookContextMenu(CPoint screenPoint) {
    if (m_screen != SCREEN_BOOK_LIST || !m_db.IsLoggedIn()) return;
    CPoint clientPoint = screenPoint;
    ScreenToClient(&clientPoint);
    int hit = m_bInspectMode ? m_selectedIndex : HitTestTableRow(clientPoint);
    if (hit < 0 && m_tableHoverRow >= 0 && m_tableHoverRow < (int)m_books.size()) hit = m_tableHoverRow;
    if (hit < 0 || hit >= (int)m_books.size()) return;
    m_contextBookId = m_books[hit].id;
    m_selectedIndex = hit;

    CMenu menu;
    menu.CreatePopupMenu();
    menu.AppendMenu(MF_STRING, ID_CONTEXT_DETAILS, _T("Переглянути детально"));
    menu.AppendMenu(MF_STRING, ID_CONTEXT_3D_INSPECT, _T("3D огляд"));
    menu.AppendMenu(MF_SEPARATOR);
    menu.AppendMenu(MF_STRING, ID_CONTEXT_RESERVE, _T("Забронювати"));
    menu.AppendMenu(MF_STRING, ID_CONTEXT_REVIEW, _T("Залишити відгук"));
    
    UINT nCmd = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, screenPoint.x, screenPoint.y, this);
    if (nCmd != 0) {
        OnCommand(nCmd, 0);
    }
    menu.DestroyMenu();
    Invalidate(FALSE);
}

void CLibraryAppView::OnContextMenu(CWnd* pWnd, CPoint point) {
    (void)pWnd;
    if (point.x == -1 && point.y == -1) {
        ::GetCursorPos(&point);
    }
    if (m_screen == SCREEN_PROFILE)
        ShowProfileContextMenu(point);
    else
        ShowBookContextMenu(point);
}

void CLibraryAppView::ShowProfileContextMenu(CPoint screenPoint)
{
    if (!EnsureLoggedIn() || m_screen != SCREEN_PROFILE) return;

    m_profileContextReservationId = 0;
    m_profileContextReviewId = 0;
    m_profileContextReservationStatus.Empty();

    CPoint pt = screenPoint;
    ScreenToClient(&pt);

    CRect rc;
    GetClientRect(&rc);
    CRect panel(12, 12, rc.right - 12, rc.bottom - 12);
    CRect scrollRect(panel.left + 10, panel.top + 58, panel.right - 10, panel.bottom - 10);
    if (!scrollRect.PtInRect(pt)) return;

    auto reservations = m_db.GetCurrentUserReservations();
    auto reviews = m_db.GetCurrentUserReviews();

    std::set<CString> readBooksSet;
    for (const auto& rec : reservations)
    {
        CString status = rec.status;
        status.MakeLower();
        if (status == _T("returned") && !rec.bookTitle.IsEmpty())
            readBooksSet.insert(rec.bookTitle);
    }
    std::vector<CString> readBooks(readBooksSet.begin(), readBooksSet.end());

    int y = scrollRect.top + 10 + (int)m_profileScrollY;
    y += 24;

    int clickedReservationIdx = -1;
    if (!reservations.empty())
    {
        for (int i = 0; i < (int)reservations.size(); ++i)
        {
            CRect item(scrollRect.left + 8, y, scrollRect.right - 8, y + 46);
            if (item.PtInRect(pt))
            {
                clickedReservationIdx = i;
                break;
            }
            y += 52;
        }
    }
    else
    {
        y += 30;
    }

    y += 4;
    y += 24;
    if (readBooks.empty()) y += 28;
    else y += (int)readBooks.size() * 24;
    y += 2;
    y += 24;

    int clickedReviewIdx = -1;
    if (!reviews.empty())
    {
        for (int i = 0; i < (int)reviews.size(); ++i)
        {
            CRect item(scrollRect.left + 8, y, scrollRect.right - 8, y + 48);
            if (item.PtInRect(pt))
            {
                clickedReviewIdx = i;
                break;
            }
            y += 54;
        }
    }

    CMenu menu;
    menu.CreatePopupMenu();

    if (clickedReviewIdx >= 0 && clickedReviewIdx < (int)reviews.size())
    {
        m_profileContextReviewId = reviews[clickedReviewIdx].id;
        menu.AppendMenu(MF_STRING, ID_CONTEXT_PROFILE_EDIT_REVIEW, _T("Редагувати відгук"));
        menu.AppendMenu(MF_STRING, ID_CONTEXT_PROFILE_DELETE_REVIEW, _T("Видалити відгук"));
    }
    else if (clickedReservationIdx >= 0 && clickedReservationIdx < (int)reservations.size())
    {
        m_profileContextReservationId = reservations[clickedReservationIdx].id;
        m_profileContextReservationStatus = reservations[clickedReservationIdx].status;

        CString status = m_profileContextReservationStatus;
        status.MakeLower();
        if (status == _T("reserved") || status == _T("ready_for_pickup") || status == _T("return_requested"))
        {
            menu.AppendMenu(MF_STRING, ID_CONTEXT_PROFILE_CANCEL_RESERVATION, _T("Скасувати бронювання"));
        }
        if (status == _T("issued") || status == _T("overdue") || status == _T("return_requested") || status == _T("ready_for_pickup"))
        {
            menu.AppendMenu(MF_STRING, ID_CONTEXT_PROFILE_REPORT_LOST, _T("Повідомити про втрату"));
            menu.AppendMenu(MF_STRING, ID_CONTEXT_PROFILE_REPORT_DAMAGED, _T("Повідомити про пошкодження"));
        }
    }

    if (menu.GetMenuItemCount() <= 0)
    {
        menu.DestroyMenu();
        return;
    }

    UINT cmd = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, screenPoint.x, screenPoint.y, this);
    if (cmd != 0) OnCommand(cmd, 0);
    menu.DestroyMenu();
}

void CLibraryAppView::OnMouseMove(UINT nFlags, CPoint point) {
    if (m_chartDragging && m_screen == SCREEN_ANALYTICS && m_chart3DMode) {
        int dx = point.x - m_lastMousePos.x, dy = point.y - m_lastMousePos.y;
        m_chartRotY += dx * 0.6f;
        m_chartRotX += dy * 0.6f;
        m_lastMousePos = point;
        Invalidate(FALSE);
        CView::OnMouseMove(nFlags, point);
        return;
    }
    if (m_isDragging && (m_bInspectMode || m_screen == SCREEN_3D_INSPECT)) {
        int dx = point.x - m_lastMousePos.x, dy = point.y - m_lastMousePos.y;
        m_inspectRotY += dx * 0.5f;
        m_inspectRotX += dy * 0.5f;
        m_lastMousePos = point;
        Invalidate(FALSE);
    }
    else if (!m_bInspectMode && m_screen == SCREEN_BOOK_LIST) {
        int row = HitTestTableRow(point);
        if (row != m_tableHoverRow) m_tableHoverRow = row;

        int btnW = 60;
        int btnH = 20;
        int btnY = m_tableRect.bottom + 8;
        
        CRect btnPrev(m_tableRect.left, btnY, m_tableRect.left + btnW, btnY + btnH);
        CRect btnNext(m_tableRect.right - btnW, btnY, m_tableRect.right, btnY + btnH);

        bool prevHover = btnPrev.PtInRect(point) && m_currentPage > 0;
        bool nextHover = btnNext.PtInRect(point) && m_currentPage + 1 < m_totalPages;

        if (m_hoverBtnPrev != prevHover || m_hoverBtnNext != nextHover) {
            m_hoverBtnPrev = prevHover;
            m_hoverBtnNext = nextHover;
            Invalidate(FALSE);
        }
    }
    CView::OnMouseMove(nFlags, point);
}

BOOL CLibraryAppView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) {
    (void)nFlags;
    (void)pt;
    if (m_screen == SCREEN_ANALYTICS) {
        m_chartZoom += (zDelta > 0) ? 1.0f : -1.0f;
        if (m_chartZoom > -4.0f) m_chartZoom = -4.0f;
        if (m_chartZoom < -30.0f) m_chartZoom = -30.0f;
        Invalidate(FALSE);
        return TRUE;
    }
    if (m_screen == SCREEN_3D_INSPECT) {
        if (zDelta > 0) m_inspectDist += 0.5f;
        else m_inspectDist -= 0.5f;
        if (m_inspectDist > -2.0f) m_inspectDist = -2.0f;
        if (m_inspectDist < -10.0f) m_inspectDist = -10.0f;
        Invalidate(FALSE);
        return TRUE;
    }
    if (m_screen == SCREEN_PROFILE)
    {
        const float step = 36.0f;
        m_profileScrollY += (zDelta > 0) ? step : -step;
        Invalidate(FALSE);
        return TRUE;
    }
    if (m_screen == SCREEN_BOOK_DETAILS)
    {
        const float step = 28.0f;
        m_detailsScrollY += (zDelta > 0) ? step : -step;
        Invalidate(FALSE);
        return TRUE;
    }
    if (m_screen != SCREEN_BOOK_LIST) return TRUE;
    if (m_bInspectMode) {
        if (zDelta > 0) m_inspectDist += 0.5f;
        else m_inspectDist -= 0.5f;
        if (m_inspectDist > -2.0f) m_inspectDist = -2.0f;
        if (m_inspectDist < -10.0f) m_inspectDist = -10.0f;
    }
    else {
        float speed = 0.8f;
        if (zDelta > 0) m_scrollOffset -= speed;
        else m_scrollOffset += speed;
        int rows = ((int)m_books.size() + 2) / 3;
        float maxScroll = (rows * 3.5f) - 3.5f;
        if (m_scrollOffset < -1.0f) m_scrollOffset = -1.0f;
        if (m_scrollOffset > maxScroll) m_scrollOffset = maxScroll;
    }
    return TRUE;
}

void CLibraryAppView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
    if (m_screen == SCREEN_3D_INSPECT && nChar == VK_ESCAPE) {
        m_screen = SCREEN_BOOK_LIST;
        Invalidate(FALSE);
        CView::OnKeyDown(nChar, nRepCnt, nFlags);
        return;
    }
    if (m_screen == SCREEN_BOOK_DETAILS && nChar == VK_ESCAPE) {
        m_screen = SCREEN_BOOK_LIST;
        Invalidate(FALSE);
        CView::OnKeyDown(nChar, nRepCnt, nFlags);
        return;
    }
    if (m_screen == SCREEN_BOOK_LIST && m_bInspectMode && nChar == VK_ESCAPE) {
        m_bInspectMode = false;
        m_selectedIndex = -1;
        Invalidate(FALSE);
        CView::OnKeyDown(nChar, nRepCnt, nFlags);
        return;
    }
    if (m_screen == SCREEN_BOOK_LIST && !m_bInspectMode) {
        if (nChar == VK_LEFT && m_currentPage > 0) {
            m_currentPage--;
            ReloadBooksPage();
        }
        else if (nChar == VK_RIGHT && m_currentPage + 1 < m_totalPages) {
            m_currentPage++;
            ReloadBooksPage();
        }
        else if (nChar == VK_UP) {
            m_filter.PrevCategory();
            m_currentPage = 0;
            ReloadBooksPage();
        }
        else if (nChar == VK_DOWN) {
            m_filter.NextCategory();
            m_currentPage = 0;
            ReloadBooksPage();
        }
        Invalidate(FALSE);
    }
    else if (m_screen == SCREEN_ANALYTICS) {
        if (m_chart3DMode) {
            if (nChar == 'W') m_chartRotX -= 4.0f;
            else if (nChar == 'S') m_chartRotX += 4.0f;
            else if (nChar == 'A') m_chartRotY -= 4.0f;
            else if (nChar == 'D') m_chartRotY += 4.0f;
        }
        if (nChar == VK_LEFT) m_chartDataIndex = (m_chartDataIndex + 15) % 16;
        else if (nChar == VK_RIGHT) m_chartDataIndex = (m_chartDataIndex + 1) % 16;
        Invalidate(FALSE);
    }
    CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CLibraryAppView::OnCmdLogin() {
    DAuth dlg(FALSE, this);
    if (dlg.DoModal() != IDOK) return;
    CString err;
    if (!m_db.Login(dlg.GetLogin(), dlg.GetPassword(), err)) {
        AfxMessageBox(_T("Невірний логін або пароль."));
        return;
    }
    m_currentPage = 0;
    m_filter.SetCategories(m_db.GetAllCategories());
    ReloadBooksPage();
    m_screen = SCREEN_BOOK_LIST;
    Invalidate(FALSE);
}

void CLibraryAppView::OnCmdRegister() {
    DAuth dlg(TRUE, this);
    if (dlg.DoModal() != IDOK) return;
    CString err;
    if (!m_db.RegisterUser(dlg.GetLogin(), dlg.GetPassword(), dlg.GetEmail(), dlg.GetPhone(), dlg.GetFullName(), err)) {
        AfxMessageBox(err);
        return;
    }
    m_currentPage = 0;
    m_filter.SetCategories(m_db.GetAllCategories());
    ReloadBooksPage();
    m_screen = SCREEN_BOOK_LIST;
    Invalidate(FALSE);
}

void CLibraryAppView::OnCmdLogout() {
    m_db.Logout();
    m_screen = SCREEN_AUTH_REQUIRED;
    m_bInspectMode = false;
    m_selectedIndex = -1;
    Invalidate(FALSE);
}

void CLibraryAppView::OnCmdMainMenu() {
    if (EnsureLoggedIn()) {
        m_screen = SCREEN_BOOK_LIST;
        Invalidate(FALSE);
    }
}

void CLibraryAppView::OnCmdProfile() {
    if (EnsureLoggedIn()) {
        m_profileScrollY = 0.0f;
        m_screen = SCREEN_PROFILE;
        Invalidate(FALSE);
    }
}

void CLibraryAppView::OnCmdAddBook() {
    if (!EnsureAdmin()) return;
    DAddBook dlg(this);
    if (dlg.DoModal() != IDOK) return;
    BookRecord b = dlg.GetBook();
    CString msg;
    m_db.AddBook(b, msg);
    AfxMessageBox(msg);
    ReloadBooksPage();
}
void CLibraryAppView::OnCmdEditBook() {
    if (!EnsureAdmin()) return;
    if (m_books.empty()) {
        AfxMessageBox(_T("Немає книг."));
        return;
    }
    int idx = (m_selectedIndex >= 0 && m_selectedIndex < (int)m_books.size()) ? m_selectedIndex : 0;
    BookRecord b;
    if (!m_db.GetBookById(m_books[idx].id, b)) return;
    DEditBook dlg(this);
    dlg.SetBook(b);
    if (dlg.DoModal() != IDOK) return;
    b = dlg.GetBook();
    CString msg;
    m_db.EditBook(b, msg);
    AfxMessageBox(msg);
    ReloadBooksPage();
}
void CLibraryAppView::OnCmdDeleteBook() {
    if (!EnsureAdmin()) return;
    if (m_books.empty()) return;
    int idx = (m_selectedIndex >= 0 && m_selectedIndex < (int)m_books.size()) ? m_selectedIndex : 0;
    DDeleteBook dlg(this);
    dlg.SetBookId(m_books[idx].id);
    if (dlg.DoModal() != IDOK) return;
    int idToDelete = dlg.GetBookId();
    if (idToDelete <= 0) idToDelete = m_books[idx].id;
    CString msg;
    m_db.DeleteBook(idToDelete, msg);
    AfxMessageBox(msg);
    ReloadBooksPage();
}
void CLibraryAppView::OnCmdUsersReservations() {
    if (!EnsureAdmin()) return;
    DUsersAndReservations dlg(this);
    dlg.SetRows(m_db.GetUsersReservationsRows());
    dlg.DoModal();
}

void CLibraryAppView::OnCmdAddBranch() {
    if (!EnsureAdmin()) return;
    DAddBranch dlg(this);
    if (dlg.DoModal() != IDOK) return;
    CString err;
    if (m_db.AddBranch(dlg.GetBranchName(), dlg.GetBranchAddress(), err)) {
        AfxMessageBox(_T("Філію додано успішно."));
    } else {
        AfxMessageBox(_T("Помилка: ") + err);
    }
}

void CLibraryAppView::OnCmdEditBranch() {
    if (!EnsureAdmin()) return;
    DEditBranch dlg(this);
    if (dlg.DoModal() != IDOK) return;
    CString err;
    if (m_db.EditBranch(dlg.GetSelectedBranchId(), dlg.GetBranchName(), dlg.GetBranchAddress(), err)) {
        AfxMessageBox(_T("Філію оновлено успішно."));
    } else {
        AfxMessageBox(_T("Помилка: ") + err);
    }
}

void CLibraryAppView::OnCmdDeleteBranch() {
    if (!EnsureAdmin()) return;
    DDeleteBranch dlg(this);
    if (dlg.DoModal() != IDOK) return;
    CString err;
    if (m_db.DeleteBranch(dlg.GetSelectedBranchId(), err)) {
        AfxMessageBox(_T("Філію видалено успішно."));
    } else {
        AfxMessageBox(_T("Помилка: ") + err);
    }
}

void CLibraryAppView::OnCmdAddCategory() {
    if (!EnsureAdmin()) return;
    DAddCategory dlg(this);
    if (dlg.DoModal() != IDOK) return;
    CString name = dlg.GetCategoryName();
    if (name.IsEmpty()) return;
    CString err;
    if (m_db.AddCategory(name, err)) {
        AfxMessageBox(_T("Категорію додано успішно."));
        m_filter.SetCategories(m_db.GetAllCategories());
        m_currentPage = 0;
        ReloadBooksPage();
    } else {
        AfxMessageBox(_T("Помилка: ") + err);
    }
}

void CLibraryAppView::OnCmdEditCategory() {
    if (!EnsureAdmin()) return;
    DEditCategory dlg(this);
    if (dlg.DoModal() != IDOK) return;
    int id = dlg.GetSelectedCategoryId();
    CString name = dlg.GetCategoryName();
    if (id <= 0 || name.IsEmpty()) return;
    CString err;
    if (m_db.EditCategory(id, name, err)) {
        AfxMessageBox(_T("Категорію оновлено успішно."));
        m_filter.SetCategories(m_db.GetAllCategories());
        m_currentPage = 0;
        ReloadBooksPage();
    } else {
        AfxMessageBox(_T("Помилка: ") + err);
    }
}

void CLibraryAppView::OnCmdDeleteCategory() {
    if (!EnsureAdmin()) return;
    DDeleteCategory dlg(this);
    if (dlg.DoModal() != IDOK) return;
    int id = dlg.GetSelectedCategoryId();
    if (id <= 0) return;
    CString err;
    if (m_db.DeleteCategory(id, err)) {
        AfxMessageBox(_T("Категорію видалено успішно."));
        m_filter.SetCategories(m_db.GetAllCategories());
        m_currentPage = 0;
        ReloadBooksPage();
    } else {
        AfxMessageBox(_T("Помилка: ") + err);
    }
}

void CLibraryAppView::OnCmdAnalyticsView() {
    if (EnsureLoggedIn()) {
        m_screen = SCREEN_ANALYTICS;
        Invalidate(FALSE);
    }
}
void CLibraryAppView::OnCmdAnalyticsChartType() {
    if (!EnsureLoggedIn()) return;
    m_chartPieMode = !m_chartPieMode;
    Invalidate(FALSE);
}
void CLibraryAppView::OnCmdAnalyticsDimension() {
    if (!EnsureLoggedIn()) return;
    m_chart3DMode = !m_chart3DMode;
    Invalidate(FALSE);
}
void CLibraryAppView::OnCmdAnalyticsReset() {
    if (EnsureLoggedIn()) {
        m_chartRotX = 20.0f;
        m_chartRotY = -20.0f;
        m_chartZoom = -12.0f;
        m_chartDataIndex = 0;
        Invalidate(FALSE);
    }
}
void CLibraryAppView::OnContextReserve() {
    if (!EnsureLoggedIn() || m_contextBookId == 0) return;
    DReservation dlg(this);
    if (dlg.DoModal() != IDOK) return;
    CString msg;
    m_db.ReserveBook(m_contextBookId, dlg.GetDays(), dlg.GetBranch(), msg);
    AfxMessageBox(msg);
    ReloadBooksPage();
}
void CLibraryAppView::OnContextCancelReservation() {
    if (!EnsureLoggedIn() || m_contextBookId == 0) return;
    CString msg;
    if (!m_db.CancelActiveReservationForCurrentUser(m_contextBookId, msg))
    {
        AfxMessageBox(msg);
        return;
    }
    AfxMessageBox(msg);
    ReloadBooksPage();
}
void CLibraryAppView::OnContextReview() {
    if (!EnsureLoggedIn() || m_contextBookId == 0) return;
    if (!m_db.CanUserReview(m_db.GetCurrentUserId(), m_contextBookId)) {
        AfxMessageBox(_T("Ви можете залишити відгук лише після успішного повернення книги."));
        return;
    }
    DAddReview dlg(this);
    if (dlg.DoModal() != IDOK) return;
    CString msg;
    m_db.AddReview(m_contextBookId, dlg.GetRating(), dlg.GetComment(), msg);
    AfxMessageBox(msg);
    ReloadBooksPage();
}
void CLibraryAppView::OnContextDetails() {
    if (!EnsureLoggedIn() || m_contextBookId == 0) return;
    m_detailsBookId = m_contextBookId;
    m_detailsScrollY = 0.0f;
    m_screen = SCREEN_BOOK_DETAILS;
    Invalidate(FALSE);
}

void CLibraryAppView::OnContext3DInspect() {
    if (!EnsureLoggedIn() || m_contextBookId == 0) return;
    for (int i = 0; i < (int)m_books.size(); ++i) {
        if (m_books[i].id == m_contextBookId) {
            m_selectedIndex = i;

            if (m_books[i].texIndex >= 0 && m_books[i].texIndex < (int)m_texCovers.size() && m_texCovers[m_books[i].texIndex] == 0)
            {
                COLORREF avg = RGB(110, 128, 150);
                GLuint tex = LoadTextureFromFile(m_books[i].coverPath, &avg);
                if (tex != 0)
                {
                    m_texCovers[m_books[i].texIndex] = tex;
                    if (m_books[i].texIndex < (int)m_coverAvgColors.size())
                    {
                        m_coverAvgColors[m_books[i].texIndex] = avg;
                    }
                }
            }

            m_screen = SCREEN_3D_INSPECT;
            m_inspectRotX = 0.0f;
            m_inspectRotY = 0.0f;
            m_inspectDist = -4.0f;
            Invalidate(FALSE);
            return;
        }
    }
}

void CLibraryAppView::OnContextProfileEditReview()
{
    if (!EnsureLoggedIn() || m_profileContextReviewId <= 0) return;

    auto reviews = m_db.GetCurrentUserReviews();
    ReviewRecord selected;
    bool found = false;
    for (const auto& r : reviews)
    {
        if (r.id == m_profileContextReviewId)
        {
            selected = r;
            found = true;
            break;
        }
    }
    if (!found)
    {
        AfxMessageBox(_T("Відгук не знайдено."));
        return;
    }

    DEditReview dlg(this);
    dlg.SetInitialData(selected.rating, selected.comment);
    if (dlg.DoModal() != IDOK) return;

    CString msg;
    if (!m_db.EditCurrentUserReview(m_profileContextReviewId, dlg.GetRating(), dlg.GetComment(), msg))
    {
        AfxMessageBox(msg);
        return;
    }

    AfxMessageBox(msg);
    ReloadBooksPage();
    Invalidate(FALSE);
}

void CLibraryAppView::OnContextProfileDeleteReview()
{
    if (!EnsureLoggedIn() || m_profileContextReviewId <= 0) return;

    DDeleteReview dlg(this);
    dlg.SetPrompt(_T("Видалити цей відгук? Дію не можна скасувати."));
    if (dlg.DoModal() != IDOK) return;

    CString msg;
    if (!m_db.DeleteCurrentUserReview(m_profileContextReviewId, msg))
    {
        AfxMessageBox(msg);
        return;
    }

    AfxMessageBox(msg);
    ReloadBooksPage();
    Invalidate(FALSE);
}

void CLibraryAppView::OnContextProfileCancelReservation()
{
    if (!EnsureLoggedIn() || m_profileContextReservationId <= 0) return;
    CString msg;
    if (!m_db.UpdateCurrentUserReservationStatus(m_profileContextReservationId, _T("cancelled"), msg))
    {
        AfxMessageBox(msg);
        return;
    }
    AfxMessageBox(msg);
    ReloadBooksPage();
    Invalidate(FALSE);
}

void CLibraryAppView::OnContextProfileReportLost()
{
    if (!EnsureLoggedIn() || m_profileContextReservationId <= 0) return;
    CString msg;
    if (!m_db.UpdateCurrentUserReservationStatus(m_profileContextReservationId, _T("lost"), msg))
    {
        AfxMessageBox(msg);
        return;
    }
    AfxMessageBox(msg);
    ReloadBooksPage();
    Invalidate(FALSE);
}

void CLibraryAppView::OnContextProfileReportDamaged()
{
    if (!EnsureLoggedIn() || m_profileContextReservationId <= 0) return;
    CString msg;
    if (!m_db.UpdateCurrentUserReservationStatus(m_profileContextReservationId, _T("damaged"), msg))
    {
        AfxMessageBox(msg);
        return;
    }
    AfxMessageBox(msg);
    ReloadBooksPage();
    Invalidate(FALSE);
}

void CLibraryAppView::DrawBookDetailsScreen(CDC* pDC) {
    CRect rc;
    GetClientRect(&rc);

    BookRecord book;
    if (!m_db.GetBookById(m_detailsBookId, book)) {
        pDC->SetTextColor(RGB(200, 60, 60));
        pDC->DrawText(_T("Книгу не знайдено"), &rc, DT_CENTER | DT_VCENTER);
        return;
    }

    int cardW = m_detailsCardW;
    int cardH = m_detailsCardH;
    int cardX = (rc.Width() - cardW) / 2;
    int cardY = (rc.Height() - cardH) / 2;
    CRect card(cardX, cardY, cardX + cardW, cardY + cardH);

    pDC->FillSolidRect(&card, RGB(255, 255, 255));
    pDC->Draw3dRect(&card, RGB(210, 215, 220), RGB(210, 215, 220));

    CRect hdr = card;
    hdr.bottom = hdr.top + 36;
    hdr.left += 12;
    hdr.right -= 12;
    pDC->SetTextColor(RGB(0, 100, 190));
    CFont* old = pDC->SelectObject(&m_fontTitle);
    pDC->DrawText(book.title, &hdr, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    pDC->SelectObject(&m_fontNormal);

    int coverW = 180;
    int coverH = 240;
    int coverX = card.left + 12;
    int coverY = card.top + 44;
    CRect coverRect(coverX, coverY, coverX + coverW, coverY + coverH);

    pDC->FillSolidRect(&coverRect, RGB(32, 36, 44));
    pDC->Draw3dRect(&coverRect, RGB(64, 74, 90), RGB(64, 74, 90));

    CString coverPath = ResolveCoverPath(book.coverPath);
    if (!coverPath.IsEmpty()) {
        CImage img;
        if (SUCCEEDED(img.Load(coverPath))) {
            HDC hDC = pDC->GetSafeHdc();
            int oldStretch = ::SetStretchBltMode(hDC, HALFTONE);
            POINT oldOrg = { 0, 0 };
            ::SetBrushOrgEx(hDC, 0, 0, &oldOrg);
            int imgW = img.GetWidth();
            int imgH = img.GetHeight();
            int drawW = min(coverW - 8, (coverH - 8) * imgW / imgH);
            int drawH = min(coverH - 8, drawW * imgH / imgW);
            int drawX = coverX + 4 + (coverW - 8 - drawW) / 2;
            int drawY = coverY + 4 + (coverH - 8 - drawH) / 2;
            img.Draw(hDC, drawX, drawY, drawW, drawH);
            ::SetBrushOrgEx(hDC, oldOrg.x, oldOrg.y, nullptr);
            ::SetStretchBltMode(hDC, oldStretch);
        }
    }

    int infoX = coverX + coverW + 16;
    int infoY = card.top + 44;
    int infoW = card.right - infoX - 12;
    int lineH = 22;

    pDC->SetTextColor(RGB(70, 70, 70));

    CString author; author.Format(_T("Автор: %s"), book.author.GetString());
    pDC->DrawText(author, &CRect(infoX, infoY, infoX + infoW, infoY + lineH), DT_LEFT | DT_TOP | DT_END_ELLIPSIS);

    CString yearStr; yearStr.Format(_T("Рік: %d"), book.year);
    pDC->DrawText(yearStr, &CRect(infoX, infoY + lineH, infoX + infoW, infoY + lineH * 2), DT_LEFT | DT_TOP);

    CString category; category.Format(_T("Категорія: %s"), book.category.GetString());
    pDC->DrawText(category, &CRect(infoX, infoY + lineH * 2, infoX + infoW, infoY + lineH * 3), DT_LEFT | DT_TOP);

    CString rating; rating.Format(_T("Рейтинг: %.1f / 5.0"), book.rating);
    pDC->DrawText(rating, &CRect(infoX, infoY + lineH * 3, infoX + infoW, infoY + lineH * 4), DT_LEFT | DT_TOP);

    CString available; available.Format(_T("Доступно: %d / %d"), book.quantityAvailable, book.quantityTotal);
    pDC->DrawText(available, &CRect(infoX, infoY + lineH * 4, infoX + infoW, infoY + lineH * 5), DT_LEFT | DT_TOP);

    CRect scrollRect(card.left + 12, coverY + coverH + 12, card.right - 12, card.bottom - 42);
    pDC->FillSolidRect(&scrollRect, RGB(250, 252, 255));
    pDC->Draw3dRect(&scrollRect, RGB(226, 232, 238), RGB(226, 232, 238));

    CRect measureDesc(scrollRect.left + 2, scrollRect.top + 2, scrollRect.right - 2, scrollRect.top + 2);
    pDC->SelectObject(&m_fontNormal);
    measureDesc.bottom = measureDesc.top + 1;
    pDC->DrawText(book.description, &measureDesc, DT_LEFT | DT_WORDBREAK | DT_CALCRECT);
    int descHeight = max(40, measureDesc.Height());

    auto reviews = m_db.GetReviewsByBook(m_detailsBookId);
    int topPadding = 8;
    int bottomPadding = 8;
    int sectionTitleHeight = 22;
    int descriptionBlockHeight = descHeight + 16;
    int reviewsTitleHeight = 24;
    int reviewsBlockHeight = reviews.empty() ? 22 : ((int)reviews.size() * 44);
    int contentHeight = topPadding + sectionTitleHeight + descriptionBlockHeight + reviewsTitleHeight + reviewsBlockHeight + bottomPadding;
    int minScroll = min(0, scrollRect.Height() - contentHeight);
    if (m_detailsScrollY > 0.0f) m_detailsScrollY = 0.0f;
    if (m_detailsScrollY < (float)minScroll) m_detailsScrollY = (float)minScroll;

    int saved = pDC->SaveDC();
    pDC->IntersectClipRect(&scrollRect);

    int y = scrollRect.top + (int)m_detailsScrollY + 8;
    pDC->SetTextColor(RGB(0, 100, 190));
    pDC->SelectObject(&m_fontBold);
    pDC->DrawText(_T("Опис"), &CRect(scrollRect.left + 8, y, scrollRect.right - 8, y + 20), DT_LEFT | DT_TOP | DT_SINGLELINE);
    y += 22;

    pDC->SetTextColor(RGB(70, 76, 84));
    pDC->SelectObject(&m_fontNormal);
    CRect descTextRect(scrollRect.left + 8, y, scrollRect.right - 8, y + descHeight + 8);
    pDC->DrawText(book.description, &descTextRect, DT_LEFT | DT_TOP | DT_WORDBREAK);
    y = descTextRect.bottom + 8;

    pDC->SetTextColor(RGB(0, 100, 190));
    pDC->SelectObject(&m_fontBold);
    pDC->DrawText(_T("Відгуки"), &CRect(scrollRect.left + 8, y, scrollRect.right - 8, y + 20), DT_LEFT | DT_TOP | DT_SINGLELINE);
    y += 24;

    pDC->SetTextColor(RGB(80, 86, 94));
    pDC->SelectObject(&m_fontNormal);
    if (reviews.empty())
    {
        pDC->DrawText(_T("Поки що немає відгуків."), &CRect(scrollRect.left + 8, y, scrollRect.right - 8, y + 22), DT_LEFT | DT_TOP | DT_SINGLELINE);
    }
    else
    {
        for (int i = 0; i < (int)reviews.size(); ++i)
        {
            CString userName = m_db.GetUserDisplayName(reviews[i].userId);
            CString head;
            head.Format(_T("★ %d  %s  (%s)"), reviews[i].rating, userName.GetString(), reviews[i].createdAt.GetString());
            pDC->SetTextColor(RGB(45, 54, 66));
            pDC->DrawText(head, &CRect(scrollRect.left + 8, y, scrollRect.right - 8, y + 18), DT_LEFT | DT_TOP | DT_END_ELLIPSIS | DT_SINGLELINE);
            y += 18;
            pDC->SetTextColor(RGB(85, 92, 100));
            CRect cRect(scrollRect.left + 12, y, scrollRect.right - 12, y + 22);
            CString comment = reviews[i].comment;
            if (comment.GetLength() > 180) comment = comment.Left(177) + _T("...");
            pDC->DrawText(comment, &cRect, DT_LEFT | DT_TOP | DT_WORDBREAK);
            y += 26;
        }
    }

    pDC->RestoreDC(saved);

    CRect hintRect(rc.left, rc.bottom - 20, rc.right, rc.bottom - 4);
    pDC->SetTextColor(RGB(150, 150, 150));
    pDC->SelectObject(&m_fontNormal);
    pDC->DrawText(_T("ESC → каталог | Колесо миші: прокрутка"), &hintRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    pDC->SelectObject(old);
}

void CLibraryAppView::Draw3DInspectScene(bool bPicking) {
    if (m_selectedIndex < 0 || m_selectedIndex >= (int)m_books.size()) return;

    CRect rc;
    GetClientRect(&rc);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, rc.Height() ? (GLfloat)rc.Width() / (GLfloat)rc.Height() : 1.0f, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    const double cameraDistance = max(2.0f, -m_inspectDist + 1.5f);
    gluLookAt(0.0, 0.0, cameraDistance, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    GLfloat lightPos[] = { 3.0f, 3.0f, 3.0f, 1.0f };
    GLfloat lightDif[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDif);

    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    BookData& b = m_books[m_selectedIndex];

    if (b.texIndex >= 0 && b.texIndex < (int)m_texCovers.size() && m_texCovers[b.texIndex] == 0)
    {
        COLORREF avg = RGB(110, 128, 150);
        GLuint tex = LoadTextureFromFile(b.coverPath, &avg);
        if (tex != 0)
        {
            m_texCovers[b.texIndex] = tex;
            if (b.texIndex < (int)m_coverAvgColors.size())
            {
                m_coverAvgColors[b.texIndex] = avg;
            }
        }
    }

    if (bPicking) glPushName(0);
    glPushMatrix();
    glRotatef(m_inspectRotX, 1, 0, 0);
    glRotatef(m_inspectRotY, 0, 1, 0);
    DrawBookModel(2.8f, 3.8f, 0.7f, b.texIndex, b.r, b.g, b.b);
    glPopMatrix();
    if (bPicking) glPopName();
}

#ifdef _DEBUG
CLibraryAppDoc* CLibraryAppView::GetDocument() const {
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CLibraryAppDoc)));
    return (CLibraryAppDoc*)m_pDocument;
}
#endif
