#ifndef DATABASE_H_75C3CE0F
#define DATABASE_H_75C3CE0F

#include <string>

struct sqlite3;

struct achievement {
  int achievementId;
  int gameId;
  std::string title;
  std::string color;
};

struct did {
  int profileId;
  std::string date;
};

class database {
public:

  // Constructor

  explicit database(std::string path);

  // Disable copying

  database(const database& other) = delete;
  database& operator=(const database& other) = delete;

  // Move constructor and move assignment

  database(database&& other);
  database& operator=(database&& other);

  // Swap

  friend void swap(database& first, database& second);

  // Destructor

  ~database();

  // Accessors

  achievement getRandomAchievement() const;

  std::string getRandomImageForGame(int gameId) const;

  did getRandomDidForAchievement(int achievementId) const;

private:

  database() = default;

  sqlite3* ppdb_ = nullptr;
};

#endif /* end of include guard: DATABASE_H_75C3CE0F */
