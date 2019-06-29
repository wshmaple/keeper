#ifndef INCLUDE_SRC_JSON_DB_H
#define INCLUDE_SRC_JSON_DB_H

#include <memory>
#include <mutex>
#include <json/json.h>

class JsonDB
{
public:
    JsonDB(const char* fname);
    ~JsonDB();
	void init();

    Json::Value get(const char* key, Json::Value def);
    bool set(const char* key, Json::Value value, bool fast = false);
private:
    void close_file();
    std::string read_file();

    std::mutex mtx_;
    const char* db_filename_;
    FILE* db_file_;
    Json::StyledWriter writer_;
    Json::FastWriter fast_writer_;
    Json::Reader reader;
    Json::Value db_;
};


#endif //INCLUDE_SRC_JSON_DB_H
