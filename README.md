# lunatic
lunatic is a Twitter bot that tweets Steam achievements in the style of Super Mario Odyssey moons. It monitors a few specified Steam profiles and records any achievements those profiles attain. The bot then randomly tweets achievements using imagery from Super Mario Odyssey, and with a promotional image from the achievement's game in the background.

Each tweet contains an image of a moon from Super Mario Odyssey. The color of moon is determined on a game-by-game basis; different achievements from the same game will have the same color of moon. Additionally, the date displayed under the achievement title is a date on which one of the tracked players attained that achievement.

It tweets an achievement every hour. It uses [GraphicsMagick](http://www.graphicsmagick.org/) to create the output images, [SQLite](https://www.sqlite.org/index.html) to store information, [YAMLcpp](https://github.com/jbeder/yaml-cpp) to read a configuration file, and my Twitter library [libtwitter++](https://github.com/hatkirby/libtwittercpp) to send the status updates to Twitter.

The scraper is written in Ruby, and its dependencies are enumerated in the `Gemfile`. There are multiple ways to invoke it from the command line:

- `ruby ./scraper.rb CONFIG_FILE init` - creates a database file and loads the schema
- `ruby ./scraper.rb CONFIG_FILE add STEAM_PROFILE` - adds a Steam profile to the list of tracked users and performs a full scrape of their achievements. Note that the profile argument is given in the form of `id/ID` or `profiles/NUMBER`, as the URL that Steam gives a profile is dependent on whether the user has set a profile ID.
- `ruby ./scraper.rb CONFIG_FILE update [STEAM_PROFILE]` - performs a delta scrape of all tracked users (a delta scrape only scrapes achievements from games that are marked as being recently played). Optionally, if a STEAM_PROFILE is provided, the scraper will perform a delta scrape of only that profile.
- `ruby ./scraper.rb CONFIG_FILE full [STEAM_PROFILE]` - performs a full scrape of all tracked users. Optionally, if a STEAM_PROFILE is provided, the scraper will perform a full scrape of only that profile.
- `ruby ./scraper.rb CONFIG_FILE recolor` - randomly reassigns the moon colors used for each game

The moon images in the `res` directory, as well as `res/default.png`, are from Super Mario Odyssey, which is copyrighted by Nintendo. Their inclusion in this repository qualifies as fair use under United States copyright law, as such display does not significantly impede the right of the copyright holder to sell the copyrighted material, is not being used to turn a profit in this context, and presents ideas (parody) that cannot be exhibited otherwise.

The overlay image was created using the font [MOON GET!](https://maxigamer.deviantart.com/art/Font-MOON-GET-A-Super-Mario-Odyssey-Typeface-732847350).

The promotional game images displayed in the backgrounds of tweets are copyrighted by the game's respective owners.

The canonical lunatic handle is [@SteamOdyssey](https://twitter.com/SteamOdyssey). The font used to display the achievement name and achievement date is [Roboto](https://fonts.google.com/specimen/Roboto).
