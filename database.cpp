#include "database.h"
#include <sqlite3.h>
#include <stdexcept>
#include <sstream>
#include <iomanip>

database::database(std::string path)
{
  if (sqlite3_open_v2(
    path.c_str(),
    &ppdb_,
    SQLITE_OPEN_READONLY,
    NULL) != SQLITE_OK)
  {
    // We still have to free the resources allocated. In the event that
    // allocation failed, ppdb will be null and sqlite3_close_v2 will just
    // ignore it.
    std::string errmsg(sqlite3_errmsg(ppdb_));
    sqlite3_close_v2(ppdb_);

    throw std::logic_error(errmsg);
  }
}

database::database(database&& other) : database()
{
  swap(*this, other);
}

database& database::operator=(database&& other)
{
  swap(*this, other);

  return *this;
}

void swap(database& first, database& second)
{
  std::swap(first.ppdb_, second.ppdb_);
}

database::~database()
{
  sqlite3_close_v2(ppdb_);
}

achievement database::getRandomAchievement() const
{
  std::string queryString = "SELECT achievements.achievement_id, achievements.game_id, achievements.title, games.color FROM achievements INNER JOIN games ON games.game_id = achievements.game_id ORDER BY RANDOM() LIMIT 1";

  sqlite3_stmt* ppstmt;
  if (sqlite3_prepare_v2(
    ppdb_,
    queryString.c_str(),
    queryString.length(),
    &ppstmt,
    NULL) != SQLITE_OK)
  {
    std::string errorMsg = sqlite3_errmsg(ppdb_);
    sqlite3_finalize(ppstmt);

    throw std::logic_error(errorMsg);
  }

  if (sqlite3_step(ppstmt) != SQLITE_ROW)
  {
    std::string errorMsg = sqlite3_errmsg(ppdb_);
    sqlite3_finalize(ppstmt);

    throw std::logic_error(errorMsg);
  }

  achievement result;

  result.achievementId = sqlite3_column_int(ppstmt, 0);
  result.gameId = sqlite3_column_int(ppstmt, 1);
  result.title = reinterpret_cast<const char*>(sqlite3_column_text(ppstmt, 2));
  result.color = reinterpret_cast<const char*>(sqlite3_column_text(ppstmt, 3));

  sqlite3_finalize(ppstmt);

  return result;
}

bool database::doesGameHaveImages(int gameId) const
{
  std::string queryString = "SELECT COUNT(*) FROM images WHERE game_id = ? ORDER BY RANDOM() LIMIT 1";

  sqlite3_stmt* ppstmt;
  if (sqlite3_prepare_v2(
    ppdb_,
    queryString.c_str(),
    queryString.length(),
    &ppstmt,
    NULL) != SQLITE_OK)
  {
    std::string errorMsg = sqlite3_errmsg(ppdb_);
    sqlite3_finalize(ppstmt);

    throw std::logic_error(errorMsg);
  }

  if (sqlite3_bind_int(ppstmt, 1, gameId) != SQLITE_OK)
  {
    std::string errorMsg = sqlite3_errmsg(ppdb_);
    sqlite3_finalize(ppstmt);

    throw std::logic_error(errorMsg);
  }

  if (sqlite3_step(ppstmt) != SQLITE_ROW)
  {
    std::string errorMsg = sqlite3_errmsg(ppdb_);
    sqlite3_finalize(ppstmt);

    throw std::logic_error(errorMsg);
  }

  int result = sqlite3_column_int(ppstmt, 0);

  sqlite3_finalize(ppstmt);

  return (result > 0);
}

std::string database::getRandomImageForGame(int gameId) const
{
  std::string queryString = "SELECT filename FROM images WHERE game_id = ? ORDER BY RANDOM() LIMIT 1";

  sqlite3_stmt* ppstmt;
  if (sqlite3_prepare_v2(
    ppdb_,
    queryString.c_str(),
    queryString.length(),
    &ppstmt,
    NULL) != SQLITE_OK)
  {
    std::string errorMsg = sqlite3_errmsg(ppdb_);
    sqlite3_finalize(ppstmt);

    throw std::logic_error(errorMsg);
  }

  if (sqlite3_bind_int(ppstmt, 1, gameId) != SQLITE_OK)
  {
    std::string errorMsg = sqlite3_errmsg(ppdb_);
    sqlite3_finalize(ppstmt);

    throw std::logic_error(errorMsg);
  }

  if (sqlite3_step(ppstmt) != SQLITE_ROW)
  {
    std::string errorMsg = sqlite3_errmsg(ppdb_);
    sqlite3_finalize(ppstmt);

    throw std::logic_error(errorMsg);
  }

  std::string result = reinterpret_cast<const char*>(sqlite3_column_text(ppstmt, 0));

  sqlite3_finalize(ppstmt);

  return result;
}

did database::getRandomDidForAchievement(int achievementId) const
{
  std::string queryString = "SELECT profile_id, DATETIME(achieved_at) FROM dids WHERE achievement_id = ? ORDER BY RANDOM() LIMIT 1";

  sqlite3_stmt* ppstmt;
  if (sqlite3_prepare_v2(
    ppdb_,
    queryString.c_str(),
    queryString.length(),
    &ppstmt,
    NULL) != SQLITE_OK)
  {
    std::string errorMsg = sqlite3_errmsg(ppdb_);
    sqlite3_finalize(ppstmt);

    throw std::logic_error(errorMsg);
  }

  if (sqlite3_bind_int(ppstmt, 1, achievementId) != SQLITE_OK)
  {
    std::string errorMsg = sqlite3_errmsg(ppdb_);
    sqlite3_finalize(ppstmt);

    throw std::logic_error(errorMsg);
  }

  if (sqlite3_step(ppstmt) != SQLITE_ROW)
  {
    std::string errorMsg = sqlite3_errmsg(ppdb_);
    sqlite3_finalize(ppstmt);

    throw std::logic_error(errorMsg);
  }

  did result;
  result.profileId = sqlite3_column_int(ppstmt, 0);

  std::tm achievedAt = {};

  std::stringstream dateParser;
  dateParser << reinterpret_cast<const char*>(sqlite3_column_text(ppstmt, 1));
  dateParser >> std::get_time(&achievedAt, "%Y-%m-%d %H:%M:%S");

  std::stringstream dateFormatter;
  dateFormatter << std::put_time(&achievedAt, "%m/%d/%Y");
  result.date = dateFormatter.str();

  sqlite3_finalize(ppstmt);

  return result;
}
