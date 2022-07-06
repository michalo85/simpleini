simpleini
=========

Single header library, no external dependences.

Usage
-----

Just include header in your project:
```
#include "simpleini.h"
```
Supported types
---------------
* all integral types, signed and unsigned
* float, double, long double
* std::string
* std::vector of any above types

Adding keys without section
---------------------------
```
simpleini::Config config;

config["bool_key"] = true;
config["int_key"] = 10;
config["float_key"] = 21.01f;
config["string_key"] = "text";
```

Getting key's value
-------------------
```
auto bool_value = config["bool_key"].value<bool>();
auto int_value = config["int_key"].value<int>();
auto float_value = config["float_key"].value<float>();
auto string_value = config["string_key"].value<std::string>();
```
You can pass default value which will be returned for empty key
```
auto value = config["key"].value<int>(-1);
```
Adding keys to section
----------------------
```
simpleini::Config config;
config["section"]["key"] = value;
```

Arrays
------
```
simpleini::Config config;
std::vector<int> vec {{1, 2, 3}};
config["key"] = vec;

...

auto value = config["key"].array<int>();
```

Saving to a file
----------------
```
simpleini::Config config;
config["section"]["key"] = value;
config.save("config.ini");
```

Loading from a file
-------------------
```
auto config = simpleini::Config::load("config.ini");
```
Iterating over Config
---------------------
```
simpleini::Config config;

for (const auto& sectionOrKey : config)
{
    if (sectionOrKey.second.section())
    {
        std::string sectionName = sectionOrKey.first;
        for (const auto& key : sectionOrKey.second)
        {
            auto keyName = key.first;
            auto keyValue = key.second.value<std::string>();
        }
    }
    else
    {
        auto keyName = sectionOrKey.first;
        auto keyValue = sectionOrKey.second.value<std::string>();
    }
}
```


