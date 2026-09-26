// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * prefs.h — stub of Inkscape::Preferences singleton.
 *
 * Only the entries rect-tool.cpp reads:
 *   /options/dragtolerance/value -> checkDragMoved tolerance
 *   /tools/shapes/selcue, /tools/shapes/gradientdrag -> selection cue UI
 * (rx/ry are set through sp_event_context_read -> ToolBase::set; sham no-op).
 */
#ifndef SHAM_PREFS_H
#define SHAM_PREFS_H

#include <string>
#include <unordered_map>

namespace Inkscape {

class Preferences {
public:
    static Preferences *get() {
        static Preferences instance;
        return &instance;
    }

    // Tool drag tolerance (touch): Inkscape's desktop default is 0; for the
    // alpha we seed 6px so finger micro-jitter does not create 1-2px rects.
    // Tunable later via the real preferences system.
    int getIntLimited(std::string const &path, int def, int min, int max) {
        auto it = ints.find(path);
        if (it == ints.end()) {
            if (path == "/options/dragtolerance/value") return 6; // touch UX
            return def;
        }
        return std::clamp(it->second, min, max);
    }

    bool getBool(std::string const &path, bool def = false) {
        auto it = bools.find(path);
        return it == bools.end() ? def : it->second;
    }

    // Preference entry handed to ToolBase::set(). Only rx/ry matter for the
    // rect tool, and the alpha's sp_event_context_read is a no-op with 0.
    class Entry {
    public:
        Entry() = default;
        explicit Entry(std::string n, double v = 0) : name(std::move(n)), value(v) {}
        std::string getEntryName() const { return name; }
        double getDoubleLimited() const { return value; }
    private:
        std::string name;
        double value = 0;
    };

    // --- raw access for tests / future pref UI ---
    void setInt(std::string const &path, int v) { ints[path] = v; }
    void setBool(std::string const &path, bool v) { bools[path] = v; }

private:
    Preferences() = default;
    std::unordered_map<std::string, int> ints;
    std::unordered_map<std::string, bool> bools;
};

} // namespace Inkscape

#endif // SHAM_PREFS_H