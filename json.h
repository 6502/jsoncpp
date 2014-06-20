#if !defined(JSON_H_INCLUDED)
#define JSON_H_INCLUDED

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <map>
#include <stdexcept>

namespace JSON {

void err(const std::string& msg);

struct Value {
    enum {
        NONE,
        BOOLEAN,
        NUMBER,
        STRING,
        ARRAY,
        OBJECT
    } type;

    union {
        std::vector<Value> *arr;
        std::map<std::string, Value> *obj;
        std::string *str;
        double num;
        bool bval;
    };

    Value() : type(NONE) {
    }

    void clear() {
        switch(type) {
        case ARRAY: delete arr; break;
        case OBJECT: delete obj; break;
        case STRING: delete str; break;
        case NONE: break;
        case BOOLEAN: break;
        case NUMBER: break;
        }
    }

    ~Value() {
        clear();
    }

    Value& operator=(const Value& other) {
        if (this != &other) {
            clear();
            type = other.type;
            switch(type) {
            case ARRAY: arr = new std::vector<Value>(*other.arr); break;
            case OBJECT: obj = new std::map<std::string, Value>(*other.obj); break;
            case STRING: str = new std::string(*other.str); break;
            case NUMBER: num = other.num;
            case BOOLEAN: bval = other.bval;
            case NONE: break;
            }
        }
        return *this;
    }

    Value& operator=(bool x) {
        clear();
        type = BOOLEAN;
        bval = x;
        return *this;
    }

    operator bool () {
        if (type != BOOLEAN) err("Not a boolean");
        return bval;
    }

    Value& operator=(int x) {
        clear();
        type = NUMBER;
        num = x;
        return *this;
    }

    operator int () {
        if (type != NUMBER) err("Not a number");
        return num;
    }

    Value& operator=(double x) {
        clear();
        type = NUMBER;
        num = x;
        return *this;
    }

    operator double () {
        if (type != NUMBER) err("Not a number");
        return num;
    }

    Value& operator=(const std::string& x) {
        clear();
        type = STRING;
        str = new std::string(x);
        return *this;
    }

    Value& operator=(const char *x) {
        clear();
        type = STRING;
        str = new std::string(x);
        return *this;
    }

    operator std::string () {
        if (type != STRING) err("Not a string");
        return *str;
    }

    template<typename T>
    Value& operator=(const std::vector<T>& x) {
        clear(); type = ARRAY; arr = new std::vector<Value>(x.size());
        for (int i=0,n=x.size(); i<n; i++) {
            (*arr)[i] = x[i];
        }
        return *this;
    }

    template<typename T>
    operator std::vector<T> () {
        if (type != ARRAY) err("Not an array");
        std::vector<T> result;
        for (int i=0,n=arr->size(); i<n; i++) {
            result.push_back((*arr)[i]);
        }
        return result;
    }

    template<typename T>
    Value& operator=(const std::map<std::string, T>& x) {
        clear(); type = OBJECT; obj = new std::map<std::string, Value>();
        for (typename std::map<std::string, T>::const_iterator i=x.begin(),e=x.end(); i!=e; ++i) {
            obj[i->first] = i->second;
        }
        return *this;
    }

    template<typename T>
    operator std::map<std::string, T> () {
        if (type != OBJECT) err("Not an object");
        std::map<std::string, T> result;
        for (std::map<std::string, Value>::iterator i=obj->begin(), e=obj->end(); i!=e; ++i) {
            result[i->first] = i->second;
        }
        return result;
    }

    Value& operator[](int index) const {
        if (type != ARRAY) err("Not an array");
        if (index < 0 || index > int(arr->size())) err("Invalid index");
        return (*arr)[index];
    }

    Value& operator[](int index) {
        if (type == NONE) {
            type = ARRAY;
            arr = new std::vector<Value>();
        }
        if (type != ARRAY) err("Not an array");
        if (index < 0) err("Invalid index");
        if (index >= int(arr->size())) {
            arr->resize(1 + index);
        }
        return (*arr)[index];
    }

    Value& operator[](const std::string& key) {
        if (type == NONE) {
            type = OBJECT;
            obj = new std::map<std::string, Value>();
        }
        if (type != OBJECT) err("Not an object");
        return (*obj)[key];
    }

    Value& operator[](const char *key) {
        if (type == NONE) {
            type = OBJECT;
            obj = new std::map<std::string, Value>();
        }
        if (type != OBJECT) err("Not an object");
        return (*obj)[std::string(key)];
    }

    Value(const Value& other) : type(NONE) {
        *this = other;
    }

    Value(bool x) : type(BOOLEAN), bval(x) {
    }

    Value(int x) : type(NUMBER), num(x) {
    }

    Value(double x) : type(NUMBER), num(x) {
    }

    Value(const std::string& x) : type(STRING), str(new std::string(x)) {
    }

    Value(const char *x) : type(STRING), str(new std::string(x)) {
    }

    static Value Object() {
        Value v;
        v.type = OBJECT;
        v.obj = new std::map<std::string, Value>();
        return v;
    }

    static Value Array(int size=0) {
        Value v;
        v.type = ARRAY;
        v.arr = new std::vector<Value>(size);
        return v;
    }

    int size() const {
        if (type == STRING) return str->size();
        if (type == ARRAY) return arr->size();
        err("No size defined");
        return -1;
    }

    std::map<std::string, Value>& asMap() {
        if (type != OBJECT) err("Not an object");
        return *obj;
    }
};

std::string quote(const std::string&);
std::string parseString(const char *&);

std::string json(const Value&);
Value parse(const char *&);

void save(const Value&, std::vector<unsigned char>&);
void load(Value&, const unsigned char *&);

}

#endif // JSON_H_INCLUDED
