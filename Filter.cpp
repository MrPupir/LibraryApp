#include "pch.h"
#include "Filter.h"

#include <algorithm>

Filter::Filter()
	: m_categoryIndex(-1)
	, m_sortMode(SORT_TITLE)
	, m_sortAscending(true)
{
}

void Filter::SetCategories(const std::vector<CategoryRecord>& categories)
{
	m_categories.clear();
	m_categories.push_back(_T("Усі категорії"));
	for (const auto& c : categories)
	{
		if (!c.name.IsEmpty())
		{
			m_categories.push_back(c.name);
		}
	}
	if (m_categories.empty())
	{
		m_categories.push_back(_T("Усі категорії"));
	}
	m_categoryIndex = 0;
}

void Filter::NextCategory()
{
	if (m_categories.empty()) return;
	m_categoryIndex = (m_categoryIndex + 1) % (int)m_categories.size();
}

void Filter::PrevCategory()
{
	if (m_categories.empty()) return;
	m_categoryIndex = (m_categoryIndex - 1 + (int)m_categories.size()) % (int)m_categories.size();
}

void Filter::NextSortMode()
{
	m_sortMode = (SortMode)(((int)m_sortMode + 1) % 5);
}

void Filter::SetSortMode(SortMode mode)
{
	m_sortMode = mode;
}

void Filter::ToggleSortDirection()
{
	m_sortAscending = !m_sortAscending;
}

bool Filter::Accept(const BookRecord& book) const
{
	if (m_categoryIndex <= 0 || m_categoryIndex >= (int)m_categories.size())
	{
		return true;
	}
	return book.category.CompareNoCase(m_categories[m_categoryIndex]) == 0;
}

void Filter::Sort(std::vector<BookRecord>& books) const
{
	std::sort(books.begin(), books.end(), [this](const BookRecord& a, const BookRecord& b) {
		int cmp = 0;
		switch (m_sortMode)
		{
		case SORT_TITLE:
			cmp = a.title.CompareNoCase(b.title);
			break;
		case SORT_AUTHOR:
			cmp = a.author.CompareNoCase(b.author);
			break;
		case SORT_YEAR:
			cmp = (a.year < b.year) ? -1 : ((a.year > b.year) ? 1 : 0);
			break;
		case SORT_CATEGORY:
			cmp = a.category.CompareNoCase(b.category);
			break;
		case SORT_RATING:
			cmp = (a.rating < b.rating) ? -1 : ((a.rating > b.rating) ? 1 : 0);
			break;
		default:
			cmp = 0;
			break;
		}

		if (cmp == 0)
		{
			cmp = a.title.CompareNoCase(b.title);
		}
		return m_sortAscending ? (cmp < 0) : (cmp > 0);
	});
}

CString Filter::GetCategoryLabel() const
{
	if (m_categoryIndex < 0 || m_categoryIndex >= (int)m_categories.size())
	{
		return _T("Усі категорії");
	}
	return m_categories[m_categoryIndex];
}

CString Filter::GetSortLabel() const
{
	switch (m_sortMode)
	{
	case SORT_TITLE:
		return _T("Назва");
	case SORT_AUTHOR:
		return _T("Автор");
	case SORT_YEAR:
		return _T("Рік");
	case SORT_CATEGORY:
		return _T("Категорія");
	case SORT_RATING:
		return _T("Рейтинг");
	default:
		return _T("Назва");
	}
}
