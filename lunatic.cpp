#include <yaml-cpp/yaml.h>
#include <iostream>
#include <random>
#include <fstream>
#include <twitter.h>
#include <chrono>
#include <thread>
#include <Magick++.h>
#include <list>
#include <string>
#include <set>
#include <sstream>
#include <hkutil/string.h>
#include <hkutil/database.h>
#include <iomanip>

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    std::cout << "usage: lunatic [configfile]" << std::endl;
    return -1;
  }

  Magick::InitializeMagick(nullptr);

  std::string configfile(argv[1]);
  YAML::Node config = YAML::LoadFile(configfile);

  twitter::auth auth;
  auth.setConsumerKey(config["consumer_key"].as<std::string>());
  auth.setConsumerSecret(config["consumer_secret"].as<std::string>());
  auth.setAccessKey(config["access_key"].as<std::string>());
  auth.setAccessSecret(config["access_secret"].as<std::string>());

  twitter::client client(auth);

  hatkirby::database database(
    config["database"].as<std::string>(),
    hatkirby::dbmode::read);

  std::random_device randomDevice;
  std::mt19937 rng(randomDevice());

  std::set<std::string> blacklist;
  if (config["blacklist"].IsScalar())
  {
    std::ifstream blfile(config["blacklist"].as<std::string>());

    if (!blfile.is_open())
    {
      throw std::invalid_argument("Could not find blacklist");
    }

    std::string line;
    while (getline(blfile, line))
    {
      if (line.back() == '\r')
      {
        line.pop_back();
      }

      blacklist.insert(line);
    }
  }

  for (;;)
  {
    std::cout << "Generating tweet" << std::endl;

    hatkirby::row achRow =
      database.queryFirst(
        "SELECT achievements.achievement_id,                                   \
                achievements.game_id,                                          \
                achievements.title,                                            \
                games.color                                                    \
         FROM achievements                                                     \
         INNER JOIN games ON games.game_id = achievements.game_id              \
         ORDER BY RANDOM()                                                     \
         LIMIT 1");

    int achId = mpark::get<int>(achRow[0]);
    int gameId = mpark::get<int>(achRow[1]);
    std::string achTitle = mpark::get<std::string>(achRow[2]);
    std::string achColor = mpark::get<std::string>(achRow[3]);

    if (blacklist.count(achTitle))
    {
      continue;
    }

    Magick::Image moonColor;
    moonColor.read("res/" + achColor + ".png");

    // Start with the Odyssey text overlay
    Magick::Image overlay;
    try
    {
      overlay.read("res/overlay.png");
    } catch (const Magick::WarningCoder& ex)
    {
      // Ok
    }

    // Add the moon image
    overlay.composite(moonColor, 672, 85, Magick::OverCompositeOp);

    // Add the name of the achievement
    overlay.fontPointsize(54);
    overlay.fillColor("white");
    overlay.font("@" + config["title_font"].as<std::string>());

    std::list<std::string> words =
      hatkirby::split<std::list<std::string>>(
        achTitle,
        " ");

    std::ostringstream wrappingStream;
    std::string curline;
    int lines = 1;

    Magick::TypeMetric metric;
    while (!words.empty())
    {
      std::string temp = curline;

      if (!curline.empty())
      {
        temp += " ";
      }

      temp += words.front();

      overlay.fontTypeMetrics(temp, &metric);

      if (metric.textWidth() > 1200)
      {
        wrappingStream << std::endl;
        curline = words.front();

        lines++;
      } else {
        if (!curline.empty())
        {
          wrappingStream << " ";
        }

        curline = temp;
      }

      wrappingStream << words.front();
      words.pop_front();
    }

    std::string wrapped = wrappingStream.str();

    overlay.annotate(
      wrapped,
      Magick::Geometry(1600, 228, 0, 710),
      Magick::GravityType::NorthGravity);

    // Add the achievement date
    hatkirby::row didRow =
      database.queryFirst(
        "SELECT DATETIME(achieved_at)                                          \
         FROM dids                                                             \
         WHERE achievement_id = ?                                              \
         ORDER BY RANDOM()                                                     \
         LIMIT 1",
        { achId });

    std::tm achievedAt = {};

    std::stringstream dateParser;
    dateParser << mpark::get<std::string>(didRow[0]);
    dateParser >> std::get_time(&achievedAt, "%Y-%m-%d %H:%M:%S");

    std::stringstream dateFormatter;
    dateFormatter << std::put_time(&achievedAt, "%m/%d/%Y");

    std::string didDate = dateFormatter.str();

    overlay.fontTypeMetrics(wrapped, &metric);

    overlay.fontPointsize(20);
    overlay.font("@" + config["date_font"].as<std::string>());
    overlay.annotate(
      didDate,
      Magick::Geometry(1600, 228, 0, 710 + metric.textHeight() * lines - 22),
      Magick::GravityType::NorthGravity);

    // Make a shadow copy
    Magick::Image shadow(overlay);
    shadow.negate();
    shadow.blur(0, 12);

    // Read the game image, using a default if the game has no images
    Magick::Image image;

    hatkirby::row checkRow =
      database.queryFirst(
        "SELECT COUNT(*)                                                       \
         FROM images                                                           \
         WHERE game_id = ?",
        { gameId });

    if (mpark::get<int>(checkRow[0]) > 0)
    {
      hatkirby::row imageRow =
        database.queryFirst(
          "SELECT filename                                                     \
           FROM images                                                         \
           WHERE game_id = ?                                                   \
           ORDER BY RANDOM()                                                   \
           LIMIT 1",
          { gameId });

      std::string imageName = mpark::get<std::string>(imageRow[0]);
      std::string imagePath = config["images"].as<std::string>()
        + "/" + imageName;

      try
      {
        image.read(imagePath);
      } catch (const Magick::WarningCoder& ex)
      {
        // Ok
      }

      // Stretch and pixelate it
      image.transform("1600x900!");
      image.scale("80x45");
      image.scale("1600x900");
    } else {
      try
      {
        image.read("res/default.png");
      } catch (const Magick::WarningCoder& ex)
      {
        // Ok
      }

      image.transform("1600x900!");
    }

    // Add the generated overlay to it
    image.composite(shadow, 0, 0, Magick::OverCompositeOp);
    image.composite(overlay, 0, 0, Magick::OverCompositeOp);

    // Output image
    Magick::Blob outputBlob;
    image.magick("png");

    try
    {
      image.write(&outputBlob);
    } catch (const Magick::WarningCoder& ex)
    {
      // Ok
    }

    std::string header = "YOU GOT A MOON!";
    std::string action = header + "\n" + achTitle;
    action.resize(140);

    std::cout << action << std::endl;

    try
    {
      long media_id = client.uploadMedia(
        "image/png",
        static_cast<const char*>(outputBlob.data()),
        outputBlob.length());

      client.updateStatus(action, {media_id});

      std::cout << "Tweeted!" << std::endl;
    } catch (const twitter::twitter_error& e)
    {
      std::cout << "Twitter error: " << e.what() << std::endl;
    }

    std::cout << "Waiting..." << std::endl;
    std::this_thread::sleep_for(std::chrono::hours(1));

    std::cout << std::endl;
  }
}
