#include "json_db.h"

JsonDB::JsonDB(const char* fname)
    : db_filename_(fname),
    db_file_(NULL)
{
    init();
}
JsonDB::~JsonDB()
{
    close_file();
}

void JsonDB::init()
{
    if (NULL == db_file_)
    {
        fopen_s(&db_file_, db_filename_, "rb");
    }
}

Json::Value JsonDB::get(const char* key, Json::Value def)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (reader.parse(read_file(), db_) &&
        !db_.isNull() &&
        db_.isObject() &&
        !db_[key].isNull())
    {
        return db_[key];
    }
    return def;
}

bool JsonDB::set(const char* key, Json::Value value, bool fast)
{
    std::lock_guard<std::mutex> lock(mtx_);
    bool result = true;

    close_file();
    fopen_s(&db_file_, db_filename_, "wb+");

    db_[key] = value;
    std::string json_con = fast ? fast_writer_.write(db_) : writer_.write(db_);
    fwrite(json_con.c_str(), json_con.size(), 1, db_file_);
    fflush(db_file_);

    if (!reader.parse(read_file(), db_))
    {
        printf("failed to parse %s file: \n%s\n", db_filename_,
            reader.getFormattedErrorMessages().c_str());
        result = false;
    }
    return result;
}

void JsonDB::close_file()
{
    if (NULL != db_file_)
    {
        fclose(db_file_);
    }
    db_file_ = NULL;
}
std::string JsonDB::read_file()
{
    if (NULL == db_file_)
    {
        fopen_s(&db_file_, db_filename_, "rb");
    }
    if (NULL != db_file_)
    {
        fseek(db_file_, 0, SEEK_END);
        long size = ftell(db_file_);

        std::string text;
        char *buffer = new char[size + 1];
        buffer[size] = 0;

        fseek(db_file_, 0, SEEK_SET);
        if (fread(buffer, 1, size, db_file_) > 0)
        {
            text = buffer;
        }
        delete[] buffer;
        return text;
    }
    else
    {
        return std::string("");
    }
}

std::string read_file(const char *path)
{
    FILE *file = NULL;

    fopen_s(&file, path, "rb");
    if (!file)
    {
        return std::string("");
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);

    std::string text;
    char *buffer = new char[size + 1];
    buffer[size] = 0;

    fseek(file, 0, SEEK_SET);
    if (fread(buffer, 1, size, file) > 0)
    {
        text = buffer;
    }
    fclose(file);
    delete[] buffer;
    return text;
}
