#include <yaml-cpp/yaml.h>
#include <iostream>
#include <random>
#include <fstream>
#include <twitter.h>
#include <chrono>
#include <thread>
#include <Magick++.h>
#include "database.h"

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

  //twitter::client client(auth);

  database db(config["database"].as<std::string>());

  std::random_device randomDevice;
  std::mt19937 rng(randomDevice());

  for (;;)
  {
    std::cout << "Generating tweet" << std::endl;

    achievement ach = db.getRandomAchievement();
    std::string imageName = db.getRandomImageForGame(ach.gameId);
    std::string imagePath = config["images"].as<std::string>()
      + "/" + imageName;


    try
    {
      Magick::Image image;
      image.read(imagePath);
      image.transform("1600x900");
      image.scale("160x90");
      image.scale("1600x900");
      image.magick("png");
      image.write("output.png");
    } catch (const Magick::WarningCoder& ex)
    {
      // Ok
    }
    












    /*

    // Reload achievements list every time in case it has been updated
    std::vector<std::string> achievements;
    std::ifstream datafile(config["achievements"].as<std::string>());
    if (!datafile.is_open())
    {
      std::cout << "Could not find achievements file." << std::endl;
      return 1;
    }

    std::string line;
    while (getline(datafile, line))
    {
      if (line.back() == '\r')
      {
        line.pop_back();
      }

      achievements.push_back(line);
    }

    std::uniform_int_distribution<int> dist(0, achievements.size() - 1);
    std::string achievement = achievements[dist(rng)];*/

    std::string header;
    if (std::bernoulli_distribution(1.0 / 50.0)(rng))
    {
      header = "YOU GOT A MULTI MOON!";
    } else {
      header = "YOU GOT A MOON!";
    }

    std::string action = header + "\n" + ach.title;
    action.resize(140);

    /*try
    {
      client.updateStatus(action);
    } catch (const twitter::twitter_error& e)
    {
      std::cout << "Twitter error: " << e.what() << std::endl;
    }*/

    std::cout << action << std::endl;
    std::cout << "Waiting" << std::endl;

    std::this_thread::sleep_for(std::chrono::hours(1));

    std::cout << std::endl;
  }
}
