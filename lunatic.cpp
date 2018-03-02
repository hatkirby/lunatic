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

    Magick::Image overlay;
    overlay.read("res/overlay.png");

    Magick::Image moonColor;
    moonColor.read("res/" + ach.color + ".png");

    try
    {
      // Start with the game image
      Magick::Image image;
      image.read(imagePath);

      // Stretch and pixelate it
      image.transform("1600x900!");
      image.scale("80x45");
      image.scale("1600x900");

      // Add the text and moon image from Odyssey
      image.composite(overlay, 0, 0, Magick::OverCompositeOp);
      image.composite(moonColor, 672, 85, Magick::OverCompositeOp);

      // Add the name of the achievement
      image.fontPointsize(36);
      image.fillColor("white");
      image.font("@" + config["font"].as<std::string>());
      image.annotate(
        ach.title,
        Magick::Geometry(0, 0, 0, 672),
        Magick::GravityType::NorthGravity);

      // Output for debug
      image.magick("png");
      image.write("output.png");
    } catch (const Magick::WarningCoder& ex)
    {
      // Ok
    }

    std::string header = "YOU GOT A MOON!";

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
