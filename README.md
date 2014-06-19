jsoncpp
=======

A tiny JSON-like support library for c++

Supports

- None (null)
- Boolean values
- Numbers
- Strings
- Arrays
- Objects

Features

- Automatic memory management
- Implicit conversion to/from C++ native types and std::vector/std::map (recursive)
- Conversion to JSON format and parsing of JSON strings
- Automatic creation of array/objects on element access (only from nulls)

example
=======

    JSON::Value v;
    v["a"] = false;
    v["b"] = 3.141592654;
    v["x"][2]["y"] = "Test";

    JSON::Value& vv = v["c"];
    vv[0] = 42;
    vv[2] = "Foo";
    vv[4]["x"] = 10;
    vv[4]["y"] = 20;

