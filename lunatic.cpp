#include <yaml-cpp/yaml.h>
#include <iostream>
#include <random>
#include <fstream>
#include <twitter.h>
#include <chrono>
#include <thread>

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    std::cout << "usage: lunatic [configfile]" << std::endl;
    return -1;
  }

  std::string configfile(argv[1]);
  YAML::Node config = YAML::LoadFile(configfile);

  twitter::auth auth;
  auth.setConsumerKey(config["consumer_key"].as<std::string>());
  auth.setConsumerSecret(config["consumer_secret"].as<std::string>());
  auth.setAccessKey(config["access_key"].as<std::string>());
  auth.setAccessSecret(config["access_secret"].as<std::string>());

  twitter::client client(auth);

  std::random_device randomDevice;
  std::mt19937 rng(randomDevice());

  for (;;)
  {
    std::cout << "Generating tweet" << std::endl;

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
    std::string achievement = achievements[dist(rng)];

    std::string header;
    if (std::bernoulli_distribution(1.0 / 50.0)(rng))
    {
      header = "YOU GOT A MULTI MOON!";
    } else {
      header = "YOU GOT A MOON!";
    }

    std::string action = header + "\n" + achievement;
    action.resize(140);

    try
    {
      client.updateStatus(action);
    } catch (const twitter::twitter_error& e)
    {
      std::cout << "Twitter error: " << e.what() << std::endl;
    }

    std::cout << action << std::endl;
    std::cout << "Waiting" << std::endl;

    std::this_thread::sleep_for(std::chrono::hours(3));

    std::cout << std::endl;
  }
}
