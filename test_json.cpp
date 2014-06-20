#include "json.h"

using JSON::Value;

int main() {
    Value v;
    v["x"][2]["y"] = "Test";
    v["a"] = false;
    v["b"] = 3.141592654;
    Value& vv = v["c"];
    vv[0] = 42;
    vv[2] = "Foo";
    vv[4]["x"] = 10;
    vv[4]["y"] = 20;

    std::vector< std::vector<int> > vvi(2);
    vvi[0].push_back(1);
    vvi[0].push_back(2);
    vvi[0].push_back(3);
    vvi[1].push_back(4);
    vvi[1].push_back(5);
    vvi[1].push_back(6);

    v["c"][5] = vvi;

    std::vector< std::vector<int> > vvi_b = v["c"][5];
    v["c"][6] = vvi_b;

    v["c"][6][0][0] = std::string(v["c"][2]) + "\n";

    std::string s = JSON::json(v);
    printf("%s\n", s.c_str());
    const char *p = s.c_str();
    Value v2 = JSON::parse(p);
    printf("%s\n", JSON::json(v2).c_str());

    Value& path = v["path"];
    for (int i=0; i<1000; i++) {
        path[i]["x"] = 10.0 / (i+1);
        path[i]["y"] = 20.0 / (i+1);
    }

    {
        clock_t start = clock();
        for (int i=0; i<1000; i++) {
            v["path2"] = v["path"];
        }
        clock_t stop = clock();
        printf("Assignment = %0.3fms\n", double(stop-start) / CLOCKS_PER_SEC);
    }

    {
        clock_t start = clock();
        for (int i=0; i<1000; i++) {
            std::string s = JSON::json(v["path"]);
        }
        clock_t stop = clock();
        printf("obj -> JSON = %0.3fms\n", double(stop-start) / CLOCKS_PER_SEC);
    }

    {
        std::string s = JSON::json(v["path"]);
        clock_t start = clock();
        for (int i=0; i<1000; i++) {
            const char *p = s.c_str();
            v["path"] = JSON::parse(p);
        }
        clock_t stop = clock();
        printf("JSON -> obj = %0.3fms\n", double(stop-start) / CLOCKS_PER_SEC);
        printf("data = %i bytes\n", int(s.size()));
    }

    {
        clock_t start = clock();
        for (int i=0; i<1000; i++) {
            std::vector<unsigned char> buf;
            JSON::save(v["path"], buf);
        }
        clock_t stop = clock();
        printf("obj -> buf = %0.3fms\n", double(stop-start) / CLOCKS_PER_SEC);
    }

    {
        clock_t start = clock();
        std::vector<unsigned char> buf;
        JSON::save(v["path"], buf);
        for (int i=0; i<1000; i++) {
            Value x;
            const unsigned char *p = &buf[0];
            JSON::load(x, p);
        }
        clock_t stop = clock();
        printf("buf -> obj = %0.3fms\n", double(stop-start) / CLOCKS_PER_SEC);
        printf("data = %i bytes\n", int(buf.size()));
    }
    return 0;
}
