CREATE TABLE `profiles` (
  `profile_id` INTEGER PRIMARY KEY,
  `profile_path` VARCHAR(255) NOT NULL
);

CREATE UNIQUE INDEX `profile_by_path` ON `profiles`(`profile_path`);

CREATE TABLE `games` (
  `game_id` INTEGER PRIMARY KEY,
  `steam_appid` INTEGER NOT NULL,
  `moon_image` VARCHAR(255) NOT NULL
);

CREATE UNIQUE INDEX `game_by_appid` ON `games`(`steam_appid`);

CREATE TABLE `achievements` (
  `achievement_id` INTEGER PRIMARY KEY,
  `game_id` INTEGER NOT NULL,
  `title` VARCHAR(255) NOT NULL
);

CREATE TABLE `dids` (
  `profile_id` INTEGER NOT NULL,
  `achievement_id` INTEGER NOT NULL,
  `achieved_at` DATETIME NOT NULL
);

CREATE TABLE `images` (
  `image_id` INTEGER PRIMARY KEY,
  `game_id` INTEGER NOT NULL,
  `filename` VARCHAR(255) NOT NULL
);
