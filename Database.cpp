#include "pch.h"
#include "Database.h"

#include <algorithm>
#include <afx.h>
#include <cwctype>
#include <comdef.h>

namespace
{

CString EscapeSql(const CString& value)
{
    CString escaped(value);
    escaped.Replace(_T("'"), _T("''"));
    return escaped;
}

CString Quote(const CString& value)
{
    return _T("'") + EscapeSql(value) + _T("'");
}

CString AppDirectory()
{
    TCHAR buf[MAX_PATH] = { 0 };
    ::GetModuleFileName(nullptr, buf, MAX_PATH);
    CString path(buf);
    int slash = path.ReverseFind(_T('\\'));
    if (slash > 0) path = path.Left(slash);
    return path;
}

bool FileExists(const CString& path)
{
    DWORD attr = ::GetFileAttributes(path);
    return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

CString ToLowerText(const CString& value)
{
    CString out(value);
    out.MakeLower();
    return out;
}

bool ResolveReadablePath(const CString& sourcePath, CString& outAbsolutePath)
{
    outAbsolutePath.Empty();
    CString p(sourcePath);
    p.Trim();
    if (p.IsEmpty()) return false;

    if (FileExists(p))
    {
        outAbsolutePath = p;
        return true;
    }

    CString normalized = p;
    normalized.Replace(_T("/"), _T("\\"));

    CString appDir = AppDirectory();
    CString candidates[] = {
        appDir + _T("\\") + normalized,
        appDir + _T("\\..\\") + normalized,
        appDir + _T("\\..\\..\\") + normalized,
        appDir + _T("\\..\\..\\..\\") + normalized
    };

    for (const CString& c : candidates)
    {
        if (FileExists(c))
        {
            outAbsolutePath = c;
            return true;
        }
    }

    int slash = normalized.ReverseFind(_T('\\'));
    CString fileName = (slash >= 0) ? normalized.Mid(slash + 1) : normalized;
    if (!fileName.IsEmpty())
    {
        CString booksCandidates[] = {
            appDir + _T("\\books\\") + fileName,
            appDir + _T("\\..\\books\\") + fileName,
            appDir + _T("\\..\\..\\books\\") + fileName,
            appDir + _T("\\..\\..\\..\\books\\") + fileName
        };
        for (const CString& c : booksCandidates)
        {
            if (FileExists(c))
            {
                outAbsolutePath = c;
                return true;
            }
        }
    }

    return false;
}

bool CopyCoverToBooksFolder(const CString& sourcePath, int bookId, CString& outRelativePath, CString& outError)
{
    outRelativePath.Empty();
    outError.Empty();

    CString absSource;
    if (!ResolveReadablePath(sourcePath, absSource))
    {
        outError = _T("Файл обкладинки не знайдено.");
        return false;
    }

    CString ext = _T(".jpg");
    int dot = absSource.ReverseFind(_T('.'));
    if (dot >= 0)
    {
        CString candidate = ToLowerText(absSource.Mid(dot));
        if (candidate == _T(".jpg") || candidate == _T(".jpeg") || candidate == _T(".png") || candidate == _T(".bmp") || candidate == _T(".webp"))
        {
            ext = candidate;
        }
    }

    CString booksDir = AppDirectory() + _T("\\books");
    ::CreateDirectory(booksDir, nullptr);

    CString destAbs;
    destAbs.Format(_T("%s\\%d%s"), booksDir.GetString(), bookId, ext.GetString());
    if (!::CopyFile(absSource, destAbs, FALSE))
    {
        outError.Format(_T("Не вдалося скопіювати обкладинку. Код: %lu"), ::GetLastError());
        return false;
    }

    outRelativePath.Format(_T("books/%d%s"), bookId, ext.GetString());
    return true;
}

bool IsUrl(const CString& path)
{
    CString lower = ToLowerText(path);
    return lower.Left(7) == _T("http://") || lower.Left(8) == _T("https://");
}

bool DownloadFileFromUrl(const CString& url, const CString& destPath, CString& outError)
{
    outError.Empty();
    try
    {
        CInternetSession session(_T("LibraryApp/1.0"));
        session.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 10000);
        session.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, 15000);

        CStdioFile* pFile = session.OpenURL(url, 1, INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_RELOAD);
        if (!pFile)
        {
            outError = _T("Не вдалося відкрити URL.");
            return false;
        }

        CFile localFile;
        if (!localFile.Open(destPath, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary))
        {
            pFile->Close();
            delete pFile;
            outError = _T("Не вдалося створити локальний файл.");
            return false;
        }

        BYTE buf[8192];
        UINT bytesRead;
        while ((bytesRead = pFile->Read(buf, sizeof(buf))) > 0)
        {
            localFile.Write(buf, bytesRead);
        }

        localFile.Close();
        pFile->Close();
        delete pFile;
        session.Close();
        return true;
    }
    catch (CInternetException* e)
    {
        TCHAR szErr[512];
        e->GetErrorMessage(szErr, 512);
        outError = szErr;
        e->Delete();
        return false;
    }
    catch (CFileException* e)
    {
        outError = _T("Помилка запису файлу.");
        e->Delete();
        return false;
    }
}

CString AdoFieldStr(_RecordsetPtr& rs, const _variant_t& field)
{
    try
    {
        _variant_t val = rs->Fields->GetItem(field)->Value;
        if (val.vt == VT_NULL || val.vt == VT_EMPTY) return _T("");
        return CString((LPCTSTR)_bstr_t(val));
    }
    catch (...) { return _T(""); }
}

int AdoFieldInt(_RecordsetPtr& rs, const _variant_t& field)
{
    try
    {
        _variant_t val = rs->Fields->GetItem(field)->Value;
        if (val.vt == VT_NULL || val.vt == VT_EMPTY) return 0;
        return (int)val;
    }
    catch (...) { return 0; }
}

float AdoFieldFloat(_RecordsetPtr& rs, const _variant_t& field)
{
    try
    {
        _variant_t val = rs->Fields->GetItem(field)->Value;
        if (val.vt == VT_NULL || val.vt == VT_EMPTY) return 0.0f;
        return (float)(double)val;
    }
    catch (...) { return 0.0f; }
}

bool QueryScalar(_ConnectionPtr& conn, const CString& sql, CString& outValue, CString* outError = nullptr)
{
    outValue.Empty();
    try
    {
        _RecordsetPtr rs;
        rs.CreateInstance(__uuidof(Recordset));
        rs->Open(_bstr_t(sql), _variant_t((IDispatch*)conn), adOpenForwardOnly, adLockReadOnly, adCmdText);
        if (rs->ADO_EOF)
        {
            rs->Close();
            return false;
        }
        _variant_t val = rs->Fields->GetItem((short)0)->Value;
        if (val.vt != VT_NULL && val.vt != VT_EMPTY)
            outValue = CString((LPCTSTR)_bstr_t(val));
        rs->Close();
        return true;
    }
    catch (_com_error& e)
    {
        if (outError) *outError = CString(e.ErrorMessage());
        return false;
    }
}

CString Trimmed(const CString& value)
{
    CString out(value);
    out.Trim();
    return out;
}

bool IsPrintableText(const CString& value)
{
    for (int i = 0; i < value.GetLength(); ++i)
    {
        WCHAR ch = value[i];
        if (ch < 32 && ch != _T('\n') && ch != _T('\r') && ch != _T('\t'))
            return false;
    }
    return true;
}

bool IsValidLogin(const CString& login)
{
    if (login.GetLength() < 3 || login.GetLength() > 64) return false;
    for (int i = 0; i < login.GetLength(); ++i)
    {
        WCHAR ch = login[i];
        bool ok = (ch >= _T('a') && ch <= _T('z')) ||
                  (ch >= _T('A') && ch <= _T('Z')) ||
                  (ch >= _T('0') && ch <= _T('9')) ||
                  ch == _T('_') || ch == _T('-') || ch == _T('.');
        if (!ok) return false;
    }
    return true;
}

bool IsValidEmail(const CString& email)
{
    int at = email.Find(_T('@'));
    if (at <= 0 || at != email.ReverseFind(_T('@'))) return false;
    int dot = email.ReverseFind(_T('.'));
    if (dot <= at + 1 || dot >= email.GetLength() - 1) return false;
    if (email.Find(_T(' ')) >= 0) return false;
    return true;
}

bool IsValidPhone(const CString& phone)
{
    if (phone.IsEmpty()) return true;
    if (phone.GetLength() > 32) return false;

    int digits = 0;
    for (int i = 0; i < phone.GetLength(); ++i)
    {
        WCHAR ch = phone[i];
        if (ch >= _T('0') && ch <= _T('9')) { digits++; continue; }
        if (ch == _T('+') || ch == _T(' ') || ch == _T('-') || ch == _T('(') || ch == _T(')')) continue;
        return false;
    }
    return digits >= 7;
}

bool IsReasonableYear(int year)
{
    CTime now = CTime::GetCurrentTime();
    int maxYear = now.GetYear() + 1;
    return year >= 1450 && year <= maxYear;
}

bool IsAllowedReservationStatus(const CString& status)
{
    CString s = ToLowerText(Trimmed(status));
    return s == _T("reserved") ||
           s == _T("ready_for_pickup") ||
           s == _T("issued") ||
           s == _T("overdue") ||
           s == _T("return_requested") ||
           s == _T("returned") ||
           s == _T("lost") ||
           s == _T("damaged") ||
           s == _T("cancelled") ||
           s == _T("expired");
}

bool IsInventoryActiveStatus(const CString& status)
{
    CString s = ToLowerText(Trimmed(status));
    return s == _T("reserved") ||
           s == _T("ready_for_pickup") ||
           s == _T("issued") ||
           s == _T("overdue") ||
           s == _T("return_requested");
}

bool IsInventoryReturnStatus(const CString& status)
{
    CString s = ToLowerText(Trimmed(status));
    return s == _T("returned") ||
           s == _T("cancelled") ||
           s == _T("expired");
}

CString ReservationStatusUiLabel(const CString& status)
{
    CString s = ToLowerText(Trimmed(status));
    if (s == _T("reserved")) return _T("Зарезервовано");
    if (s == _T("ready_for_pickup")) return _T("Готово до видачі");
    if (s == _T("issued")) return _T("Видано");
    if (s == _T("overdue")) return _T("Прострочено");
    if (s == _T("return_requested")) return _T("Запит на повернення");
    if (s == _T("returned")) return _T("Повернуто");
    if (s == _T("lost")) return _T("Втрачено");
    if (s == _T("damaged")) return _T("Пошкоджено");
    if (s == _T("cancelled")) return _T("Скасовано");
    if (s == _T("expired")) return _T("Термін минув");
    return status;
}

}

Database& Database::Instance()
{
    static Database db;
    return db;
}

Database::Database()
{
    m_conn = nullptr;
    Connect();
}

bool Database::Connect()
{
    try
    {
        if (m_conn != nullptr && (m_conn->State & adStateOpen))
            return true;
    }
    catch (...) { m_conn = nullptr; }

    m_lastDbError.Empty();
    m_connectedDbName.Empty();

    const CString dbName = _T("Library");

    CString conn;
    conn += _T("Provider=MSDASQL;");
    conn += _T("DRIVER={MySQL ODBC 8.0 Unicode Driver};");
    conn += _T("SERVER=127.0.0.1;");
    conn += _T("PORT=3306;");
    conn += _T("DATABASE=") + dbName + _T(";");
    conn += _T("UID=root;");
    conn += _T("PWD=;");
    conn += _T("OPTION=3;");

    try
    {
        m_conn.CreateInstance(__uuidof(Connection));
        m_conn->CursorLocation = adUseClient;
        m_conn->Open(_bstr_t(conn), _bstr_t(L""), _bstr_t(L""), adConnectUnspecified);
        m_connectedDbName = dbName;
        m_lastDbError.Empty();
        return true;
    }
    catch (_com_error& e)
    {
        m_lastDbError = CString(e.ErrorMessage());
        try { if (m_conn != nullptr && (m_conn->State & adStateOpen)) m_conn->Close(); } catch (...) {}
        m_conn = nullptr;
    }

    return false;
}

void Database::Disconnect()
{
    try
    {
        if (m_conn != nullptr && (m_conn->State & adStateOpen))
            m_conn->Close();
    }
    catch (...) {}
    m_conn = nullptr;
}

bool Database::IsConnected() const
{
    try
    {
        return m_conn != nullptr && (m_conn->State & adStateOpen);
    }
    catch (...) { return false; }
}

bool Database::Login(const CString& login, const CString& password, CString& errorText)
{
    errorText.Empty();

    CString loginTrim = Trimmed(login);
    CString passwordTrim = Trimmed(password);
    if (loginTrim.IsEmpty() || passwordTrim.IsEmpty())
    {
        errorText = _T("Введіть логін і пароль.");
        return false;
    }

    if (!Connect())
    {
        errorText = _T("Немає підключення до бази даних: ") + m_lastDbError;
        return false;
    }

    try
    {
        CString sql;
        sql.Format(
            _T("SELECT id,login,role,full_name,phone,email FROM users WHERE login=%s AND password=%s LIMIT 1"),
            Quote(loginTrim).GetString(),
            Quote(passwordTrim).GetString());

        _RecordsetPtr rs;
        rs.CreateInstance(__uuidof(Recordset));
        rs->Open(_bstr_t(sql), _variant_t((IDispatch*)m_conn), adOpenForwardOnly, adLockReadOnly, adCmdText);
        if (rs->ADO_EOF)
        {
            rs->Close();
            errorText = _T("Невірний логін або пароль.");
            return false;
        }

        m_currentUserId = AdoFieldInt(rs, _T("id"));
        m_currentLogin = AdoFieldStr(rs, _T("login"));
        m_currentRole = AdoFieldStr(rs, _T("role"));
        m_currentFullName = AdoFieldStr(rs, _T("full_name"));
        m_currentPhone = AdoFieldStr(rs, _T("phone"));
        m_currentEmail = AdoFieldStr(rs, _T("email"));
        rs->Close();

        return m_currentUserId > 0;
    }
    catch (_com_error& e)
    {
        errorText = CString(e.ErrorMessage());
        return false;
    }
}

bool Database::RegisterUser(const CString& login, const CString& password, const CString& email, const CString& phone, const CString& fullName, CString& errorText)
{
    errorText.Empty();

    CString loginTrim = Trimmed(login);
    CString passwordTrim = Trimmed(password);
    CString emailTrim = Trimmed(email);
    CString phoneTrim = Trimmed(phone);
    CString fullNameTrim = Trimmed(fullName);

    if (loginTrim.IsEmpty() || passwordTrim.IsEmpty() || emailTrim.IsEmpty() || fullNameTrim.IsEmpty())
    {
        errorText = _T("Заповніть усі обов'язкові поля.");
        return false;
    }

    if (!IsValidLogin(loginTrim))
    {
        errorText = _T("Логін: 3-64 символи, тільки латиниця, цифри, _, -, .");
        return false;
    }

    if (passwordTrim.GetLength() < 6 || passwordTrim.GetLength() > 255)
    {
        errorText = _T("Пароль має містити від 6 до 255 символів.");
        return false;
    }

    if (!IsValidEmail(emailTrim))
    {
        errorText = _T("Некоректний формат email.");
        return false;
    }

    if (fullNameTrim.GetLength() < 3 || fullNameTrim.GetLength() > 150 || !IsPrintableText(fullNameTrim))
    {
        errorText = _T("ПІБ має містити від 3 до 150 символів.");
        return false;
    }

    if (!IsValidPhone(phoneTrim))
    {
        errorText = _T("Некоректний формат телефону.");
        return false;
    }

    if (!Connect())
    {
        errorText = _T("Немає підключення до бази даних: ") + m_lastDbError;
        return false;
    }

    try
    {
        CString phoneSql = phoneTrim.IsEmpty() ? _T("NULL") : Quote(phoneTrim);

        CString sql;
        sql.Format(
            _T("INSERT INTO users(login,password,role,full_name,phone,email) VALUES(%s,%s,'user',%s,%s,%s)"),
            Quote(loginTrim).GetString(),
            Quote(passwordTrim).GetString(),
            Quote(fullNameTrim).GetString(),
            phoneSql.GetString(),
            Quote(emailTrim).GetString());

        m_conn->Execute(_bstr_t(sql), nullptr, adCmdText | adExecuteNoRecords);

        CString idText;
        if (!QueryScalar(m_conn, _T("SELECT LAST_INSERT_ID()"), idText, &errorText))
        {
            errorText = _T("Користувача створено, але не вдалося отримати ID.");
            return false;
        }

        m_currentUserId = _ttoi(idText);
        m_currentLogin = loginTrim;
        m_currentRole = _T("user");
        m_currentFullName = fullNameTrim;
        m_currentPhone = phoneTrim;
        m_currentEmail = emailTrim;
        return m_currentUserId > 0;
    }
    catch (_com_error& e)
    {
        CString dbError = CString(e.ErrorMessage());

        if (dbError.Find(_T("uq_users_login")) >= 0 || dbError.Find(_T("login")) >= 0)
            errorText = _T("Такий логін вже існує.");
        else if (dbError.Find(_T("uq_users_email")) >= 0 || dbError.Find(_T("email")) >= 0)
            errorText = _T("Такий email вже існує.");
        else
            errorText = dbError;
        return false;
    }
}

void Database::Logout()
{
    m_currentUserId = 0;
    m_currentLogin.Empty();
    m_currentRole.Empty();
    m_currentFullName.Empty();
    m_currentPhone.Empty();
    m_currentEmail.Empty();
}

bool Database::IsLoggedIn() const { return m_currentUserId > 0; }
bool Database::IsAdmin() const { return m_currentRole.CompareNoCase(_T("admin")) == 0; }
int Database::GetCurrentUserId() const { return m_currentUserId; }
CString Database::GetCurrentRole() const { return m_currentRole; }
CString Database::GetCurrentLogin() const { return m_currentLogin; }
CString Database::GetCurrentFullName() const { return m_currentFullName; }
CString Database::GetCurrentEmail() const { return m_currentEmail; }
CString Database::GetCurrentPhone() const { return m_currentPhone; }

int Database::GetBooksCount() const
{
    if (!const_cast<Database*>(this)->Connect()) return 0;

    CString value;
    if (!QueryScalar(const_cast<Database*>(this)->m_conn, _T("SELECT COUNT(*) FROM books"), value))
        return 0;
    return _ttoi(value);
}

std::vector<BookRecord> Database::GetBooksPage(int page, int pageSize) const
{
    std::vector<BookRecord> result;
    if (page < 0 || pageSize <= 0) return result;
    if (!const_cast<Database*>(this)->Connect()) return result;

    try
    {
        CString sql;
        sql.Format(
            _T("SELECT b.id,b.category_id,b.title,b.author,b.year,c.name AS category,b.rating,b.description,b.cover_path,b.quantity_total,b.quantity_available ")
            _T("FROM books b JOIN categories c ON c.id=b.category_id ")
            _T("ORDER BY b.id DESC LIMIT %d OFFSET %d"),
            pageSize, page * pageSize);

        _RecordsetPtr rs;
        rs.CreateInstance(__uuidof(Recordset));
        rs->Open(_bstr_t(sql), _variant_t((IDispatch*)m_conn), adOpenForwardOnly, adLockReadOnly, adCmdText);

        while (!rs->ADO_EOF)
        {
            BookRecord b;
            b.id = AdoFieldInt(rs, _T("id"));
            b.categoryId = AdoFieldInt(rs, _T("category_id"));
            b.title = AdoFieldStr(rs, _T("title"));
            b.author = AdoFieldStr(rs, _T("author"));
            b.year = AdoFieldInt(rs, _T("year"));
            b.category = AdoFieldStr(rs, _T("category"));
            b.rating = AdoFieldFloat(rs, _T("rating"));
            b.description = AdoFieldStr(rs, _T("description"));
            b.coverPath = AdoFieldStr(rs, _T("cover_path"));
            b.quantityTotal = AdoFieldInt(rs, _T("quantity_total"));
            b.quantityAvailable = AdoFieldInt(rs, _T("quantity_available"));
            if (b.description.IsEmpty())
            {
                b.description = _T("Опис відсутній.");
            }
            result.push_back(b);
            rs->MoveNext();
        }
        rs->Close();
    }
    catch (_com_error&) {}

    return result;
}

std::vector<BookRecord> Database::GetAllBooks() const
{
    std::vector<BookRecord> result;
    if (!const_cast<Database*>(this)->Connect()) return result;

    try
    {
        CString sql =
            _T("SELECT b.id,b.category_id,b.title,b.author,b.year,c.name AS category,b.rating,b.description,b.cover_path,b.quantity_total,b.quantity_available ")
            _T("FROM books b JOIN categories c ON c.id=b.category_id ORDER BY b.title");

        _RecordsetPtr rs;
        rs.CreateInstance(__uuidof(Recordset));
        rs->Open(_bstr_t(sql), _variant_t((IDispatch*)m_conn), adOpenForwardOnly, adLockReadOnly, adCmdText);

        while (!rs->ADO_EOF)
        {
            BookRecord b;
            b.id = AdoFieldInt(rs, _T("id"));
            b.categoryId = AdoFieldInt(rs, _T("category_id"));
            b.title = AdoFieldStr(rs, _T("title"));
            b.author = AdoFieldStr(rs, _T("author"));
            b.year = AdoFieldInt(rs, _T("year"));
            b.category = AdoFieldStr(rs, _T("category"));
            b.rating = AdoFieldFloat(rs, _T("rating"));
            b.description = AdoFieldStr(rs, _T("description"));
            b.coverPath = AdoFieldStr(rs, _T("cover_path"));
            b.quantityTotal = AdoFieldInt(rs, _T("quantity_total"));
            b.quantityAvailable = AdoFieldInt(rs, _T("quantity_available"));
            if (b.description.IsEmpty())
            {
                b.description = _T("Опис відсутній.");
            }
            result.push_back(b);
            rs->MoveNext();
        }
        rs->Close();
    }
    catch (_com_error&) {}

    return result;
}

bool Database::GetBookById(int bookId, BookRecord& outBook) const
{
    if (!const_cast<Database*>(this)->Connect()) return false;

    try
    {
        CString sql;
        sql.Format(
            _T("SELECT b.id,b.category_id,b.title,b.author,b.year,c.name AS category,b.rating,b.description,b.cover_path,b.quantity_total,b.quantity_available ")
            _T("FROM books b JOIN categories c ON c.id=b.category_id WHERE b.id=%d LIMIT 1"),
            bookId);

        _RecordsetPtr rs;
        rs.CreateInstance(__uuidof(Recordset));
        rs->Open(_bstr_t(sql), _variant_t((IDispatch*)m_conn), adOpenForwardOnly, adLockReadOnly, adCmdText);
        if (rs->ADO_EOF) { rs->Close(); return false; }

        outBook.id = AdoFieldInt(rs, _T("id"));
        outBook.categoryId = AdoFieldInt(rs, _T("category_id"));
        outBook.title = AdoFieldStr(rs, _T("title"));
        outBook.author = AdoFieldStr(rs, _T("author"));
        outBook.year = AdoFieldInt(rs, _T("year"));
        outBook.category = AdoFieldStr(rs, _T("category"));
        outBook.rating = AdoFieldFloat(rs, _T("rating"));
        outBook.description = AdoFieldStr(rs, _T("description"));
        outBook.coverPath = AdoFieldStr(rs, _T("cover_path"));
        outBook.quantityTotal = AdoFieldInt(rs, _T("quantity_total"));
        outBook.quantityAvailable = AdoFieldInt(rs, _T("quantity_available"));
        if (outBook.description.IsEmpty())
        {
            outBook.description = _T("Опис відсутній.");
        }
        rs->Close();
        return true;
    }
    catch (_com_error&) { return false; }
}

std::vector<CategoryRecord> Database::GetAllCategories() const
{
    std::vector<CategoryRecord> result;
    if (!const_cast<Database*>(this)->Connect()) return result;

    try
    {
        CString sql = _T("SELECT c.id, c.name, COUNT(b.id) AS book_count FROM categories c ")
                      _T("LEFT JOIN books b ON b.category_id = c.id ")
                      _T("GROUP BY c.id, c.name ORDER BY c.name");

        _RecordsetPtr rs;
        rs.CreateInstance(__uuidof(Recordset));
        rs->Open(_bstr_t(sql), _variant_t((IDispatch*)m_conn), adOpenForwardOnly, adLockReadOnly, adCmdText);

        while (!rs->ADO_EOF)
        {
            CategoryRecord cat;
            cat.id = AdoFieldInt(rs, _T("id"));
            cat.name = AdoFieldStr(rs, _T("name"));
            cat.bookCount = AdoFieldInt(rs, _T("book_count"));
            result.push_back(cat);
            rs->MoveNext();
        }
        rs->Close();
    }
    catch (_com_error&) {}

    return result;
}

bool Database::AddCategory(const CString& name, CString& errorText)
{
    errorText.Empty();
    CString trimmedName = name;
    trimmedName.Trim();

    if (trimmedName.IsEmpty())
    {
        errorText = _T("Назва категорії не може бути порожньою.");
        return false;
    }

    if (trimmedName.GetLength() < 2 || trimmedName.GetLength() > 100 || !IsPrintableText(trimmedName))
    {
        errorText = _T("Назва категорії має містити від 2 до 100 символів.");
        return false;
    }

    if (!IsAdmin())
    {
        errorText = _T("Тільки адміністратор може додавати категорії.");
        return false;
    }

    if (!Connect())
    {
        errorText = _T("Немає підключення до БД.");
        return false;
    }

    if (CategoryExists(trimmedName))
    {
        errorText = _T("Категорія з такою назвою вже існує.");
        return false;
    }

    try
    {
        CString sql;
        sql.Format(_T("INSERT INTO categories(name) VALUES(%s)"), Quote(trimmedName).GetString());
        m_conn->Execute(_bstr_t(sql), nullptr, adCmdText | adExecuteNoRecords);
        return true;
    }
    catch (_com_error& e)
    {
        errorText = CString(e.ErrorMessage());
        return false;
    }
}

bool Database::EditCategory(int categoryId, const CString& name, CString& errorText)
{
    errorText.Empty();
    CString trimmedName = name;
    trimmedName.Trim();

    if (trimmedName.IsEmpty())
    {
        errorText = _T("Назва категорії не може бути порожньою.");
        return false;
    }

    if (categoryId <= 0)
    {
        errorText = _T("Некоректний ID категорії.");
        return false;
    }

    if (trimmedName.GetLength() < 2 || trimmedName.GetLength() > 100 || !IsPrintableText(trimmedName))
    {
        errorText = _T("Назва категорії має містити від 2 до 100 символів.");
        return false;
    }

    if (!IsAdmin())
    {
        errorText = _T("Тільки адміністратор може редагувати категорії.");
        return false;
    }

    if (!Connect())
    {
        errorText = _T("Немає підключення до БД.");
        return false;
    }

    try
    {
        CString sql;
        sql.Format(_T("UPDATE categories SET name=%s WHERE id=%d"), Quote(trimmedName).GetString(), categoryId);
        m_conn->Execute(_bstr_t(sql), nullptr, adCmdText | adExecuteNoRecords);
        return true;
    }
    catch (_com_error& e)
    {
        errorText = CString(e.ErrorMessage());
        return false;
    }
}

bool Database::DeleteCategory(int categoryId, CString& errorText)
{
    errorText.Empty();

    if (!IsAdmin())
    {
        errorText = _T("Тільки адміністратор може видаляти категорії.");
        return false;
    }

    if (!Connect())
    {
        errorText = _T("Немає підключення до БД.");
        return false;
    }

    try
    {
        CString checkSql;
        checkSql.Format(_T("SELECT COUNT(*) FROM books WHERE category_id=%d"), categoryId);
        CString countStr;
        if (QueryScalar(m_conn, checkSql, countStr, nullptr) && _ttoi(countStr) > 0)
        {
            errorText = _T("Неможливо видалити категорію: в ній є книги.");
            return false;
        }

        CString sql;
        sql.Format(_T("DELETE FROM categories WHERE id=%d"), categoryId);
        m_conn->Execute(_bstr_t(sql), nullptr, adCmdText | adExecuteNoRecords);
        return true;
    }
    catch (_com_error& e)
    {
        errorText = CString(e.ErrorMessage());
        return false;
    }
}

bool Database::CategoryExists(const CString& name) const
{
    if (!const_cast<Database*>(this)->Connect()) return false;

    CString sql;
    sql.Format(_T("SELECT COUNT(*) FROM categories WHERE name=%s"), Quote(name).GetString());
    CString countStr;
    if (QueryScalar(const_cast<Database*>(this)->m_conn, sql, countStr, nullptr))
        return _ttoi(countStr) > 0;
    return false;
}

int Database::GetCategoryIdByName(const CString& name) const
{
    if (!const_cast<Database*>(this)->Connect()) return 0;

    CString sql;
    sql.Format(_T("SELECT id FROM categories WHERE name=%s LIMIT 1"), Quote(name).GetString());
    CString idStr;
    if (QueryScalar(const_cast<Database*>(this)->m_conn, sql, idStr, nullptr))
        return _ttoi(idStr);
    return 0;
}

bool Database::ReserveBook(int bookId, int days, const CString& branchName, CString& messageText)
{
    messageText.Empty();

    if (!IsLoggedIn())
    {
        messageText = _T("Спочатку увійдіть.");
        return false;
    }

    if (days <= 0 || days > 30)
    {
        messageText = _T("Кількість днів має бути від 1 до 30.");
        return false;
    }

    CString branchNameTrim = Trimmed(branchName);
    if (bookId <= 0)
    {
        messageText = _T("Некоректний ID книги.");
        return false;
    }
    if (branchNameTrim.IsEmpty())
    {
        messageText = _T("Оберіть філію.");
        return false;
    }

    if (!Connect())
    {
        messageText = _T("Немає підключення до бази даних: ") + m_lastDbError;
        return false;
    }

    CString branchIdSql;
    branchIdSql.Format(_T("SELECT id FROM branches WHERE name=%s LIMIT 1"), Quote(branchNameTrim).GetString());
    CString branchIdStr;
    if (!QueryScalar(m_conn, branchIdSql, branchIdStr, &messageText))
    {
        messageText = _T("Філію не знайдено. Спочатку створіть філію.");
        return false;
    }
    int branchId = _ttoi(branchIdStr);

    try
    {
        m_conn->BeginTrans();

        CString qtySql;
        qtySql.Format(_T("SELECT quantity_available FROM books WHERE id=%d FOR UPDATE"), bookId);
        CString qtyText;
        if (!QueryScalar(m_conn, qtySql, qtyText, &messageText))
        {
            m_conn->RollbackTrans();
            messageText = _T("Книгу не знайдено.");
            return false;
        }

        int available = _ttoi(qtyText);
        if (available <= 0)
        {
            m_conn->RollbackTrans();
            messageText = _T("Книга зараз недоступна.");
            return false;
        }

        CString updateSql;
        updateSql.Format(_T("UPDATE books SET quantity_available=quantity_available-1 WHERE id=%d"), bookId);
        m_conn->Execute(_bstr_t(updateSql), nullptr, adCmdText | adExecuteNoRecords);

        CString insertSql;
        insertSql.Format(
            _T("INSERT INTO reservations(user_id,book_id,branch_id,reservation_date,return_date,status) ")
            _T("VALUES(%d,%d,%d,NOW(),DATE_ADD(NOW(), INTERVAL %d DAY),'reserved')"),
            m_currentUserId, bookId, branchId, days);
        m_conn->Execute(_bstr_t(insertSql), nullptr, adCmdText | adExecuteNoRecords);

        m_conn->CommitTrans();
        messageText.Format(_T("Бронювання успішне!\nФілія: %s\nТермін: %d днів"), branchNameTrim.GetString(), days);
        return true;
    }
    catch (_com_error& e)
    {
        try { m_conn->RollbackTrans(); } catch (...) {}
        messageText = CString(e.ErrorMessage());
        return false;
    }
}

bool Database::AddReview(int bookId, int rating, const CString& comment, CString& messageText)
{
    messageText.Empty();

    CString commentTrim = Trimmed(comment);

    if (!IsLoggedIn())
    {
        messageText = _T("Спочатку увійдіть.");
        return false;
    }

    if (rating < 1 || rating > 5)
    {
        messageText = _T("Оцінка має бути від 1 до 5.");
        return false;
    }

    if (bookId <= 0)
    {
        messageText = _T("Некоректний ID книги.");
        return false;
    }

    if (commentTrim.GetLength() < 2 || commentTrim.GetLength() > 5000 || !IsPrintableText(commentTrim))
    {
        messageText = _T("Коментар має містити від 2 до 5000 символів.");
        return false;
    }

    if (!Connect())
    {
        messageText = _T("Немає підключення до бази даних: ") + m_lastDbError;
        return false;
    }

    try
    {
        CString sql;
        sql.Format(
            _T("INSERT INTO reviews(user_id,book_id,rating,comment,created_at) ")
            _T("VALUES(%d,%d,%d,%s,NOW()) ")
            _T("ON DUPLICATE KEY UPDATE rating=VALUES(rating), comment=VALUES(comment), created_at=NOW()"),
            m_currentUserId, bookId, rating, Quote(commentTrim).GetString());
        m_conn->Execute(_bstr_t(sql), nullptr, adCmdText | adExecuteNoRecords);

        RecalculateRating(bookId);
        messageText = _T("Відгук збережено.");
        return true;
    }
    catch (_com_error& e)
    {
        messageText = CString(e.ErrorMessage());
        return false;
    }
}

std::vector<ReviewRecord> Database::GetReviewsByBook(int bookId) const
{
    std::vector<ReviewRecord> result;
    if (!const_cast<Database*>(this)->Connect()) return result;

    try
    {
        CString sql;
        sql.Format(
            _T("SELECT r.id, r.user_id, r.book_id, r.rating, r.comment, ")
            _T("COALESCE(NULLIF(u.full_name,''), u.login) AS user_name, ")
            _T("DATE_FORMAT(r.created_at,'%%Y-%%m-%%d') AS created ")
            _T("FROM reviews r JOIN users u ON u.id=r.user_id ")
            _T("WHERE r.book_id=%d ORDER BY r.created_at DESC"), bookId);

        _RecordsetPtr rs;
        rs.CreateInstance(__uuidof(Recordset));
        rs->Open(_bstr_t(sql), _variant_t((IDispatch*)m_conn), adOpenForwardOnly, adLockReadOnly, adCmdText);

        while (!rs->ADO_EOF)
        {
            ReviewRecord r;
            r.id = AdoFieldInt(rs, _T("id"));
            r.userId = AdoFieldInt(rs, _T("user_id"));
            r.bookId = AdoFieldInt(rs, _T("book_id"));
            r.rating = AdoFieldInt(rs, _T("rating"));
            r.comment = AdoFieldStr(rs, _T("comment"));
            r.userName = AdoFieldStr(rs, _T("user_name"));
            r.createdAt = AdoFieldStr(rs, _T("created"));
            result.push_back(r);
            rs->MoveNext();
        }
        rs->Close();
    }
    catch (_com_error&) {}

    return result;
}

CString Database::GetUserDisplayName(int userId) const
{
    if (!const_cast<Database*>(this)->Connect()) return _T("Користувач");

    try
    {
        CString sql;
        sql.Format(_T("SELECT COALESCE(NULLIF(full_name,''), login) FROM users WHERE id=%d"), userId);
        CString name;
        if (QueryScalar(const_cast<Database*>(this)->m_conn, sql, name, nullptr))
            return name;
    }
    catch (_com_error&) {}

    return _T("Користувач");
}

bool Database::GetUserByDisplayName(const CString& displayName, UserRecord& outUser) const
{
    if (!const_cast<Database*>(this)->Connect()) return false;

    CString name(displayName);
    name.Trim();
    if (name.IsEmpty()) return false;

    try
    {
        CString sql;
        sql.Format(
            _T("SELECT id, login, password, role, full_name, phone, email ")
            _T("FROM users WHERE full_name=%s OR login=%s LIMIT 1"),
            Quote(name).GetString(), Quote(name).GetString());

        _RecordsetPtr rs;
        rs.CreateInstance(__uuidof(Recordset));
        rs->Open(_bstr_t(sql), _variant_t((IDispatch*)m_conn), adOpenForwardOnly, adLockReadOnly, adCmdText);
        if (rs->ADO_EOF)
        {
            rs->Close();
            return false;
        }

        outUser.id = AdoFieldInt(rs, (short)0);
        outUser.login = AdoFieldStr(rs, (short)1);
        outUser.password = AdoFieldStr(rs, (short)2);
        outUser.role = AdoFieldStr(rs, (short)3);
        outUser.fullName = AdoFieldStr(rs, (short)4);
        outUser.phone = AdoFieldStr(rs, (short)5);
        outUser.email = AdoFieldStr(rs, (short)6);
        rs->Close();
        return outUser.id > 0;
    }
    catch (_com_error&)
    {
        return false;
    }
}

std::vector<ReservationRecord> Database::GetAllReservationsDetailed() const
{
    std::vector<ReservationRecord> result;
    if (!const_cast<Database*>(this)->Connect()) return result;

    try
    {
        CString sql =
            _T("SELECT r.id, r.user_id, r.book_id, r.branch_id, ")
            _T("COALESCE(NULLIF(u.full_name,''), u.login) AS user_name, ")
            _T("b.title AS book_title, br.name AS branch_name, ")
            _T("DATE_FORMAT(r.reservation_date,'%Y-%m-%d') AS res_date, ")
            _T("DATE_FORMAT(r.return_date,'%Y-%m-%d') AS ret_date, ")
            _T("r.status ")
            _T("FROM reservations r ")
            _T("JOIN users u ON u.id=r.user_id ")
            _T("JOIN books b ON b.id=r.book_id ")
            _T("JOIN branches br ON br.id=r.branch_id ")
            _T("ORDER BY r.id DESC");

        _RecordsetPtr rs;
        rs.CreateInstance(__uuidof(Recordset));
        rs->Open(_bstr_t(sql), _variant_t((IDispatch*)m_conn), adOpenForwardOnly, adLockReadOnly, adCmdText);

        while (!rs->ADO_EOF)
        {
            ReservationRecord rec;
            rec.id = AdoFieldInt(rs, (short)0);
            rec.userId = AdoFieldInt(rs, (short)1);
            rec.bookId = AdoFieldInt(rs, (short)2);
            rec.branchId = AdoFieldInt(rs, (short)3);
            rec.userName = AdoFieldStr(rs, (short)4);
            rec.bookTitle = AdoFieldStr(rs, (short)5);
            rec.branchName = AdoFieldStr(rs, (short)6);
            rec.reservationDate = AdoFieldStr(rs, (short)7);
            rec.returnDate = AdoFieldStr(rs, (short)8);
            rec.status = AdoFieldStr(rs, (short)9);
            result.push_back(rec);
            rs->MoveNext();
        }
        rs->Close();
    }
    catch (_com_error&) {}

    return result;
}

std::vector<CString> Database::GetUsersReservationsRows()
{
    std::vector<CString> rows;
    std::vector<ReservationRecord> reservations = GetAllReservationsDetailed();
    
    if (reservations.empty()) {
        rows.push_back(_T("Немає активних бронювань"));
        return rows;
    }
    
    for (const auto& r : reservations) {
        CString row;
        row.Format(_T("%d|%s|%s|%s|%s|%s|%s|%s"),
            r.id, r.userName.GetString(), r.bookTitle.GetString(),
            r.branchName.GetString(), r.reservationDate.GetString(),
            r.returnDate.GetString(), r.status.GetString(), ReservationStatusUiLabel(r.status).GetString());
        rows.push_back(row);
    }
    
    return rows;
}

std::vector<ReservationRecord> Database::GetCurrentUserReservations() const
{
    std::vector<ReservationRecord> result;
    if (!const_cast<Database*>(this)->Connect() || m_currentUserId <= 0) return result;

    try
    {
        CString sql;
        sql.Format(
            _T("SELECT r.id, r.book_id, r.branch_id, b.title, br.name, ")
            _T("DATE_FORMAT(r.reservation_date,'%%Y-%%m-%%d'), ")
            _T("DATE_FORMAT(r.return_date,'%%Y-%%m-%%d'), r.status ")
            _T("FROM reservations r ")
            _T("JOIN books b ON b.id=r.book_id ")
            _T("JOIN branches br ON br.id=r.branch_id ")
            _T("WHERE r.user_id=%d ORDER BY r.reservation_date DESC LIMIT 20"),
            m_currentUserId);

        _RecordsetPtr rs;
        rs.CreateInstance(__uuidof(Recordset));
        rs->Open(_bstr_t(sql), _variant_t((IDispatch*)m_conn), adOpenForwardOnly, adLockReadOnly, adCmdText);

        while (!rs->ADO_EOF)
        {
            ReservationRecord rec;
            rec.id = AdoFieldInt(rs, (short)0);
            rec.bookId = AdoFieldInt(rs, (short)1);
            rec.branchId = AdoFieldInt(rs, (short)2);
            rec.bookTitle = AdoFieldStr(rs, (short)3);
            rec.branchName = AdoFieldStr(rs, (short)4);
            rec.reservationDate = AdoFieldStr(rs, (short)5);
            rec.returnDate = AdoFieldStr(rs, (short)6);
            rec.status = AdoFieldStr(rs, (short)7);
            rec.userId = m_currentUserId;
            result.push_back(rec);
            rs->MoveNext();
        }
        rs->Close();
    }
    catch (_com_error&) {}

    return result;
}

std::vector<ReviewRecord> Database::GetCurrentUserReviews() const
{
    std::vector<ReviewRecord> result;
    if (!const_cast<Database*>(this)->Connect() || m_currentUserId <= 0) return result;

    try
    {
        CString sql;
        sql.Format(
            _T("SELECT r.id, r.book_id, r.rating, r.comment, b.title, ")
            _T("DATE_FORMAT(r.created_at,'%%Y-%%m-%%d') ")
            _T("FROM reviews r JOIN books b ON b.id=r.book_id ")
            _T("WHERE r.user_id=%d ORDER BY r.created_at DESC LIMIT 20"),
            m_currentUserId);

        _RecordsetPtr rs;
        rs.CreateInstance(__uuidof(Recordset));
        rs->Open(_bstr_t(sql), _variant_t((IDispatch*)m_conn), adOpenForwardOnly, adLockReadOnly, adCmdText);

        while (!rs->ADO_EOF)
        {
            ReviewRecord rec;
            rec.id = AdoFieldInt(rs, (short)0);
            rec.bookId = AdoFieldInt(rs, (short)1);
            rec.rating = AdoFieldInt(rs, (short)2);
            rec.comment = AdoFieldStr(rs, (short)3);
            rec.userName = AdoFieldStr(rs, (short)4);
            rec.createdAt = AdoFieldStr(rs, (short)5);
            rec.userId = m_currentUserId;
            result.push_back(rec);
            rs->MoveNext();
        }
        rs->Close();
    }
    catch (_com_error&) {}

    return result;
}

bool Database::AddBook(const BookRecord& book, CString& messageText)
{
    messageText.Empty();

    CString title = Trimmed(book.title);
    CString author = Trimmed(book.author);
    CString category = Trimmed(book.category);
    CString description = Trimmed(book.description);

    if (title.IsEmpty() || title.GetLength() > 255 || !IsPrintableText(title))
    {
        messageText = _T("Назва книги обов'язкова (1-255 символів).");
        return false;
    }
    if (author.IsEmpty() || author.GetLength() > 180 || !IsPrintableText(author))
    {
        messageText = _T("Автор обов'язковий (1-180 символів).");
        return false;
    }
    if (!IsReasonableYear(book.year))
    {
        messageText = _T("Некоректний рік видання.");
        return false;
    }
    if (book.quantityTotal < 0 || book.quantityAvailable < 0 || book.quantityAvailable > book.quantityTotal)
    {
        messageText = _T("Некоректна кількість примірників.");
        return false;
    }
    if (description.GetLength() > 5000 || !IsPrintableText(description))
    {
        messageText = _T("Опис має містити до 5000 символів.");
        return false;
    }

    if (!IsAdmin())
    {
        messageText = _T("Доступ тільки для адміністратора.");
        return false;
    }

    if (!Connect())
    {
        messageText = _T("Немає підключення до бази даних: ") + m_lastDbError;
        return false;
    }

    int categoryId = book.categoryId;
    if (categoryId <= 0)
    {
        categoryId = GetCategoryIdByName(category);
        if (categoryId <= 0)
        {
            messageText = _T("Категорію не знайдено. Спочатку створіть категорію.");
            return false;
        }
    }

    try
    {
        CString sql;
        sql.Format(
            _T("INSERT INTO books(category_id,title,author,year,rating,description,cover_path,quantity_total,quantity_available,created_at) ")
            _T("VALUES(%d,%s,%s,%d,0.0,%s,NULL,%d,%d,CURRENT_TIMESTAMP)"),
            categoryId,
            Quote(title).GetString(),
            Quote(author).GetString(),
            book.year,
            Quote(description).GetString(),
            max(0, book.quantityTotal),
            max(0, min(book.quantityAvailable, book.quantityTotal)));

        m_conn->Execute(_bstr_t(sql), nullptr, adCmdText | adExecuteNoRecords);

        CString idText;
        if (!QueryScalar(m_conn, _T("SELECT LAST_INSERT_ID()"), idText, &messageText))
        {
            messageText = _T("Книгу додано, але не вдалося отримати ID.");
            return true;
        }

        int newBookId = _ttoi(idText);
        if (!book.coverPath.IsEmpty())
        {
            if (IsUrl(book.coverPath))
            {
                CString updateSql;
                updateSql.Format(_T("UPDATE books SET cover_path=%s WHERE id=%d"), Quote(book.coverPath).GetString(), newBookId);
                m_conn->Execute(_bstr_t(updateSql), nullptr, adCmdText | adExecuteNoRecords);
                messageText = _T("Книгу додано успішно!");
            }
            else
            {
                CString copiedRelPath, coverErr;
                if (CopyCoverToBooksFolder(book.coverPath, newBookId, copiedRelPath, coverErr))
                {
                    CString updateSql;
                    updateSql.Format(_T("UPDATE books SET cover_path=%s WHERE id=%d"), Quote(copiedRelPath).GetString(), newBookId);
                    m_conn->Execute(_bstr_t(updateSql), nullptr, adCmdText | adExecuteNoRecords);
                    messageText = _T("Книгу додано успішно!");
                }
                else
                {
                    messageText = _T("Книгу додано, але обкладинку не збережено: ") + coverErr;
                }
            }
        }
        else
        {
            messageText = _T("Книгу додано успішно!");
        }
        return true;
    }
    catch (_com_error& e)
    {
        messageText = CString(e.ErrorMessage());
        return false;
    }
}

bool Database::EditBook(const BookRecord& book, CString& messageText)
{
    messageText.Empty();

    CString title = Trimmed(book.title);
    CString author = Trimmed(book.author);
    CString category = Trimmed(book.category);
    CString description = Trimmed(book.description);

    if (book.id <= 0)
    {
        messageText = _T("Некоректний ID книги.");
        return false;
    }
    if (title.IsEmpty() || title.GetLength() > 255 || !IsPrintableText(title))
    {
        messageText = _T("Назва книги обов'язкова (1-255 символів).");
        return false;
    }
    if (author.IsEmpty() || author.GetLength() > 180 || !IsPrintableText(author))
    {
        messageText = _T("Автор обов'язковий (1-180 символів).");
        return false;
    }
    if (!IsReasonableYear(book.year))
    {
        messageText = _T("Некоректний рік видання.");
        return false;
    }
    if (book.quantityTotal < 0 || book.quantityAvailable < 0 || book.quantityAvailable > book.quantityTotal)
    {
        messageText = _T("Некоректна кількість примірників.");
        return false;
    }
    if (description.GetLength() > 5000 || !IsPrintableText(description))
    {
        messageText = _T("Опис має містити до 5000 символів.");
        return false;
    }

    if (!IsAdmin())
    {
        messageText = _T("Доступ тільки для адміністратора.");
        return false;
    }

    if (!Connect())
    {
        messageText = _T("Немає підключення до бази даних: ") + m_lastDbError;
        return false;
    }

    int categoryId = book.categoryId;
    if (categoryId <= 0)
    {
        categoryId = GetCategoryIdByName(category);
        if (categoryId <= 0)
        {
            messageText = _T("Категорію не знайдено. Спочатку створіть категорію.");
            return false;
        }
    }

    try
    {
        CString finalCoverPath = book.coverPath;
        CString lowerPath = ToLowerText(finalCoverPath);
        const bool isStoredRelative = lowerPath.Left(6) == _T("books/");
        const bool isUrlPath = IsUrl(finalCoverPath);

        if (!finalCoverPath.IsEmpty() && !isStoredRelative && !isUrlPath)
        {
            CString absCover;
            if (!ResolveReadablePath(finalCoverPath, absCover))
            {
                messageText = _T("Файл обкладинки не знайдено.");
                return false;
            }

            CString copiedRelPath, coverErr;
            if (!CopyCoverToBooksFolder(absCover, book.id, copiedRelPath, coverErr))
            {
                messageText = coverErr;
                return false;
            }
            finalCoverPath = copiedRelPath;
        }

        CString coverSql = finalCoverPath.IsEmpty() ? _T("NULL") : Quote(finalCoverPath);

        CString sql;
        sql.Format(
            _T("UPDATE books SET category_id=%d,title=%s,author=%s,year=%d,description=%s,cover_path=%s,quantity_total=%d,quantity_available=%d WHERE id=%d"),
            categoryId,
            Quote(title).GetString(),
            Quote(author).GetString(),
            book.year,
            Quote(description).GetString(),
            coverSql.GetString(),
            max(0, book.quantityTotal),
            max(0, min(book.quantityAvailable, book.quantityTotal)),
            book.id);

        m_conn->Execute(_bstr_t(sql), nullptr, adCmdText | adExecuteNoRecords);
        messageText = _T("Книгу оновлено успішно!");
        return true;
    }
    catch (_com_error& e)
    {
        messageText = CString(e.ErrorMessage());
        return false;
    }
}

bool Database::DeleteBook(int bookId, CString& messageText)
{
    messageText.Empty();

    if (!IsAdmin())
    {
        messageText = _T("Доступ тільки для адміністратора.");
        return false;
    }

    if (!Connect())
    {
        messageText = _T("Немає підключення до бази даних: ") + m_lastDbError;
        return false;
    }

    try
    {
        CString checkSql;
        checkSql.Format(_T("SELECT COUNT(*) FROM reservations WHERE book_id=%d AND status IN ('reserved','ready_for_pickup','issued','overdue','return_requested')"), bookId);
        CString countStr;
        if (QueryScalar(m_conn, checkSql, countStr, nullptr) && _ttoi(countStr) > 0)
        {
            messageText = _T("Неможливо видалити: книга має активні бронювання.");
            return false;
        }

        CString sql;
        sql.Format(_T("DELETE FROM books WHERE id=%d"), bookId);
        m_conn->Execute(_bstr_t(sql), nullptr, adCmdText | adExecuteNoRecords);
        messageText = _T("Книгу видалено успішно!");
        return true;
    }
    catch (_com_error& e)
    {
        messageText = CString(e.ErrorMessage());
        return false;
    }
}

std::vector<CategoryStat> Database::GetCategoryStats() const
{
    std::vector<CategoryStat> stats;
    if (!const_cast<Database*>(this)->Connect()) return stats;

    try
    {
        CString sql =
            _T("SELECT c.name, COUNT(b.id) AS cnt ")
            _T("FROM categories c LEFT JOIN books b ON b.category_id=c.id ")
            _T("GROUP BY c.id,c.name ORDER BY cnt DESC, c.name");

        _RecordsetPtr rs;
        rs.CreateInstance(__uuidof(Recordset));
        rs->Open(_bstr_t(sql), _variant_t((IDispatch*)m_conn), adOpenForwardOnly, adLockReadOnly, adCmdText);

        while (!rs->ADO_EOF)
        {
            CategoryStat s;
            s.name = AdoFieldStr(rs, (short)0);
            s.count = AdoFieldInt(rs, (short)1);
            stats.push_back(s);
            rs->MoveNext();
        }
        rs->Close();
    }
    catch (_com_error&) {}

    return stats;
}

void Database::GetAvailabilityStats(int& availableCopies, int& issuedCopies) const
{
    availableCopies = 0;
    issuedCopies = 0;

    if (!const_cast<Database*>(this)->Connect()) return;

    try
    {
        CString sql = _T("SELECT COALESCE(SUM(quantity_available),0), COALESCE(SUM(quantity_total - quantity_available),0) FROM books");
        _RecordsetPtr rs;
        rs.CreateInstance(__uuidof(Recordset));
        rs->Open(_bstr_t(sql), _variant_t((IDispatch*)m_conn), adOpenForwardOnly, adLockReadOnly, adCmdText);
        if (!rs->ADO_EOF)
        {
            availableCopies = AdoFieldInt(rs, (short)0);
            issuedCopies = AdoFieldInt(rs, (short)1);
        }
        rs->Close();
    }
    catch (_com_error&) {}
}

std::vector<CategoryStat> Database::GetUserActivityStats() const
{
    std::vector<CategoryStat> stats;
    if (!const_cast<Database*>(this)->Connect()) return stats;

    try
    {
        CString sql =
            _T("SELECT COALESCE(NULLIF(TRIM(u.full_name),''), u.login) AS uname, ")
            _T("(SELECT COUNT(*) FROM reservations r WHERE r.user_id=u.id) + ")
            _T("(SELECT COUNT(*) FROM reviews rv WHERE rv.user_id=u.id) AS activity ")
            _T("FROM users u ORDER BY activity DESC, uname LIMIT 10");

        _RecordsetPtr rs;
        rs.CreateInstance(__uuidof(Recordset));
        rs->Open(_bstr_t(sql), _variant_t((IDispatch*)m_conn), adOpenForwardOnly, adLockReadOnly, adCmdText);

        while (!rs->ADO_EOF)
        {
            CategoryStat s;
            s.name = AdoFieldStr(rs, (short)0);
            s.count = AdoFieldInt(rs, (short)1);
            if (s.count > 0) stats.push_back(s);
            rs->MoveNext();
        }
        rs->Close();
    }
    catch (_com_error&) {}

    return stats;
}

AnalyticsData Database::GetAnalytics() const
{
    AnalyticsData data;
    if (!const_cast<Database*>(this)->Connect()) return data;

    try
    {
        CString val;
        if (QueryScalar(const_cast<Database*>(this)->m_conn, _T("SELECT COUNT(*) FROM books"), val, nullptr))
            data.totalBooks = _ttoi(val);
        if (QueryScalar(const_cast<Database*>(this)->m_conn, _T("SELECT COUNT(*) FROM categories"), val, nullptr))
            data.totalCategories = _ttoi(val);
        if (QueryScalar(const_cast<Database*>(this)->m_conn, _T("SELECT COUNT(*) FROM users"), val, nullptr))
            data.totalUsers = _ttoi(val);
        if (QueryScalar(const_cast<Database*>(this)->m_conn, _T("SELECT COUNT(*) FROM branches"), val, nullptr))
            data.totalBranches = _ttoi(val);
        if (QueryScalar(const_cast<Database*>(this)->m_conn, _T("SELECT COUNT(*) FROM reservations WHERE status IN ('reserved','ready_for_pickup','issued','overdue','return_requested')"), val, nullptr))
            data.activeReservations = _ttoi(val);
        if (QueryScalar(const_cast<Database*>(this)->m_conn, _T("SELECT COUNT(*) FROM reservations WHERE status='returned'"), val, nullptr))
            data.returnedBooks = _ttoi(val);
        if (QueryScalar(const_cast<Database*>(this)->m_conn, _T("SELECT COUNT(*) FROM reviews"), val, nullptr))
            data.totalReviews = _ttoi(val);
        if (QueryScalar(const_cast<Database*>(this)->m_conn, _T("SELECT COALESCE(AVG(rating),0) FROM reviews"), val, nullptr))
            data.avgRating = (float)_tstof(val);
    }
    catch (_com_error&) {}

    return data;
}

std::vector<BookRecord> Database::GetTopRatedBooks(int limit) const
{
    std::vector<BookRecord> result;
    if (!const_cast<Database*>(this)->Connect()) return result;

    try
    {
        CString sql;
        sql.Format(
            _T("SELECT b.id, b.title, b.author, b.rating, c.name AS category ")
            _T("FROM books b JOIN categories c ON c.id=b.category_id ")
            _T("WHERE b.rating > 0 ORDER BY b.rating DESC LIMIT %d"), limit);

        _RecordsetPtr rs;
        rs.CreateInstance(__uuidof(Recordset));
        rs->Open(_bstr_t(sql), _variant_t((IDispatch*)m_conn), adOpenForwardOnly, adLockReadOnly, adCmdText);

        while (!rs->ADO_EOF)
        {
            BookRecord b;
            b.id = AdoFieldInt(rs, _T("id"));
            b.title = AdoFieldStr(rs, _T("title"));
            b.author = AdoFieldStr(rs, _T("author"));
            b.rating = AdoFieldFloat(rs, _T("rating"));
            b.category = AdoFieldStr(rs, _T("category"));
            result.push_back(b);
            rs->MoveNext();
        }
        rs->Close();
    }
    catch (_com_error&) {}

    return result;
}

std::vector<BookRecord> Database::GetMostReservedBooks(int limit) const
{
    std::vector<BookRecord> result;
    if (!const_cast<Database*>(this)->Connect()) return result;

    try
    {
        CString sql;
        sql.Format(
            _T("SELECT b.id, b.title, b.author, COUNT(r.id) AS res_count ")
            _T("FROM books b LEFT JOIN reservations r ON r.book_id=b.id ")
            _T("GROUP BY b.id, b.title, b.author ")
            _T("HAVING res_count > 0 ORDER BY res_count DESC LIMIT %d"), limit);

        _RecordsetPtr rs;
        rs.CreateInstance(__uuidof(Recordset));
        rs->Open(_bstr_t(sql), _variant_t((IDispatch*)m_conn), adOpenForwardOnly, adLockReadOnly, adCmdText);

        while (!rs->ADO_EOF)
        {
            BookRecord b;
            b.id = AdoFieldInt(rs, _T("id"));
            b.title = AdoFieldStr(rs, _T("title"));
            b.author = AdoFieldStr(rs, _T("author"));
            b.quantityTotal = AdoFieldInt(rs, _T("res_count"));
            result.push_back(b);
            rs->MoveNext();
        }
        rs->Close();
    }
    catch (_com_error&) {}

    return result;
}

void Database::RecalculateRating(int bookId)
{
    if (!Connect()) return;

    try
    {
        CString sql;
        sql.Format(
            _T("UPDATE books b SET b.rating=(SELECT COALESCE(AVG(r.rating),0) FROM reviews r WHERE r.book_id=b.id) WHERE b.id=%d"),
            bookId);
        m_conn->Execute(_bstr_t(sql), nullptr, adCmdText | adExecuteNoRecords);
    }
    catch (_com_error&) {}
}

std::vector<CString> Database::GetBranches()
{
    std::vector<CString> result;
    if (!Connect()) return result;

    try
    {
        _RecordsetPtr rs;
        rs.CreateInstance(__uuidof(Recordset));
        rs->Open(_bstr_t(_T("SELECT name FROM branches ORDER BY name")), _variant_t((IDispatch*)m_conn), adOpenForwardOnly, adLockReadOnly, adCmdText);
        while (!rs->ADO_EOF)
        {
            CString name = AdoFieldStr(rs, (short)0);
            name.Trim();
            result.push_back(name);
            rs->MoveNext();
        }
        rs->Close();
    }
    catch (_com_error&) {}

    return result;
}

bool Database::CanUserReview(int userId, int bookId)
{
    if (!Connect()) return false;

    try
    {
        CString sql;
        sql.Format(
            _T("SELECT COUNT(*) FROM reservations WHERE user_id=%d AND book_id=%d AND status='returned'"),
            userId, bookId);
        CString countStr;
        if (QueryScalar(m_conn, sql, countStr, nullptr))
            return _ttoi(countStr) > 0;
    }
    catch (_com_error&) {}

    return false;
}

void Database::UpdateReservationStatus(int reservationId, const CString& newStatus)
{
    if (!Connect()) return;

    CString normalizedStatus = ToLowerText(Trimmed(newStatus));
    if (reservationId <= 0 || !IsAllowedReservationStatus(normalizedStatus)) return;

    try
    {
        CString oldStatus;
        CString oldStatusSql;
        oldStatusSql.Format(_T("SELECT status FROM reservations WHERE id=%d"), reservationId);
        if (!QueryScalar(m_conn, oldStatusSql, oldStatus, nullptr)) return;
        oldStatus = ToLowerText(Trimmed(oldStatus));

        CString sql;
        sql.Format(_T("UPDATE reservations SET status=%s WHERE id=%d"), Quote(normalizedStatus).GetString(), reservationId);
        m_conn->Execute(_bstr_t(sql), nullptr, adCmdText | adExecuteNoRecords);

        bool wasActive = IsInventoryActiveStatus(oldStatus);
        bool nowActive = IsInventoryActiveStatus(normalizedStatus);
        bool shouldReturnCopy = IsInventoryReturnStatus(normalizedStatus);

        if (wasActive && !nowActive && shouldReturnCopy)
        {
            CString bookIdStr;
            CString sql2;
            sql2.Format(_T("SELECT book_id FROM reservations WHERE id=%d"), reservationId);
            if (QueryScalar(m_conn, sql2, bookIdStr, nullptr))
            {
                int bookId = _ttoi(bookIdStr);
                CString updateSql;
                updateSql.Format(_T("UPDATE books SET quantity_available=quantity_available+1 WHERE id=%d"), bookId);
                m_conn->Execute(_bstr_t(updateSql), nullptr, adCmdText | adExecuteNoRecords);
            }
        }
        else if (!wasActive && nowActive)
        {
            CString bookIdStr;
            CString sql2;
            sql2.Format(_T("SELECT book_id FROM reservations WHERE id=%d"), reservationId);
            if (QueryScalar(m_conn, sql2, bookIdStr, nullptr))
            {
                int bookId = _ttoi(bookIdStr);
                CString updateSql;
                updateSql.Format(_T("UPDATE books SET quantity_available=quantity_available-1 WHERE id=%d AND quantity_available>0"), bookId);
                m_conn->Execute(_bstr_t(updateSql), nullptr, adCmdText | adExecuteNoRecords);
            }
        }
    }
    catch (_com_error&) {}
}

bool Database::CancelActiveReservationForCurrentUser(int bookId, CString& messageText)
{
    messageText.Empty();
    if (!IsLoggedIn())
    {
        messageText = _T("Спочатку увійдіть.");
        return false;
    }
    if (bookId <= 0)
    {
        messageText = _T("Некоректний ID книги.");
        return false;
    }
    if (!Connect())
    {
        messageText = _T("Немає підключення до БД.");
        return false;
    }

    try
    {
        CString idSql;
        idSql.Format(
            _T("SELECT id FROM reservations WHERE user_id=%d AND book_id=%d ")
            _T("AND status IN ('reserved','ready_for_pickup','return_requested') ")
            _T("ORDER BY id DESC LIMIT 1"),
            m_currentUserId, bookId);

        CString resId;
        if (!QueryScalar(m_conn, idSql, resId, nullptr) || _ttoi(resId) <= 0)
        {
            messageText = _T("Немає бронювання, яке можна скасувати.");
            return false;
        }

        UpdateReservationStatus(_ttoi(resId), _T("cancelled"));
        messageText = _T("Бронювання скасовано.");
        return true;
    }
    catch (_com_error& e)
    {
        messageText = CString(e.ErrorMessage());
        return false;
    }
}

bool Database::UpdateCurrentUserReservationStatus(int reservationId, const CString& newStatus, CString& messageText)
{
    messageText.Empty();
    if (!IsLoggedIn())
    {
        messageText = _T("Спочатку увійдіть.");
        return false;
    }
    if (reservationId <= 0)
    {
        messageText = _T("Некоректний ID бронювання.");
        return false;
    }

    CString normalizedStatus = ToLowerText(Trimmed(newStatus));
    if (!IsAllowedReservationStatus(normalizedStatus))
    {
        messageText = _T("Некоректний статус бронювання.");
        return false;
    }

    if (!Connect())
    {
        messageText = _T("Немає підключення до БД.");
        return false;
    }

    try
    {
        CString ownerSql;
        ownerSql.Format(_T("SELECT user_id,status FROM reservations WHERE id=%d LIMIT 1"), reservationId);

        _RecordsetPtr rs;
        rs.CreateInstance(__uuidof(Recordset));
        rs->Open(_bstr_t(ownerSql), _variant_t((IDispatch*)m_conn), adOpenForwardOnly, adLockReadOnly, adCmdText);
        if (rs->ADO_EOF)
        {
            rs->Close();
            messageText = _T("Бронювання не знайдено.");
            return false;
        }

        int ownerId = AdoFieldInt(rs, (short)0);
        CString currentStatus = AdoFieldStr(rs, (short)1);
        rs->Close();

        if (ownerId != m_currentUserId)
        {
            messageText = _T("Можна змінювати лише власні бронювання.");
            return false;
        }

        CString from = ToLowerText(Trimmed(currentStatus));
        bool toCancelled = (normalizedStatus == _T("cancelled"));
        bool toLostOrDamaged = (normalizedStatus == _T("lost") || normalizedStatus == _T("damaged"));

        if (toCancelled)
        {
            if (!(from == _T("reserved") || from == _T("ready_for_pickup") || from == _T("return_requested")))
            {
                messageText = _T("Це бронювання вже не можна скасувати.");
                return false;
            }
        }
        else if (toLostOrDamaged)
        {
            if (!(from == _T("issued") || from == _T("overdue") || from == _T("return_requested") || from == _T("ready_for_pickup")))
            {
                messageText = _T("Для цього статусу не можна повідомити про втрату або пошкодження.");
                return false;
            }
        }
        else
        {
            messageText = _T("Цю зміну статусу з профілю не дозволено.");
            return false;
        }

        UpdateReservationStatus(reservationId, normalizedStatus);

        if (normalizedStatus == _T("cancelled"))
            messageText = _T("Бронювання скасовано.");
        else if (normalizedStatus == _T("lost"))
            messageText = _T("Позначено як «Втрачено».");
        else
            messageText = _T("Позначено як «Пошкоджено».");

        return true;
    }
    catch (_com_error& e)
    {
        messageText = CString(e.ErrorMessage());
        return false;
    }
}

bool Database::EditCurrentUserReview(int reviewId, int rating, const CString& comment, CString& messageText)
{
    messageText.Empty();
    CString commentTrim = Trimmed(comment);

    if (!IsLoggedIn())
    {
        messageText = _T("Спочатку увійдіть.");
        return false;
    }
    if (reviewId <= 0)
    {
        messageText = _T("Некоректний ID відгуку.");
        return false;
    }
    if (rating < 1 || rating > 5)
    {
        messageText = _T("Оцінка має бути від 1 до 5.");
        return false;
    }
    if (commentTrim.GetLength() < 2 || commentTrim.GetLength() > 5000 || !IsPrintableText(commentTrim))
    {
        messageText = _T("Коментар має містити від 2 до 5000 символів.");
        return false;
    }
    if (!Connect())
    {
        messageText = _T("Немає підключення до БД.");
        return false;
    }

    try
    {
        CString bookIdSql;
        bookIdSql.Format(
            _T("SELECT book_id FROM reviews WHERE id=%d AND user_id=%d LIMIT 1"),
            reviewId, m_currentUserId);

        CString bookIdText;
        if (!QueryScalar(m_conn, bookIdSql, bookIdText, nullptr) || _ttoi(bookIdText) <= 0)
        {
            messageText = _T("Відгук не знайдено або немає доступу.");
            return false;
        }

        CString sql;
        sql.Format(
            _T("UPDATE reviews SET rating=%d, comment=%s, created_at=NOW() WHERE id=%d AND user_id=%d"),
            rating,
            Quote(commentTrim).GetString(),
            reviewId,
            m_currentUserId);
        m_conn->Execute(_bstr_t(sql), nullptr, adCmdText | adExecuteNoRecords);

        RecalculateRating(_ttoi(bookIdText));
        messageText = _T("Відгук оновлено.");
        return true;
    }
    catch (_com_error& e)
    {
        messageText = CString(e.ErrorMessage());
        return false;
    }
}

bool Database::DeleteCurrentUserReview(int reviewId, CString& messageText)
{
    messageText.Empty();
    if (!IsLoggedIn())
    {
        messageText = _T("Спочатку увійдіть.");
        return false;
    }
    if (reviewId <= 0)
    {
        messageText = _T("Некоректний ID відгуку.");
        return false;
    }
    if (!Connect())
    {
        messageText = _T("Немає підключення до БД.");
        return false;
    }

    try
    {
        CString bookIdSql;
        bookIdSql.Format(
            _T("SELECT book_id FROM reviews WHERE id=%d AND user_id=%d LIMIT 1"),
            reviewId, m_currentUserId);

        CString bookIdText;
        if (!QueryScalar(m_conn, bookIdSql, bookIdText, nullptr) || _ttoi(bookIdText) <= 0)
        {
            messageText = _T("Відгук не знайдено або немає доступу.");
            return false;
        }

        CString sql;
        sql.Format(_T("DELETE FROM reviews WHERE id=%d AND user_id=%d"), reviewId, m_currentUserId);
        m_conn->Execute(_bstr_t(sql), nullptr, adCmdText | adExecuteNoRecords);

        RecalculateRating(_ttoi(bookIdText));
        messageText = _T("Відгук видалено.");
        return true;
    }
    catch (_com_error& e)
    {
        messageText = CString(e.ErrorMessage());
        return false;
    }
}

bool Database::AddBranch(const CString& name, const CString& address, CString& errorText)
{
    errorText.Empty();
    CString nameTrim = Trimmed(name);
    CString addressTrim = Trimmed(address);

    if (nameTrim.IsEmpty())
    {
        errorText = _T("Назва філії не може бути порожньою.");
        return false;
    }

    if (nameTrim.GetLength() < 2 || nameTrim.GetLength() > 120 || !IsPrintableText(nameTrim))
    {
        errorText = _T("Назва філії має містити від 2 до 120 символів.");
        return false;
    }
    if (addressTrim.IsEmpty() || addressTrim.GetLength() > 255 || !IsPrintableText(addressTrim))
    {
        errorText = _T("Адреса філії обов'язкова (1-255 символів).");
        return false;
    }

    if (!Connect())
    {
        errorText = _T("Немає підключення до БД.");
        return false;
    }

    try
    {
        CString sql;
        sql.Format(_T("INSERT INTO branches(name, address) VALUES(%s, %s)"),
            Quote(nameTrim).GetString(), Quote(addressTrim).GetString());
        m_conn->Execute(_bstr_t(sql), nullptr, adCmdText | adExecuteNoRecords);
        return true;
    }
    catch (_com_error& e)
    {
        errorText = CString(e.ErrorMessage());
        return false;
    }
}

bool Database::EditBranch(int branchId, const CString& name, const CString& address, CString& errorText)
{
    errorText.Empty();
    CString nameTrim = Trimmed(name);
    CString addressTrim = Trimmed(address);

    if (branchId <= 0)
    {
        errorText = _T("Некоректний ID філії.");
        return false;
    }

    if (nameTrim.IsEmpty())
    {
        errorText = _T("Назва філії не може бути порожньою.");
        return false;
    }

    if (nameTrim.GetLength() < 2 || nameTrim.GetLength() > 120 || !IsPrintableText(nameTrim))
    {
        errorText = _T("Назва філії має містити від 2 до 120 символів.");
        return false;
    }
    if (addressTrim.IsEmpty() || addressTrim.GetLength() > 255 || !IsPrintableText(addressTrim))
    {
        errorText = _T("Адреса філії обов'язкова (1-255 символів).");
        return false;
    }

    if (!Connect())
    {
        errorText = _T("Немає підключення до БД.");
        return false;
    }

    try
    {
        CString sql;
        sql.Format(_T("UPDATE branches SET name=%s, address=%s WHERE id=%d"),
            Quote(nameTrim).GetString(), Quote(addressTrim).GetString(), branchId);
        m_conn->Execute(_bstr_t(sql), nullptr, adCmdText | adExecuteNoRecords);
        return true;
    }
    catch (_com_error& e)
    {
        errorText = CString(e.ErrorMessage());
        return false;
    }
}

bool Database::DeleteBranch(int branchId, CString& errorText)
{
    errorText.Empty();

    if (!Connect())
    {
        errorText = _T("Немає підключення до БД.");
        return false;
    }

    try
    {
        CString checkSql;
        checkSql.Format(_T("SELECT COUNT(*) FROM reservations WHERE branch_id=%d AND status IN ('reserved','ready_for_pickup','issued','overdue','return_requested')"), branchId);
        CString countStr;
        if (QueryScalar(m_conn, checkSql, countStr, nullptr) && _ttoi(countStr) > 0)
        {
            errorText = _T("Неможливо видалити: є активні бронювання у цій філії.");
            return false;
        }

        CString sql;
        sql.Format(_T("DELETE FROM branches WHERE id=%d"), branchId);
        m_conn->Execute(_bstr_t(sql), nullptr, adCmdText | adExecuteNoRecords);
        return true;
    }
    catch (_com_error& e)
    {
        errorText = CString(e.ErrorMessage());
        return false;
    }
}

std::vector<Database::BranchRecord> Database::GetAllBranches()
{
    std::vector<BranchRecord> result;
    if (!Connect()) return result;

    try
    {
        CString sql =
            _T("SELECT b.id, b.name, b.address, ")
            _T("(SELECT COUNT(*) FROM reservations r WHERE r.branch_id=b.id AND r.status IN ('reserved','ready_for_pickup','issued','overdue','return_requested')) AS res_count ")
            _T("FROM branches b ORDER BY b.name");

        _RecordsetPtr rs;
        rs.CreateInstance(__uuidof(Recordset));
        rs->Open(_bstr_t(sql), _variant_t((IDispatch*)m_conn), adOpenForwardOnly, adLockReadOnly, adCmdText);

        while (!rs->ADO_EOF)
        {
            BranchRecord br;
            br.id = AdoFieldInt(rs, (short)0);
            br.name = AdoFieldStr(rs, (short)1);
            br.address = AdoFieldStr(rs, (short)2);
            br.reservationCount = AdoFieldInt(rs, (short)3);
            br.name.Trim();
            br.address.Trim();
            result.push_back(br);
            rs->MoveNext();
        }
        rs->Close();
    }
    catch (_com_error&) {}

    return result;
}
