require 'json'
require 'nokogiri'
require 'open-uri'
require 'yaml'

config = YAML.load(open(ARGV[0]))
usernames = config["usernames"]

achieves = usernames.map do |username|
  page = Nokogiri::HTML(open("https://steamcommunity.com/id/#{username}/games/?tab=all"))
  script = page.css(".responsive_page_template_content script").text[18..-1]
  data = JSON.parse(script[0..script.index(";\r\n\t\t")-1])
  ids = data.map { |d| d["appid"] }

  ids.map do |id|
    achsp = Nokogiri::HTML(open("https://steamcommunity.com/id/#{username}/stats/#{id}/"))
    achsp.css(".achieveTxt .achieveUnlockTime + h3").map { |d| d.text }
  end
end.flatten

if File.exists?(config["achievements"])
  already = File.read(config["achievements"]).split("\n")
  all_achieves = achieves + already
else
  all_achieves = achieves
end

all_achieves.sort!
all_achieves.uniq!

if config.key? "blacklist"
  blacklist = File.read(config["blacklist"]).split("\n")
  all_achieves.reject! { |l| blacklist.include? l }
end

File.open(config["achievements"], "w") do |f|
  f << all_achieves.join("\n")
end
