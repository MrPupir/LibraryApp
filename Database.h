#pragma once

#include <vector>

struct UserRecord
{
    int id = 0;
    CString login;
    CString password;
    CString role;
    CString fullName;
    CString phone;
    CString email;
    CString createdAt;
};

struct ReviewRecord
{
    int id = 0;
    int userId = 0;
    int bookId = 0;
    int rating = 0;
    CString comment;
    CString userName;
    CString createdAt;
};

struct ReservationRecord
{
    int id = 0;
    int userId = 0;
    int bookId = 0;
    int branchId = 0;
    CString userName;
    CString bookTitle;
    CString branchName;
    CString reservationDate;
    CString returnDate;
    CString status;
};

struct BookRecord
{
    int id = 0;
    int categoryId = 0;
    CString title;
    CString author;
    int year = 0;
    CString category;
    float rating = 0.0f;
    CString description;
    CString coverPath;
    int quantityTotal = 0;
    int quantityAvailable = 0;
    CString createdAt;
};

struct CategoryRecord
{
    int id = 0;
    CString name;
    int bookCount = 0;
};

struct CategoryStat
{
    CString name;
    int count = 0;
};

struct AnalyticsData
{
    int totalBooks = 0;
    int totalCategories = 0;
    int totalUsers = 0;
    int totalBranches = 0;
    int activeReservations = 0;
    int returnedBooks = 0;
    int totalReviews = 0;
    float avgRating = 0.0f;
};

class Database
{
public:
    static Database& Instance();

    bool Connect();
    void Disconnect();
    bool IsConnected() const;
    CString GetLastDbError() const { return m_lastDbError; }

    bool Login(const CString& login, const CString& password, CString& errorText);
    bool RegisterUser(const CString& login, const CString& password, const CString& email, const CString& phone, const CString& fullName, CString& errorText);
    void Logout();

    bool IsLoggedIn() const;
    bool IsAdmin() const;
    int GetCurrentUserId() const;
    CString GetCurrentRole() const;
    CString GetCurrentLogin() const;
    CString GetCurrentFullName() const;
    CString GetCurrentEmail() const;
    CString GetCurrentPhone() const;

    int GetBooksCount() const;
    std::vector<BookRecord> GetBooksPage(int page, int pageSize) const;
    std::vector<BookRecord> GetAllBooks() const;
    bool GetBookById(int bookId, BookRecord& outBook) const;

    bool ReserveBook(int bookId, int days, const CString& branchName, CString& messageText);
    bool AddReview(int bookId, int rating, const CString& comment, CString& messageText);
    std::vector<ReviewRecord> GetReviewsByBook(int bookId) const;
    CString GetUserDisplayName(int userId) const;
    bool GetUserByDisplayName(const CString& displayName, UserRecord& outUser) const;
    std::vector<ReservationRecord> GetAllReservationsDetailed() const;
    std::vector<ReservationRecord> GetCurrentUserReservations() const;
    std::vector<ReviewRecord> GetCurrentUserReviews() const;

    bool AddBook(const BookRecord& book, CString& messageText);
    bool EditBook(const BookRecord& book, CString& messageText);
    bool DeleteBook(int bookId, CString& messageText);

    std::vector<CategoryRecord> GetAllCategories() const;
    bool AddCategory(const CString& name, CString& errorText);
    bool EditCategory(int categoryId, const CString& name, CString& errorText);
    bool DeleteCategory(int categoryId, CString& errorText);
    bool CategoryExists(const CString& name) const;
    int GetCategoryIdByName(const CString& name) const;

    std::vector<CategoryStat> GetCategoryStats() const;
    void GetAvailabilityStats(int& availableCopies, int& issuedCopies) const;
    std::vector<CategoryStat> GetUserActivityStats() const;
    AnalyticsData GetAnalytics() const;
    std::vector<BookRecord> GetTopRatedBooks(int limit = 5) const;
    std::vector<BookRecord> GetMostReservedBooks(int limit = 5) const;

    std::vector<CString> GetBranches();
    bool CanUserReview(int userId, int bookId);
    void UpdateReservationStatus(int reservationId, const CString& newStatus);
    bool CancelActiveReservationForCurrentUser(int bookId, CString& messageText);
    bool UpdateCurrentUserReservationStatus(int reservationId, const CString& newStatus, CString& messageText);
    bool EditCurrentUserReview(int reviewId, int rating, const CString& comment, CString& messageText);
    bool DeleteCurrentUserReview(int reviewId, CString& messageText);

    bool AddBranch(const CString& name, const CString& address, CString& errorText);
    bool EditBranch(int branchId, const CString& name, const CString& address, CString& errorText);
    bool DeleteBranch(int branchId, CString& errorText);

    std::vector<CString> GetUsersReservationsRows();

    struct BranchRecord {
        int id = 0;
        CString name;
        CString address;
        int reservationCount = 0;
    };
    std::vector<BranchRecord> GetAllBranches();

private:
    Database();
    void RecalculateRating(int bookId);

private:
    ADODB::_ConnectionPtr m_conn;
    CString m_lastDbError;
    CString m_connectedDbName;

    int m_currentUserId = 0;
    CString m_currentLogin;
    CString m_currentRole;
    CString m_currentFullName;
    CString m_currentPhone;
    CString m_currentEmail;
};

