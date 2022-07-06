#ifndef SIMPLEINI_H
#define SIMPLEINI_H

#include <map>
#include <sstream>
#include <fstream>
#include <limits>
#include <vector>

#define SIMPLEINI_VERSION_MAJOR 1
#define SIMPLEINI_VERSION_MINOR 0
#define SIMPLEINI_VERSION "1.0"

namespace simpleini
{

namespace utils
{
template <typename>
class Raw;
}

namespace traits
{
    template <bool, bool>
    struct logical_and { };

    template <>
    struct logical_and<true, true> : public std::true_type { };

    template <bool, bool>
    struct logical_or { };

    template <>
    struct logical_or<true, true> : public std::true_type { };

    template <>
    struct logical_or<false, true> : public std::true_type { };

    template <>
    struct logical_or<true, false> : public std::true_type { };

    template <typename T>
    using remove_cvref = std::remove_cv<typename std::remove_reference<T>::type>;

    template <typename T>
    using is_integral = std::is_integral<typename remove_cvref<T>::type>;

    template <typename T>
    using is_floating_point = std::is_floating_point<typename remove_cvref<T>::type>;

    template <typename T>
    using is_unsigned = std::is_unsigned<typename remove_cvref<T>::type>;

    template <typename T>
    using is_bool = std::is_same<T, bool>;

    template <typename T>
    using is_text = logical_or<
            std::is_same<typename remove_cvref<T>::type, std::string>::value,
            std::is_same<T, const char*>::value
        >;

    template <typename T>
    using enable_raw = typename std::enable_if<
            std::is_same<T, utils::Raw<void>>::value, T
        >::type;

    template <typename T>
    using enable_intergral_numbers = typename std::enable_if<
            logical_and<
                is_integral<T>::value, !traits::is_bool<T>::value
            >::value, T
        >::type;

    template <typename T>
    using enable_floating_point_numbers = typename std::enable_if<
            is_floating_point<T>::value, T
        >::type;

    template <typename T>
    using enable_text = typename std::enable_if<
            is_text<T>::value, T
        >::type;

    template <typename T>
    using enable_bool = typename std::enable_if<
            is_bool<T>::value, T
        >::type;
}

namespace utils
{
    template <typename = void>
    class Raw
    {
    public:
        Raw(const std::string& raw = {})
            : m_value{raw}
        { }

        inline const std::string& value() const
        {
            return m_value;
        }
    private:
        std::string m_value;
    };

    constexpr bool Encode = true;
    constexpr bool Decode = false;

    template <typename = void>
    std::string transcode_text(std::string text, bool encode)
    {
        const std::map<std::string, std::string> table
        {
            {"\n", "\\n"},
            {"\t", "\\t"},
            {"\"", "\\\""},
            {"\r", "\\r"}
        };

        if (!text.empty())
        {
            for (const auto& kv : table)
            {
                size_t size = encode ? kv.first.size() : kv.second.size();
                size_t pos { 0 };
                while (pos <= text.size() - 1)
                {
                    pos = text.find(encode ? kv.first : kv.second, pos);
                    if (pos == std::string::npos)
                    {
                        break;
                    }
                    text.replace(pos, size, encode ? kv.second : kv.first);
                    pos += encode ? kv.second.size() : kv.first.size();
                }
            }
        }
        if (encode)
        {
            text.insert(0, "\"");
            text.append("\"");
        }
        else
        {
            if (!text.empty() && text[0] == '\"')
            {
                text.replace(0, 1, "");
            }
            if (!text.empty() && text[text.size() - 1] == '\"')
            {
                text.replace(text.size() - 1, 1, "");
            }
        }

        return text;
    }

    template <typename = void>
    std::vector<std::string> splitArray(const std::string& array)
    {
        auto findStringEnd = [](const std::string& str, size_t pos)
        {
            for (size_t i = pos + 1; i < str.size(); ++i)
            {
                if (str[i] == '\"' && str[i - 1] != '\\')
                {
                    return i;
                }
            }
            return std::string::npos;
        };

        auto b = array.find('[');
        auto e = array.rfind(']');
        if (b == std::string::npos || e == std::string::npos || b + 1 >= e)
        {
            return {};
        }
        std::vector<std::string> values;
        size_t pos = b + 1;
        for (; pos < array.size();)
        {
            if (array[pos] == ']')
            {
                break;
            }
            if (array[pos] == ',')
            {
                ++pos;
                continue;
            }

            size_t end = std::string::npos;
            bool text { false };
            if (array[pos] == '\"')
            {
                text = true;
                end = findStringEnd(array, pos);
            }
            else
            {
                end = array.find_first_of(",]", pos);
            }

            if (end == std::string::npos)
            {
                break;
            }

            if (text)
            {
                ++end;
            }
            values.push_back(array.substr(pos, end - pos));
            pos = end;
        }
        return values;
    }

    template <typename T>
    std::string to_raw_value(T value, traits::enable_raw<T>* = nullptr)
    {
        return value.value();
    }

    template <typename T>
    std::string to_raw_value(T value, traits::enable_text<T>* = nullptr)
    {
        return transcode_text(value, utils::Encode);
    }

    template <typename T>
    std::string to_raw_value(T value, traits::enable_bool<T>* = nullptr)
    {
        return value ? "true" : "false";
    }

    template <typename T>
    std::string to_raw_value(T value, traits::enable_intergral_numbers<T>* = nullptr)
    {
        using V = typename std::conditional<traits::is_unsigned<T>::value, unsigned long long, long long>::type;
        std::ostringstream oss;
        oss << static_cast<V>(value);
        return oss.str();
    }

    template <typename T>
    std::string to_raw_value(T value, traits::enable_floating_point_numbers<T>* = nullptr)
    {
        std::ostringstream oss;
        oss.precision(std::numeric_limits<T>::max_digits10);
        oss << value;
        return oss.str();
    }

    template <
        template <typename, typename> class Container,
        typename T,
        typename Alloc = std::allocator<T>
    >
    std::string to_raw_value(const Container<T, Alloc>& container)
    {
        std::ostringstream oss;
        oss << '[';
        bool sep { false };
        for (const auto& v : container)
        {
            if (sep) oss << ',';
            sep = true;
            oss << to_raw_value<T>(v);
        }
        oss << ']';
        return oss.str();
    }

    template <typename T>
    traits::enable_raw<T> from_raw_value(const std::string& raw)
    {
        return T{raw};
    }

    template <typename T>
    traits::enable_bool<T> from_raw_value(const std::string& raw)
    {
        return raw == "true";
    }

    template <typename T>
    traits::enable_intergral_numbers<T> from_raw_value(const std::string& raw)
    {
        using V = typename std::conditional<traits::is_unsigned<T>::value, unsigned long long, long long>::type;
        std::istringstream iss{raw};
        V v {};
        iss >> v;
        return static_cast<T>(v);
    }

    template <typename T>
    traits::enable_floating_point_numbers<T> from_raw_value(const std::string& raw)
    {
        std::istringstream iss{raw};
        T v {};
        iss >> v;
        return v;
    }

    template <typename T>
    traits::enable_text<T> from_raw_value(const std::string& raw)
    {
        return utils::transcode_text(raw, utils::Decode);
    }

    template <typename T>
    std::vector<T> from_raw_array(const std::string& raw)
    {
        std::vector<T> out;
        auto vs = utils::splitArray(raw);
        for (const auto& v : vs)
        {
            out.push_back(from_raw_value<T>(v));
        }
        return out;
    }
}

class Value
{
public:
    template <typename T>
    Value& operator=(T&& v)
    {
        m_raw = utils::to_raw_value(std::forward<T>(v));
        return *this;
    }

    template<typename T>
    T value(const T& defaultValue = T{}) const
    {
        return m_raw.empty() ? defaultValue : utils::from_raw_value<T>(m_raw);
    }

    template <typename T>
    std::vector<T> array() const
    {
        return utils::from_raw_array<T>(m_raw);
    }

    void clear()
    {
        m_raw.clear();
    }

    bool empty() const
    {
        return m_raw.empty();
    }

private:
    std::string m_raw;
};

template<uint T>
class Entry
{
};


template<>
class Entry<1> : public Value
{
public:
    template<typename T>
    Entry<1>& operator=(T&& value)
    {
        (void)Value::operator=(std::forward<T>(value));
        return *this;
    }
};

template<>
class Entry<0> : public Value
{
public:
    using iterator = std::map<std::string, Entry<1>>::iterator;
    using const_iterator = std::map<std::string, Entry<1>>::const_iterator;

    template <typename T>
    Entry<0>& operator=(T&& value)
    {
        m_section = false;
        (void)Value::operator=(std::forward<T>(value));
        return *this;
    }

    void clear()
    {
        Value::clear();
        m_kv.clear();
    }

    bool empty() const
    {
        return m_section ? m_kv.empty() : Value::empty();
    }

    size_t count() const
    {
        return m_section ? m_kv.size() : 1;
    }

    bool section() const
    {
        return m_section;
    }

    Entry<1>& operator[](const std::string& name)
    {
        m_section = true;
        if (m_kv.find(name) == m_kv.end())
        {
            m_kv[name] = Entry<1>{};
        }
        return m_kv[name];
    }

    iterator begin()
    {
        return m_kv.begin();
    }

    iterator end()
    {
        return m_kv.end();
    }

    const_iterator begin() const
    {
        return m_kv.cbegin();
    }

    const_iterator end() const
    {
        return m_kv.cend();
    }

private:
    bool m_section { false };
    std::map<std::string, Entry<1>> m_kv;
};

enum SaveFlags
{
    SaveFlag_Default = 0,
    SaveFlag_SkipEmptyKeys = 0x01
};

template <typename = void>
class Reader
{
public:
    Reader(const std::string& name)
    {
        m_input.open(name, std::ios::in);
    }

    bool getLine(std::string& line)
    {
        return static_cast<bool>(std::getline(m_input, line));
    }

private:
    std::ifstream m_input;
};

template <typename = void>
class Writer
{
public:
    Writer(const std::string& name)
    {
        m_stream.open(name, std::ios::trunc);
    }

    bool is_open() const
    {
        return m_stream.is_open();
    }

    template <typename T>
    std::ostream& operator<<(const T& t)
    {
        return m_stream << t;
    }

private:
    std::ofstream m_stream;
};

template <typename R, typename W>
class ConfigImpl
{
public:
    using iterator = std::map<std::string, Entry<0>>::iterator;
    using const_iterator = std::map<std::string, Entry<0>>::const_iterator;

    template <typename = void>
    Entry<0>& operator[](const std::string& name)
    {
        if (m_entries.find(name) == m_entries.end())
        {
            m_entries[name] = {};
        }
        return m_entries[name];
    }

    size_t count() const
    {
        size_t size { 0 };
        for (const auto& e0 : m_entries)
        {
            size += e0.second.count();
        }
        return size;
    }

    iterator begin()
    {
        return m_entries.begin();
    }

    iterator end()
    {
        return m_entries.end();
    }

    const_iterator begin() const
    {
        return m_entries.cbegin();
    }

    const_iterator end() const
    {
        return m_entries.cend();
    }

    template <typename = void>
    bool save(const std::string& fileName, SaveFlags flags = SaveFlag_Default) const
    {
        W writer{fileName};
        if (!writer.is_open())
        {
            return false;
        }
        for (const auto& e : m_entries)
        {
            if (e.second.section())
            {
                continue;
            }
            if ((flags & SaveFlag_SkipEmptyKeys) && e.second.empty())
            {
                continue;
            }

            writer << e.first << '=' << e.second.value<utils::Raw<>>().value() << '\n';
        }
        for (const auto& e : m_entries)
        {
            if (!e.second.section())
            {
                continue;
            }

            if ((flags & SaveFlag_SkipEmptyKeys) && e.second.empty())
            {
                continue;
            }

            writer << "\n[" << e.first << "]\n";
            for (const auto& c : e.second)
            {
                if ((flags & SaveFlag_SkipEmptyKeys) && c.second.empty())
                {
                    continue;
                }
                writer << c.first << '=' << c.second.value<utils::Raw<>>().value() << '\n';
            }
        }
        return true;
    }

    template<typename = void>
    static ConfigImpl load(const std::string& file)
    {
        R reader{file};

        ConfigImpl config;
        std::string line;
        std::string section;

        while (reader.getLine(line))
        {
            if (line.empty())
            {
                continue;
            }

            if (line[0] == '[')
            {
                size_t e = line.find(']');
                if (e != std::string::npos)
                {
                    section = line.substr(1, e - 1);
                    continue;
                }
            }

            size_t beg = line.find_first_not_of(" \t");
            if (beg == std::string::npos)
            {
                continue;
            }

            if (line[beg] == '#' || line[beg] == ';')
            {
                continue;
            }

            size_t sep = line.find('=', beg);
            if (sep == std::string::npos)
            {
                continue;
            }

            std::string key = line.substr(beg, sep - beg);
            std::string value = line.substr(sep + 1, line.size() - 1);

            if (section.empty())
            {
                config[key] = utils::Raw<>{value};
            }
            else
            {
                config[section][key] = utils::Raw<>{value};
            }
        }

        return config;
    }

private:
    std::map<std::string, Entry<0>> m_entries;
};

using Config = ConfigImpl<Reader<>, Writer<>>;

}
#endif // SIMPLEINI_H
