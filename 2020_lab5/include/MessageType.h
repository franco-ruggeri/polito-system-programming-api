/*
 * Message types for the chat room application protocol.
 *
 * Author: Franco Ruggeri
 */

#pragma once

namespace chat_room {
    enum class MessageType : int {
        error=0,
        login=1,
        logout=2,
        online_users=3,
        done_last_messages=4,
        private_message=5,
        public_message=6,
        quit=7
    };
}
