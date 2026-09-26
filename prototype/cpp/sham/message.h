// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * message.h — stub of Inkscape's MessageContext (status-bar messages).
 *
 * rect-tool.cpp prints live size/ratio status text via setF(). The alpha
 * stores the last formatted message; a future HUD overlay can draw it.
 */
#ifndef SHAM_MESSAGE_H
#define SHAM_MESSAGE_H

#include <cstdarg>
#include <cstdio>
#include <string>

namespace Inkscape {

enum MessageType {
    NORMAL_MESSAGE,
    WARNING_MESSAGE,
    ERROR_MESSAGE,
    IMMEDIATE_MESSAGE,
    CLEAR_MESSAGE,
};

class MessageContext {
public:
    void set(MessageType /*type*/, std::string const &msg) { lastMessage = msg; }

    void setF(MessageType /*type*/, char const *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        char buf[512];
        vsnprintf(buf, sizeof buf, fmt, ap);
        va_end(ap);
        lastMessage = buf;
    }

    void flash(MessageType /*type*/, std::string const &msg) { lastMessage = msg; }
    void clear() { lastMessage.clear(); }

    std::string const &getMessage() const { return lastMessage; }

private:
    std::string lastMessage;
};

} // namespace Inkscape

#endif // SHAM_MESSAGE_H