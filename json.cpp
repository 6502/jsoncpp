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

void save(const Value& x, std::vector<unsigned char>& out) {
    switch(x.type) {
    case Value::NONE: out.push_back(0); break;
    case Value::BOOLEAN: out.push_back(x.bval ? 't' : 'f'); break;
    case Value::NUMBER: {
        out.resize(out.size() + 9);
        out[out.size()-9] = 'n';
        memcpy(&out[out.size()-8], &x.num, 8);
        break;
    }
    case Value::STRING: {
        int sz = x.str->size();
        out.resize(out.size() + 5 + sz);
        out[out.size()-5-sz] = 's';
        memcpy(&out[out.size()-4-sz], &sz, 4);
        memcpy(&out[out.size()-sz], x.str->data(), sz);
        break;
    }
    case Value::ARRAY: {
        int sz = x.arr->size();
        out.resize(out.size() + 5);
        out[out.size()-5] = 'a';
        memcpy(&out[out.size()-4], &sz, 4);
        for (int i=0; i<sz; i++) {
            save((*x.arr)[i], out);
        }
        break;
    }
    case Value::OBJECT: {
        int sz = x.obj->size();
        out.resize(out.size() + 5);
        out[out.size()-5] = 'o';
        memcpy(&out[out.size()-4], &sz, 4);
        for (std::map<std::string, Value>::const_iterator i=x.obj->begin(), e=x.obj->end(); i!=e; ++i) {
            const std::string& key = i->first;
            int sz = key.size();
            out.resize(out.size() + sz + 4);
            memcpy(&out[out.size()-sz-4], &sz, 4);
            memcpy(&out[out.size()-sz], key.data(), sz);
            save(i->second, out);
        }
        break;
    }
    default:
        err("Unknown object type");
    }
}

void load(Value& x, const unsigned char *& p) {
    switch(*p++) {
    case 0: x = Value(); break;
    case 't': x = true; break;
    case 'f': x = false; break;
    case 'n': {
        x = 1.;
        memcpy(&x.num, p, 8); p += 8;
        break;
    }
    case 's': {
        int sz; memcpy(&sz, p, 4); p += 4;
        x = std::string((char *)p, (char *)p+sz);
        p += sz;
        break;
    }
    case 'a': {
        int sz; memcpy(&sz, p, 4); p += 4;
        x = Value::Array(sz);
        for (int i=0; i<sz; i++) {
            load(x[i], p);
        }
        break;
    }
    case 'o': {
        int sz; memcpy(&sz, p, 4); p += 4;
        x = Value::Object();
        for (int i=0; i<sz; i++) {
            int ksz; memcpy(&ksz, p, 4); p += 4;
            std::string key((char *)p, (char *)p+ksz);
            p += ksz;
            load(x[key], p);
        }
        break;
    }
    default:
        err("Internal error during load");
    }
}

}
