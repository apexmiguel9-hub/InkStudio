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

    // Preference entry handed to ToolBase::set(). rx/ry carry a double;
    // select-tool's "show"/"transform" carry a string.
    class Entry {
    public:
        Entry() = default;
        explicit Entry(std::string n, double v = 0) : name(std::move(n)), value(v) {}
        explicit Entry(std::string n, std::string s) : name(std::move(n)), str(std::move(s)) {}
        std::string getEntryName() const { return name; }
        double getDoubleLimited() const { return value; }
        std::string getString() const { return str; }
    private:
        std::string name;
        double value = 0;
        std::string str;
    };

    // String prefs (select-tool: /tools/select/show -> "outline"/"" default).
    std::string getString(std::string const &path, std::string const &def = "") {
        auto it = strings.find(path);
        return it == strings.end() ? def : it->second;
    }

    double getDoubleLimited(std::string const &path, double def, int min, int max,
                            std::string const & /*unit*/) {
        auto it = doubles.find(path);
        if (it == doubles.end()) return def;
        return std::clamp(it->second, static_cast<double>(min), static_cast<double>(max));
    }

    // --- raw access for tests / future pref UI ---
    void setInt(std::string const &path, int v) { ints[path] = v; }
    void setBool(std::string const &path, bool v) { bools[path] = v; }
    void setString(std::string const &path, std::string const &v) { strings[path] = v; }
    void setDouble(std::string const &path, double v) { doubles[path] = v; }

private:
    Preferences() = default;
    std::unordered_map<std::string, int> ints;
    std::unordered_map<std::string, bool> bools;
    std::unordered_map<std::string, std::string> strings;
    std::unordered_map<std::string, double> doubles;
};

} // namespace Inkscape

#endif // SHAM_PREFS_H