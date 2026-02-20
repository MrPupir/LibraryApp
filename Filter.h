#pragma once

#include <vector>
#include "Database.h"

class Filter
{
public:
	enum SortMode
	{
		SORT_TITLE = 0,
		SORT_AUTHOR = 1,
		SORT_YEAR = 2,
		SORT_CATEGORY = 3,
		SORT_RATING = 4
	};

	Filter();

	void SetCategories(const std::vector<CategoryRecord>& categories);
	void NextCategory();
	void PrevCategory();
	void NextSortMode();
	void SetSortMode(SortMode mode);
	void ToggleSortDirection();

	bool Accept(const BookRecord& book) const;
	void Sort(std::vector<BookRecord>& books) const;

	CString GetCategoryLabel() const;
	CString GetSortLabel() const;
	SortMode GetSortMode() const { return m_sortMode; }
	bool IsSortAscending() const { return m_sortAscending; }

private:
	std::vector<CString> m_categories;
	int m_categoryIndex;
	SortMode m_sortMode;
	bool m_sortAscending;
};
