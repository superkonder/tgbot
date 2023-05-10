#include <tgbot/tgbot.h>
#include <mysql/mysql.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
const std::string HOST = "localhost";
const std::string USERNAME = "root";
const std::string PASSWORD = "password";
const std::string DATABASE = "database_name";
struct ModeratorInfo {
    int yellowCards = 0;
    int redCards = 0;
    int tempCards = 0;
    std::string post;
};
class MySQLConnector {
public:
    MySQLConnector() {
        mysql_init(&mysql);
        connection = mysql_real_connect(&mysql, HOST.c_str(), USERNAME.c_str(), PASSWORD.c_str(), DATABASE.c_str(), 0, nullptr, 0);

        if (connection == nullptr) {
            std::cerr << "Проблемы с подключением к БД." << std::endl;
            exit(1);
        }
        std::cout << "Успешное подключение!" << std::endl;
        std::string query = "CREATE TABLE IF NOT EXISTS staffstats ("
                            "username VARCHAR(255) PRIMARY KEY,"
                            "post VARCHAR(255),"
                            "yellow INT,"
                            "red INT"
                            ");";
        mysql_query(connection, query.c_str());
    }
    ~MySQLConnector() {
        mysql_close(connection);
    }
    void addModerator(const std::string& username, const std::string& post) {
        std::string query = "INSERT INTO staffstats (username, post, yellow, red) VALUES ('" + username + "', '" + post + "', 0, 0);";
        mysql_query(connection, query.c_str());
    }
    void deleteModerator(const std::string& username) {
        std::string query = "DELETE FROM staffstats WHERE username='" + username + "';";
        mysql_query(connection, query.c_str());
    }
    ModeratorInfo getModeratorInfo(const std::string& username) {
        ModeratorInfo info;
        std::string query = "SELECT * FROM staffstats WHERE username='" + username + "';";
        mysql_query(connection, query.c_str());
        MYSQL_RES* result = mysql_store_result(connection);
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row != nullptr) {
            info.post = row[1];
            info.yellowCards = std::stoi(row[2]);
            info.redCards = std::stoi(row[3]);
        }
        mysql_free_result(result);
        return info;
    }
    void addYellowCard(const std::string& username) {
        std::string query = "UPDATE staffstats SET yellow=yellow+1 WHERE username='" + username + "';";
        mysql_query(connection, query.c_str());
    }
    void deleteYellowCard(const std::string& username) {
        std::string query = "UPDATE staffstats SET yellow=yellow-1 WHERE username='" + username + "';";
        mysql_query(connection, query.c_str());
    }
    void addRedCard(const std::string& username) {
        std::string query = "UPDATE staffstats SET red=red+1 WHERE username='" + username + "';";
        mysql_query(connection, query.c_str());
    }
    void deleteRedCard(const std::string& username) {
        std::string query = "UPDATE staffstats SET red=red-1 WHERE username='" + username + "';";
        mysql_query(connection, query.c_str());
    }

private:
    MYSQL mysql;
    MYSQL* connection;
};
class Bot {
public:
    Bot(const std::string& token) {
        bot = new TgBot::Bot(token);
        mysql = new MySQLConnector();

        bot->getEvents().onCommand("addstaff", [this](TgBot::Message::Ptr message) {
            std::vector<std::string> args = splitString(message->text, ' ');

            if (args.size() < 3) {
                bot->getApi().sendMessage(message->chat->id, "Usage: /addstaff <username> <post>");
                return;
            }

            std::string username = args[1];
            std::string post = args[2];

            mysql->addModerator(username, post);

            bot->getApi().sendMessage(message->chat->id, "Moderator " + username + " has been added with post " + post + ".");
        });

        bot->getEvents().onCommand("delstaff", [this](TgBot::Message::Ptr message) {
            std::vector<std::string> args = splitString(message->text, ' ');

            if (args.size() < 2) {
                bot->getApi().sendMessage(message->chat->id, "Usage: /delstaff <username>");
                return;
            }

            std::string username = args[1];

            mysql->deleteModerator(username);

            bot->getApi().sendMessage(message->chat->id, "Moderator " + username + " has been deleted.");
        });

        bot->getEvents().onCommand("addyellow", [this](TgBot::Message::Ptr message) {
            std::vector<std::string> args = splitString(message->text, ' ');

            if (args.size() < 3) {
                bot->getApi().sendMessage(message->chat->id, "Usage: /addyellow <username> <n>");
                return;
            }

            std::string username = args[1];
            int n = std::stoi(args[2]);

            ModeratorInfo info = getOrCreateModeratorInfo(username);

            info.yellowCards += n;

            if (info.yellowCards > maxYellowCards) {
                info.yellowCards = 0;
                info.redCards += 1;
                mysql->addRedCard(username);

                if (info.redCards > maxRedCards) {
                    mysql->deleteModerator(username);
                }
            }

            setModeratorInfo(username, info);

            bot->getApi().sendMessage(message->chat->id, "Moderator " + username + " has received " + std::to_string(n) + " yellow cards.");
        });

        bot->getEvents().onCommand("delyellow", [this](TgBot::Message::Ptr message) {
            std::vector<std::string> args = splitString(message->text, ' ');

            if (args.size() < 2) {
                bot->getApi().sendMessage(message->chat->id, "Usage: /delyellow <username>");
                return;
            }

            std::string username = args[1];

            ModeratorInfo info = getOrCreateModeratorInfo(username);

            info.yellowCards -= 1;

            if (info.yellowCards < 0) {
                info.yellowCards = 0;
            }

            setModeratorInfo(username, info);

            bot->getApi().sendMessage(message->chat->id, "Moderator " + username + " has lost 1 yellow card.");
        });

        bot->getEvents().onCommand("addred", [this](TgBot::Message::Ptr message) {
            std::vector<std::string> args = splitString(message->text, ' ');

            if (args.size() < 3) {
                bot->getApi().sendMessage(message->chat->id, "Usage: /addred <username> <n>");
                return;
            }

            std::string username = args[1];
            int n = std::stoi(args[2]);

            ModeratorInfo info = getOrCreateModeratorInfo(username);

            info.redCards += n;

            if (info.redCards > maxRedCards) {
                mysql->deleteModerator(username);
            }

            setModeratorInfo(username, info);

            bot->getApi().sendMessage(message->chat->id, "Moderator " + username + " has received " + std::to_string(n) + " red cards.");
        });

        bot->getEvents().onCommand("delred", [this](TgBot::Message::Ptr message) {

            std::vector<std::string> args = splitString(message->text, ' ');

            if (args.size() < 2) {
                bot->getApi().sendMessage(message->chat->id, "Usage: /delred <username>");
                return;
            }

            std::string username = args[1];

            ModeratorInfo info = getOrCreateModeratorInfo(username);

            info.redCards -= 1;

            if (info.redCards < 0) {
                info.redCards = 0;
            }

            setModeratorInfo(username, info);

            bot->getApi().sendMessage(message->chat->id, "Moderator " + username + " has lost 1 red card.");
        });

        bot->getEvents().onCommand("addtemp", [this](TgBot::Message::Ptr message) {
            std::vector<std::string> args = splitString(message->text, ' ');

            if (args.size() < 4) {
                bot->getApi().sendMessage(message->chat->id, "Usage: /addtemp <username> <time> <card> <n>");
                return;
            }

            std::string username = args[1];
            int time = std::stoi(args[2]);
            std::string card = args[3];
            int n = std::stoi(args[4]);

            bot->getApi().sendMessage(message->chat->id, "Moderator " + username + " has received " + std::to_string(n) + " " + card + " cards for " + std::to_string(time) + " seconds.");
        });

        bot->getEvents().onCommand("help", [this](TgBot::Message::Ptr message) {
            bot->getApi().sendMessage(message->chat->id, "Available commands:\n"
                                                         "/addstaff <username> <post>\n"
                                                         "/delstaff <username>\n"
                                                         "/addyellow <username> <n>\n"
                                                         "/delyellow <username>\n"
                                                         "/addred <username> <n>\n"
                                                         "/delred <username>\n"
                                                         "/addtemp <username> <time> <card> <n>\n"
                                                         "/help");
        });

        // Запуск бота
        bot->getApi().sendMessage("@", "Bot has been started.");
        bot->getEvents().poll();
    }

    ~Bot() {
        delete bot;
        delete mysql;
    }

private:
    TgBot::Bot* bot;
    MySQLConnector* mysql;

    const int maxYellowCards = 5;
    const int maxRedCards = 3;

    std::map<std::string, ModeratorInfo> moderatorsInfo;

    // Разделение строки на элементы с помощью указанного разделителя (пробел)
    std::vector<std::string> splitString(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(str);

        while (getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }

        return tokens;
    }

    // Получение информации о модераторе из БД или создание новой записи, если он не найден
    ModeratorInfo getOrCreateModeratorInfo(const std::string& username) {
        ModeratorInfo info;

        if (moderatorsInfo.count(username) > 0) {
            info = moderatorsInfo[username];
        } else {
            info = mysql->getModeratorInfo(username);
            moderatorsInfo[username] = info;
        }

        return info;
    }

    void setModeratorInfo(const std::string& username, ModeratorInfo& info) {
        moderatorsInfo[username] = info;

        std::string query = "UPDATE staffstats SET post='" + info.post + "', yellow=" + std::to_string(info.yellowCards) + ", red=" + std::to_string(info.redCards) + " WHERE username='" + username + "';";
        mysql_query(mysql->connection, query.c_str());
    }
};

int main() {
    Bot bot("bot_token");
    return 0;
}