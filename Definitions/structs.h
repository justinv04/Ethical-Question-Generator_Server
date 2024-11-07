#ifndef STRUCTS_H
#define STRUCTS_H

#include <string>
#include <set>

static const std::set<std::string> Topics = {
    "Anarchism"
    "Act Utilitarianism",
    "Egoism",
    "Kantianism",
    "Natural Law Theory",
    "Nihilism",
    "Social Contract Theory",
    "Subjective Relativism",
    "Rule Utilitarianism",
};

enum Position {
    AGREE = 1,
    NEUTRAL = 0,
    DISAGREE = -1
};

struct User {
    int             id;
    std::string     name,
                    email,
                    passhash,
                    date_joined;
};

struct Question {
    int             id;
    std::string     text,
                    topic;
};

struct Report {
    int             id,
                    user_id;
    std::string     text,
                    topic,
                    date;
    Position        position;

};

#endif