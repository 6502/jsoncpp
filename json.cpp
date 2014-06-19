#include "json.h"

#include <time.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <map>
#include <stdexcept>

namespace JSON {

void err(const std::string& msg) {
    throw std::runtime_error(msg);
}

std::string quote(const std::string& x) {
    std::string result = "\"";
    for (int i=0,n=x.size(); i<n; i++) {
        switch(x[i]) {
        case '\\': result += "\\\\"; break;
        case '\n': result += "\\n"; break;
        case '\t': result += "\\t"; break;
        case '\r': result += "\\r"; break;
        case '\b': result += "\\b"; break;
        case '\v': result += "\\v"; break;
        case '\"': result += "\\\""; break;
        default:
            if (x[i] >= ' ' && x[i] < 127) {
                result += x[i];
            } else {
                char buf[5];
                sprintf(buf, "\\x%02x", x[i] & 0xFF);
                result += buf;
            }
        }
    }
    return result + "\"";
}

std::string json(const Value& x) {
    switch(x.type) {
    case Value::NONE: return "null";
    case Value::BOOLEAN: return x.bval ? "true" : "false";
    case Value::NUMBER: {
        char buf[200]; sprintf(buf, "%.18g", x.num);
        return buf;
    }
    case Value::STRING: return quote(*x.str);
    case Value::ARRAY: {
        std::string result = "[";
        for (int i=0,n=x.arr->size(); i<n; i++) {
            if (i) result += ",";
            result += json(x[i]);
        }
        return result + "]";
    }
    case Value::OBJECT: {
        std::string result = "{";
        for (std::map<std::string, Value>::const_iterator i=x.obj->begin(),e=x.obj->end(); i!=e; ++i) {
            if (i != x.obj->begin()) result += ",";
            result += quote(i->first) + ":" + json(i->second);
        }
        return result + "}";
    }
    }
    err("Unknown type");
    return "";
}

std::string parseString(const char *& s) {
    while (*s && isspace((unsigned char)*s)) s++;
    if (*s != '\"') err("Double quote expected");
    s++;
    std::string result;
    while (*s && *s != '\"') {
        if (*s == '\\') {
            s++;
            switch(*s) {
            case '\\': result += "\\"; s++; break;
            case 'n': result += "\n"; s++; break;
            case 'r': result += "\r"; s++; break;
            case 't': result += "\t"; s++; break;
            case 'b': result += "\b"; s++; break;
            case 'v': result += "\v"; s++; break;
            case 'x': {
                int x1 = -1, x2 = -1;
                if (s[1] >= '0' && s[1] <= '9') {
                    x1 = s[1] - '0';
                } else if (s[1] >= 'a' && s[1] <= 'f') {
                    x1 = s[1] - 'a' + 10;
                } else if (s[1] >= 'A' && s[1] <= 'F') {
                    x1 = s[1] - 'A' + 10;
                } else err("Invalid escape sequence");
                if (s[2] >= '0' && s[2] <= '9') {
                    x2 = s[2] - '0';
                } else if (s[2] >= 'a' && s[2] <= 'f') {
                    x2 = s[2] - 'a' + 10;
                } else if (s[2] >= 'A' && s[2] <= 'F') {
                    x2 = s[2] - 'A' + 10;
                } else err("Invalid escape sequence");
                result += char(x1*16 + x2);
                s += 3;
                break;
            }
            default: err("Invalid escape sequence");
            }
        } else {
            result += *s++;
        }
    }
    if (*s != '\"') err("Double quote expected");
    s++;
    return result;
}

Value parse(const char *& s) {
    while (*s && isspace((unsigned char)*s)) s++;
    if (*s == '\"') {
        return parseString(s);
    } else if (*s == '[') {
        s++;
        Value v = Value::Array();
        while (*s && isspace((unsigned char)*s)) s++;
        while (*s && *s != ']') {
            v.arr->push_back(parse(s));
            while (*s && isspace((unsigned char)*s)) s++;
            if (*s == ',') {
                s++;
                while (*s && isspace((unsigned char)*s)) s++;
            }
        }
        if (*s == ']') {
            s++;
            return v;
        } else {
            err("']' expected");
        }
    } else if (*s == '{') {
        s++;
        Value v = Value::Object();
        while (*s && isspace((unsigned char)*s)) s++;
        while (*s && *s != '}') {
            std::string key = parseString(s);
            while (*s && isspace((unsigned char)*s)) s++;
            if (*s != ':') err("':' expected");
            s++;
            (*v.obj)[key] = parse(s);
            while (*s && isspace((unsigned char)*s)) s++;
            if (*s == ',') {
                s++;
                while (*s && isspace((unsigned char)*s)) s++;
            }
        }
        if (*s == '}') {
            s++;
            return v;
        } else {
            err("'}' expected");
        }
    } else if (*s == '-' || *s == '+' || (*s>='0' && *s <= '9') || *s == '.') {
        char *s0 = NULL;
        double x = strtod(s, &s0);
        s = s0;
        return x;
    } else if (strncmp(s, "null", 4) == 0) {
        s += 4;
        return Value();
    } else if (strncmp(s, "true", 4) == 0) {
        s += 4;
        return Value(true);
    } else if (strncmp(s, "false", 5) == 0) {
        s += 5;
        return Value(false);
    } else {
        err("Invalid string");
    }
    return Value();
}

}
