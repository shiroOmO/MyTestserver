#include "MessageDao.h"

#include <algorithm>


MessageDao::MessageDao() : connection_() {
}

MessageDao::~MessageDao() {
}

bool MessageDao::saveMessage(int userId, const std::string &content) {
    if (!connection_.connected()) {
        return false;
    }

    std::string escapedContent = connection_.escape(content);
    std::string sql = "INSERT INTO messages (user_id, content) VALUES (" +
                      std::to_string(userId) + ", '" + escapedContent + "')";

    return connection_.execute(sql);
}

std::vector<Message> MessageDao::getRecentMessages(int limit) {
    std::vector<Message> messages;
    if (!connection_.connected()) {
        return messages;
    }

    std::string sql = "SELECT m.id, m.user_id, u.username, m.content, "
                      "DATE_FORMAT(m.timestamp, '%Y-%m-%d %H:%i:%s') as formatted_time "
                      "FROM messages m JOIN users u ON m.user_id = u.id "
                      "ORDER BY m.timestamp DESC LIMIT " + std::to_string(limit);

    MYSQL_RES* result = connection_.query(sql);
    if (!result) {
        return messages;
    }

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != nullptr) {
        Message msg;
        msg.id = atoi(row[0]);
        msg.userId = atoi(row[1]);
        msg.username = row[2] ? row[2] : "";
        msg.content = row[3] ? row[3] : "";
        msg.timestamp = row[4] ? row[4] : "";
        messages.push_back(msg);
    }

    mysql_free_result(result);
    // Reverse to get oldest first
    std::reverse(messages.begin(), messages.end());
    return messages;
}
