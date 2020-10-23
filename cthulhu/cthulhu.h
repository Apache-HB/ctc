#pragma once

#include <variant>
#include <string>
#include <fmt/core.h>

struct Where {
    int dist = 0;
    int line = 0;
    int col = 0;
    int len = 0;
    const struct Stream* source;
};

struct Stream {
    Stream(void* in, int(*func)(void*), std::string id)
        : stream(in)
        , get(func)
        , name(id)
    { 
        ahead = get(stream);
        pos.source = this;
    }

    int next() { 
        int c = ahead;
        ahead = get(stream);

        buffer.push_back(c);

        pos.dist++;
        if (c == '\n') {
            pos.line++;
            pos.col = 0;
        } else {
            pos.col++;
        }

        return c;
    }

    int peek() {
        return ahead;
    }

    void* stream;
    int(*get)(void*);
    int ahead;
    std::string name;
    Where pos;

    std::string buffer;
};

std::vector<std::string> split(std::string str, std::string tok) {
    std::vector<std::string> out;
    while (str.size()) {
        auto idx = str.find(tok);
        if (idx != std::string::npos) {
            out.push_back(str.substr(0, idx));
            str = str.substr(idx + tok.size());
            if (!str.size())
                out.push_back(str);
        } else {
            out.push_back(str);
            str = "";
        }
    }
    return out;
}

std::string underline(
    Where it, 
    const std::string& msg = "",
    const std::vector<std::string>& notes = {}
) {
    auto in = it.source;
    std::string out = fmt::format("{} -> [{}:{}]\n", in->name, it.line, it.col);

    auto len = fmt::format(" {} ", it.line).length();

    auto begin = std::max(it.dist - it.col, 0);
    auto end = in->buffer.find_first_of("\n\0", it.dist);

    auto line = in->buffer.substr(begin, end);

    out += std::string(len, ' ') + "|\n";
    out += fmt::format(" {} | ", it.line) + line + "\n";
    auto under = std::string(len, ' ') + "| " + std::string(it.col, ' ') + '^' + std::string(it.len - 1, '~');
    out += under;

    if (msg.length() > 0) {
        bool once = true;
        for (auto part : split(msg, "\n")) {
            if (once) {
                once = false;
                out += " " + part + '\n';
            } else {
                out += std::string(under.length() + 1, ' ') + part + '\n';
            }
        }
    }

    for (auto note : notes) {
        out += std::string(len, ' ') + "* " + note + '\n';
    }

    return out;
}
