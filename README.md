# LibraryApp

A Windows desktop app for a library with several branches. It covers the book catalog, reservations, reviews and usage charts. Written in C++ with MFC on MySQL (ADO over ODBC) as a university project in February 2026. The interface is in Ukrainian.

## Features

- Login and registration. Registration checks the login format, password length (6-255 characters), e-mail, phone and full name. New accounts get the `user` role. Commands in the Адміністрування (Administration) menu run only for accounts with the `admin` role.
- Catalog table:
  - filter by category with the Up/Down keys
  - sort by title, author, year, category or rating by clicking a column header (click again to reverse)
  - page through with the Left/Right keys; the page size follows the window height
  - double-click a row to open the book card with the cover, description, available copies and reviews
- Reservations: pick a branch and a term of 1 to 30 days. A book with no available copies can't be reserved. The profile screen lists the user's latest reservations, returned books and reviews. Right-click there to cancel an active reservation or report a book as lost or damaged.
- Reviews with a 1-5 rating, offered only after the user has returned the book. The book's rating is recalculated as the average of its reviews. Users can edit or delete their own reviews from the profile.
- Admin add/edit/delete for books, categories and branches. A category that still has books, or a book or branch with active reservations, can't be deleted. A reservations dialog lists every reservation, and the admin can set any of the 10 statuses: `reserved`, `ready_for_pickup`, `issued`, `overdue`, `return_requested`, `returned`, `lost`, `damaged`, `cancelled`, `expired`.
- Book covers: a local file path is copied to `books\<id>.<ext>` next to the executable. An http(s) URL is downloaded the first time the cover is shown and cached in `books\cache\`. The cover appears on the book card and on a 3D book model that you rotate with the mouse and zoom with the wheel.
- OpenGL analytics screen with bar or pie charts, in 2D or 3D. Datasets include books per category, copy availability, reservation statuses, review rating distribution, top authors and reservations per branch. Left/Right switches the dataset, mouse drag or W/A/S/D rotates the 3D view, and the wheel zooms.

## Tech stack

- C++, MFC (shared DLL), Unicode build, Visual Studio 2022 (toolset v143)
- MySQL through ADO (`msado15`), the MSDASQL provider and the MySQL ODBC 8.0 Unicode driver
- Fixed-function OpenGL and GLU for the 3D book view and charts. GDI for the catalog table, profile and book card.
- WinINet (`CInternetSession`) for downloading cover images

## Getting started

Prerequisites:

- Windows with Visual Studio 2022 and the "Desktop development with C++" workload, including the MFC component
- MySQL Server on `127.0.0.1:3306`
- MySQL Connector/ODBC 8.0. The connection string uses the driver name `MySQL ODBC 8.0 Unicode Driver`. If you install a newer connector, change that name in `Database::Connect()`. The driver must have the same bitness as your build (x64 or x86).

Steps:

1. **ADO headers.** `pch.h` includes `msado15.tlh`, but `.gitignore` excludes `*.tlh` and `*.tli`, so the generated headers are not in the repository. You can replace that include with
   ```cpp
   #import "C:\Program Files\Common Files\System\ado\msado15.dll" rename("EOF", "ADO_EOF")
   ```
   or put generated `msado15.tlh` and `msado15.tli` next to `pch.h`. The code uses the `ADODB` namespace and `ADO_EOF`.
2. **Database.** The repository has no schema or dump. Create a MySQL database named `Library` with the `utf8mb4` character set, since all data is in Ukrainian. The database name, server, port, user and password are hard-coded in `Database::Connect()` in `Database.cpp`. Edit them to match your MySQL setup. Then create the tables that the queries in `Database.cpp` use:

   | Table | Columns used by the code |
   |---|---|
   | `users` | `id`, `login`, `password`, `role`, `full_name`, `phone`, `email` |
   | `categories` | `id`, `name` |
   | `branches` | `id`, `name`, `address` |
   | `books` | `id`, `category_id`, `title`, `author`, `year`, `rating`, `description`, `cover_path`, `quantity_total`, `quantity_available`, `created_at` |
   | `reservations` | `id`, `user_id`, `book_id`, `branch_id`, `reservation_date`, `return_date`, `status` |
   | `reviews` | `id`, `user_id`, `book_id`, `rating`, `comment`, `created_at` |

   Column requirements:

   - No `INSERT` sets `id`, so every `id` column must be `AUTO_INCREMENT`.
   - `users.phone` and `books.cover_path` must allow NULL.
   - The date columns are written with `NOW()`, so make them `DATETIME`.
   - `books.rating` stores an average, so give it a decimal type.
   - Add unique keys on `users.login`, on `users.email` and on `reviews (user_id, book_id)`. Saving a review uses `INSERT ... ON DUPLICATE KEY UPDATE`, so a second review from the same user replaces the first.
3. Open `LibraryApp.sln`, pick a configuration and platform, then build and run.
4. Register through Бібліотека > Реєстрація (Library > Register). The app has no screen for managing roles. To use the admin commands, set `role` to `admin` for your user in the `users` table, then log out and log in again. Create at least one category before adding books and one branch before making reservations. Reviews open up once Адміністрування > Бронювання sets a reservation to `returned`.

## Project structure

- `Database.h`, `Database.cpp`: singleton data layer with the connection, input validation, all SQL queries and the statistics.
- `LibraryAppView.h`, `LibraryAppView.cpp`: main view. It holds the screens (catalog, profile, book card, 3D view, analytics), the GDI and OpenGL drawing, and mouse and keyboard handling.
- `Filter.h`, `Filter.cpp`: catalog category filter and sort modes.
- `DAuth`, `DReservation`, `DAdd*`, `DEdit*`, `DDelete*`, `DUsersAndReservations` (`.h`/`.cpp`): modal dialogs. They cover login and registration, reservations, books, categories, branches, reviews and the admin reservations list.
- `LibraryApp.rc`, `resource.h`, `res/`: menus, dialogs, icons and the paper texture.

## Author

Kyrylo Radchenko. University project, February 2026.
