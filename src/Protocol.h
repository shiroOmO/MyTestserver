#pragma once

// JSON Protocol Message Types
// Client -> Server
#define MSG_TYPE_REGISTER "register"
#define MSG_TYPE_LOGIN "login"
#define MSG_TYPE_LOGOUT "logout"
#define MSG_TYPE_CHAT "chat"
#define MSG_TYPE_HEARTBEAT "heartbeat"
#define MSG_TYPE_GET_HISTORY "get_history"

// Server -> Client
#define MSG_TYPE_REGISTER_RESP "register_resp"
#define MSG_TYPE_LOGIN_RESP "login_resp"
#define MSG_TYPE_LOGOUT_RESP "logout_resp"
#define MSG_TYPE_BROADCAST "broadcast"
#define MSG_TYPE_HISTORY "history"
#define MSG_TYPE_ERROR "error"
#define MSG_TYPE_HEARTBEAT_RESP "heartbeat_resp"

// JSON Field Names
#define FIELD_MSG_TYPE "msgType"
#define FIELD_USERNAME "username"
#define FIELD_PASSWORD "password"
#define FIELD_CONTENT "content"
#define FIELD_SUCCESS "success"
#define FIELD_ERROR "error"
#define FIELD_SENDER "sender"
#define FIELD_TIMESTAMP "timestamp"
#define FIELD_SERVER_TIME "serverTime"
#define FIELD_ONLINE_USERS "onlineUsers"
#define FIELD_MESSAGES "messages"
#define FIELD_LIMIT "limit"

