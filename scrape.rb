require 'json'
require 'open-uri'
require 'yaml'

require 'rubygems'
require 'bundler/setup'
Bundler.require :default

MOON_COLORS = [
  :blue,
  :brown,
  :cyan,
  :green,
  :orange,
  :pink,
  :purple,
  :red,
  :star,
  :white,
  :yellow
]

@config = YAML.load(open(ARGV[0]))
db_existed = File.exists?(@config["database"])
db = Sequel.connect("sqlite://" + @config["database"])

if ARGV[1] == "init"
  if db_existed
    raise "Datafile already exists"
  end

  schema = File.read("schema.sql")

  db.run schema

  puts "Initialized datafile"

  exit
end

class Profile < Sequel::Model
  many_to_many :achievements, join_table: :dids
end

class Game < Sequel::Model
  one_to_many :achievements
  one_to_many :images
end

class Achievement < Sequel::Model
  many_to_one :game
  many_to_many :profiles, join_table: :dids
end

class Image < Sequel::Model
  many_to_one :game
end

class Did < Sequel::Model
  many_to_one :profile
  many_to_one :achievement
end

def scrape_profile(profile, full)
  if full
    url = "https://steamcommunity.com/#{profile.profile_path}/games/?tab=all"
  else
    url = "https://steamcommunity.com/#{profile.profile_path}/games/"
  end

  page = Nokogiri::HTML(open(url))
  script = page.css(".responsive_page_template_content script").text[18..-1]
  data = JSON.parse(script[0..script.index(";\r\n\t\t")-1])
  ids = data.map { |d| d["appid"] }

  index = 0
  ids.each do |id|
    index += 1
    puts "#{profile.profile_path} - #{index}/#{ids.count}"

    achsp = Nokogiri::HTML(
      open("https://steamcommunity.com/#{profile.profile_path}/stats/#{id}/"))

    achsp.css(".achieveTxt").each do |node|
      unless node.css(".achieveUnlockTime").empty?
        if Game.where(steam_appid: id).count > 0
          game = Game.where(steam_appid: id).first
        else
          moon_index = Random.rand(MOON_COLORS.size)

          game = Game.new(steam_appid: id, color: MOON_COLORS[moon_index])
          game.save

          storepage = Nokogiri::HTML(
            open("http://store.steampowered.com/app/#{id}"))

          img_id = 0
          storepage.css(".highlight_screenshot_link").each do |node|
            begin
              imagepage = open(node["href"]).read

              img_id += 1
              img_filename = "#{id}-#{img_id}.jpg"
              img_filepath = File.join(@config["images"], img_filename)

              img_file = File.open(img_filepath, "w")
              img_file.write(imagepage)
              img_file.close

              image = Image.new(game: game, filename: img_filename)
              image.save
            rescue OpenURI::HTTPError
              puts "Error downloading an image"
            end

            sleep 2
          end
        end

        title = node.at_css("h3").text

        if game.achievements_dataset.where(title: title).count > 0
          achievement = game.achievements_dataset.where(title: title).first
        else
          achievement = Achievement.new(game: game, title: title)
          achievement.save
        end

        unless Did.where(profile: profile, achievement: achievement).count > 0
          begin
            unlock = DateTime.strptime(
              node.css(".achieveUnlockTime").text.lstrip[9..-1],
              "%b %d, %Y @ %l:%M%P")
          rescue ArgumentError
            unlock = DateTime.strptime(
              node.css(".achieveUnlockTime").text.lstrip[9..-1],
              "%b %d @ %l:%M%P")
          end

          join = Did.new(
            profile: profile,
            achievement: achievement,
            achieved_at: unlock)
          join.save
        end
      end
    end
  end
end

if ARGV[1] == "add"
  userpath = ARGV[2]

  if Profile.where(profile_path: userpath).count > 0
    raise "Profile " + userpath + " already exists"
  end

  profile = Profile.new(profile_path: userpath)
  profile.save

  scrape_profile profile, true
elsif ARGV[1] == "update"
  if ARGV.size == 3
    scrape_profile Profile.where(profile_path: ARGV[2]).first, false
  else
    Profile.all.each do |profile|
      scrape_profile profile, false
    end
  end
elsif ARGV[1] == "full"
  if ARGV.size == 3
    scrape_profile Profile.where(profile_path: ARGV[2]).first, true
  else
    Profile.all.each do |profile|
      scrape_profile profile, true
    end
  end
elsif ARGV[1] == "recolor"
  Game.all.each do |game|
    moon_index = Random.rand(MOON_COLORS.size)

    game.color = MOON_COLORS[moon_index]
    game.save
  end
end
