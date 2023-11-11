
#if !defined(__MC_OBJECT_HPP)
    #define __MC_OBJECT_HPP

    #include <cstdint>
    #include <map>
    #include <optional>
    #include <string>
	#include <ranges>
    #include <vector>

const std::map<const std::string, std::tuple<std::uint16_t, std::uint16_t>> TYPE_MAP = {
    {"U8",   { 0, 1 }},
    { "I8",  { 1, 1 }},
    { "U16", { 2, 2 }},
    { "I16", { 3, 2 }},
    { "U32", { 4, 4 }},
    { "I32", { 5, 4 }},
    { "U64", { 6, 8 }},
    { "I64", { 7, 8 }},
    { "F32", { 8, 4 }},
    { "F64", { 9, 8 }},
};

class McObject {
   public:

    explicit McObject(
        std::string_view name, std::uint32_t address, std::uint8_t ext, std::uint16_t length, const std::string& data_type,
        const std::vector<McObject>& components = std::vector<McObject>()
    ) :
        m_name(name),
        m_address(address),
        m_ext(ext),
        m_length(length),
        m_data_type(data_type),
        m_components(components),
        m_type_index(-1) {
        if (data_type != "") {
			std::string dt_toupper;
			std::transform(data_type.begin(), data_type.end(), dt_toupper.begin(), [](unsigned char c) -> unsigned char { return std::toupper(c); });
            const auto [ti, len] = TYPE_MAP.at(dt_toupper);
            m_type_index         = ti;
            m_length             = len;
        }
    }

    McObject(const McObject& obj)        = default;
    McObject(McObject&& obj)             = default;
    McObject& operator=(const McObject&) = default;
    McObject& operator=(McObject&&)      = default;

    const std::string& get_name() const {
        return m_name;
    }

    void set_name(std::string_view name) {
        m_name = name;
    }

    std::uint32_t get_address() const {
        return m_address;
    }

    void set_address(std::uint32_t address) {
        m_address = address;
    }

    std::uint8_t get_ext() const {
        return m_ext;
    }

    void set_ext(std::uint8_t ext) {
        m_ext = ext;
    }

    const std::string& get_data_type() const {
        return m_data_type;
    }

    void set_data_type(const std::string& value) {
        m_data_type = value;
    }

    std::uint16_t get_length() const {
        return m_length;
    }

    void set_length(std::uint16_t length) {
        m_length = length;
    }

    std::int32_t get_type_index() const {
        return m_type_index;
    }

    const std::vector<McObject>& get_components() const {
        return m_components;
    }

    void add_component(const McObject& obj) {
        m_components.emplace_back(obj);
    }

    bool operator==(const McObject& other) const {
        return (m_name == other.m_name) && (m_address == other.m_address) && (m_ext == other.m_ext) &&
               (m_length == other.m_length) && (m_data_type == other.m_data_type) &&
               (std::equal(m_components.begin(), m_components.end(), other.m_components.begin(), other.m_components.end()));
    }

   private:

    std::string           m_name;
    std::uint32_t         m_address;
    std::uint8_t          m_ext;
    std::uint16_t         m_length;
    std::string           m_data_type;
    std::int16_t          m_type_index;
    std::vector<McObject> m_components{};
};

std::string mc_components_to_string(const std::vector<McObject>& components);

std::string to_string(const McObject& obj) {
    std::stringstream ss;

    ss << "McObject(name='" << obj.get_name() << "', address=" << obj.get_address()
       << ", ext=" << static_cast<std::uint16_t >(obj.get_ext()) << ", data_type='" << obj.get_data_type()
       << "', length=" << obj.get_length() << ", components=[" << mc_components_to_string(obj.get_components()) << "])";
    return ss.str();
}

std::string mc_components_to_string(const std::vector<McObject>& components) {
    std::stringstream ss;

    for (const auto& obj : components) {
        ss << to_string(obj) << ", ";
    }
    return ss.str();
}

#endif  // __MC_OBJECT_HPP
