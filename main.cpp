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