#ifndef MESSAGEDAO_H
#define MESSAGEDAO_H

#include "DbConnection.h"
#include <string>
#include <vector>

struct Message {
    int id;
    int userId;
    std::string username;
    std::string content;
    std::string timestamp;
};

class MessageDao {
public:
    MessageDao();
    ~MessageDao();

    bool saveMessage(int userId, const std::string& content);
    std::vector<Message> getRecentMessages(int limit);

private:
    DbConnection connection_;
};

#endif // MESSAGEDAO_H
