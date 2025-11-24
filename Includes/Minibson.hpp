/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: Minibson.hpp
Open source lines: 879/880 (99.89%)
*****************************************************/

/*
MIT License

Copyright (c) 2017 Emilio Guijarro Cameros

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <string>
#include "string.h"
#include <map>
#include <random>
#include <cstring>
#include "memory"
#include "MiscUtils.hpp"
#include "optional"

namespace minibson {

    using namespace CTRPluginFramework;

    // Basic types

    enum bson_node_type : unsigned char {
        double_node = 0x01,
        string_node = 0x02,
        document_node = 0x03,
        array_node = 0x04,
        binary_node = 0x05,
        boolean_node = 0x08,
        null_node = 0x0A,
        int32_node = 0x10,
        uint64_node = 0x11,
        int64_node = 0x12,
        unknown_node = 0xFF
    };
    
    template<typename T> struct type_converter { };
   
    class node {
        public:
            virtual ~node() { }
            virtual void serialize(void* const buffer, const size_t count) const = 0;
            virtual size_t get_serialized_size() const = 0;
            virtual unsigned char get_node_code() const { return 0; }
            virtual std::unique_ptr<node> copy() const = 0;
            static std::unique_ptr<node> create(bson_node_type type, const void* const buffer, const size_t count);
    };

    // Value types

    class null : public node {
        public:
            null() = default;

            null(const void* const buffer, const size_t count) { }

            void serialize(void* const buffer, const size_t count) const { }

            size_t get_serialized_size() const { 
                return 0; 
            }

            unsigned char get_node_code() const {
                return null_node;
            }

            std::unique_ptr<node> copy() const {
                return std::make_unique<null>();
            }
    };

    template<typename T, bson_node_type N>
        class scalar : public node {
            private:
                T value{};
            public:
                scalar() = default;

                explicit scalar(const T value) : value(value) { }

                explicit scalar(const void* const buffer, const size_t count) {
                    std::memcpy(reinterpret_cast<unsigned char*>(&value), reinterpret_cast<const unsigned char*>(buffer), sizeof(T));
                };

                void serialize(void* const buffer, const size_t count) const {
                    if (count < sizeof(T)) return;
                    
                    std::memcpy(reinterpret_cast<unsigned char*>(buffer), reinterpret_cast<const unsigned char*>(&value), sizeof(T));
                }

                size_t get_serialized_size() const {
                    return sizeof(T);
                }

                unsigned char get_node_code() const {
                    return N;
                }

                std::unique_ptr<node> copy() const {
                    return std::make_unique<scalar<T, N>>(value);
                }

                const T& get_value() const { return value; }
        };

    class int32 : public scalar<int, int32_node> {
        public:
            int32() = default;

            explicit int32(const int value) : scalar<int, int32_node>(value) { }

            explicit int32(const void* const buffer, const size_t count) : scalar<int, int32_node>(buffer, count) { };
    };
    
    template<> struct type_converter<int> { enum { node_type_code = int32_node }; typedef int32 node_class; };
    
    class uint64 : public scalar<unsigned long long int, uint64_node> {
        public:
            uint64() = default;
            
            explicit uint64(const unsigned long long int value) : scalar<unsigned long long int, uint64_node>(value) { }

            explicit uint64(const void* const buffer, const size_t count) : scalar<unsigned long long int, uint64_node>(buffer, count) { };
    };

    template<> struct type_converter<unsigned long long int> { enum { node_type_code = uint64_node }; typedef uint64 node_class; };

    class int64 : public scalar<long long int, int64_node> {
        public:
            int64() = default;

            explicit int64(const long long int value) : scalar<long long int, int64_node>(value) { }

            explicit int64(const void* const buffer, const size_t count) : scalar<long long int, int64_node>(buffer, count) { };
    };
    
    template<> struct type_converter<long long int> { enum { node_type_code = int64_node }; typedef int64 node_class; };

    class Double : public scalar<double, double_node> {
        public:
            Double() = default;

            explicit Double(const double value) : scalar<double, double_node>(value) { }

            explicit Double(const void* const buffer, const size_t count) : scalar<double, double_node>(buffer, count) { };
    };
    
    template<> struct type_converter<double> { enum { node_type_code = double_node }; typedef Double node_class; };

    class string : public node {
        private:
            std::string value;
        public:
            string() = default;

            explicit string(const std::string& value) : value(value) { }

            explicit string(const void* const buffer, const size_t count) {
                if ( count >= 5 ) {
                    const size_t max = count - sizeof(unsigned int);
                    const size_t actual = *reinterpret_cast<const unsigned int*>(
                        buffer
                    );
                    if (actual == 0) return;

                    value.assign(
                        reinterpret_cast<const char*>(buffer) + sizeof(unsigned int),
                        std::min( actual, max ) - 1
                    );
                }
            };

            void serialize(void* const buffer, const size_t count) const override {
                const uint32_t store_length = static_cast<uint32_t>(value.size() + 1);
                const size_t needed = sizeof(store_length) + value.size() + 1;

                if (count < needed) return;

                char* out = static_cast<char*>(buffer);
                std::memcpy(out, &store_length, sizeof(store_length));
                std::memcpy(out + sizeof(store_length), value.data(), value.size());
                out[sizeof(store_length) + value.size()] = '\0';
            }

            size_t get_serialized_size() const {
                return sizeof(unsigned int) + value.length() + 1;
            }

            unsigned char get_node_code() const {
                return string_node;
            }

            std::unique_ptr<node> copy() const {
                return std::make_unique<string>(value);
            }
            
            const std::string& get_value() const { return value; }
    };
    
    template<> struct type_converter<std::string> { enum { node_type_code = string_node }; typedef string node_class; };

    class boolean : public node {
        private:
            bool value{};
        public:
            boolean() = default;

            explicit boolean(const bool value) : value(value) { }

            explicit boolean(const void* const buffer, const size_t count) {
                switch (*reinterpret_cast<const unsigned char*>(buffer)) {
                    case 1: value = true; break;
                    default: value = false; break;
                }
            };

            void serialize(void* const buffer, const size_t count) const {
                *reinterpret_cast<unsigned char*>(buffer) = value ? true : false;
            }

            size_t get_serialized_size() const {
                return 1;
            }

            unsigned char get_node_code() const {
                return boolean_node;
            }

            std::unique_ptr<node> copy() const {
                return std::make_unique<boolean>(value);
            }

            const bool& get_value() const { return value; }
    };
    
    template<> struct type_converter<bool> { enum { node_type_code = boolean_node }; typedef boolean node_class; };

    class binary : public node {
        private:
            MiscUtils::Buffer value;
            u8 subtype = 0;
        public:
            binary() = default;

            explicit binary(const MiscUtils::Buffer& other) : value(other.Copy()) {}

            explicit binary(MiscUtils::Buffer&& other) noexcept : value(std::move(other)) {}

            explicit binary(const void* const buffer, const size_t count) {
                if (count >= 5) {
                    const unsigned char* byte_buffer = reinterpret_cast<const unsigned char*>(buffer);
                    size_t length = *reinterpret_cast<const u32*>(byte_buffer);
                    subtype = byte_buffer[4];

                    if (length <= count - 5)
                        value.Set(byte_buffer + 5, length);
                }                
            };

            void serialize(void* const buffer, const size_t count) const override {
                const uint32_t data_len = static_cast<uint32_t>(value.Size());
                const size_t needed = sizeof(data_len) + 1 + value.Size();

                if (count < needed) return;

                char* out = static_cast<char*>(buffer);

                std::memcpy(out, &data_len, sizeof(data_len));
                out[sizeof(data_len)] = subtype;
                std::memcpy(out + sizeof(data_len) + 1, value.Data(), value.Size());
            }

            size_t get_serialized_size() const {
                return 5 + value.Size();
            }

            unsigned char get_node_code() const {
                return binary_node;
            }

            std::unique_ptr<node> copy() const {
                return std::make_unique<binary>(value);
            }

            const MiscUtils::Buffer& get_value() const { return value; }
    };
    
    template<> struct type_converter< MiscUtils::Buffer > { enum { node_type_code = binary_node }; typedef binary node_class; };
    
    // Composite types

    class document;
    class element_list {
        protected:
            struct transparent_less {
                using is_transparent = void;

                bool operator()(const std::string& a, const std::string& b) const { return a < b; }
                bool operator()(std::string_view a, const std::string& b) const { return a < b; }
                bool operator()(const std::string& a, std::string_view b) const { return a < b; }
                bool operator()(std::string_view a, std::string_view b) const { return a < b; }
            };
            std::map<std::string, std::unique_ptr<node>, transparent_less> map;

        public:
            static constexpr size_t MAX_ENAME_SIZE = 0x100;

            typedef decltype(map)::const_iterator const_iterator;
            typedef decltype(map)::iterator iterator;

            element_list() = default;

            element_list(const element_list& other) { // Copy constructor
                for (const_iterator i = other.cbegin(); i != other.cend(); i++)
                    map.insert({i->first, i->second->copy()});
            }

            element_list(element_list&& other) noexcept
                : map(std::move(other.map))
            {}

            element_list& operator=(const element_list& other) { // Copy assignment
                clear();
                for (const_iterator i = other.map.cbegin(); i != other.map.cend(); i++)
                    map.insert({i->first, i->second->copy()});
                return *this;
            }

            element_list& operator=(element_list&& other) noexcept { // Move assignment
                map.operator=(std::move(other.map));
                return *this;
            }

            element_list(const void* const buffer, const size_t count) {
                const unsigned char* byte_buffer = reinterpret_cast<const unsigned char*>(buffer);
                size_t position = 0;

                while (position < count) {
                    bson_node_type type = static_cast<bson_node_type>(byte_buffer[position++]);
                    if (position >= count) break;
                    size_t name_len = strnlen(reinterpret_cast<const char*>(byte_buffer + position), count - position);
                    if (name_len == count - position || name_len > MAX_ENAME_SIZE) break;

                    std::string name(reinterpret_cast<const char*>(byte_buffer + position), name_len);
                    std::unique_ptr<node> node;

                    position += name.length() + 1;
                    if (position > count) break;
                    node = node::create(type, byte_buffer + position, count - position);

                    if (node != NULL) {
                        position += node->get_serialized_size();
                        map[name] = std::move(node);
                    }
                    else
                        break;
                }
            }

            void serialize(void* const buffer, const size_t count) const {
                unsigned char* byte_buffer = reinterpret_cast<unsigned char*>(buffer);
                size_t position = 0;

                for (const_iterator i = cbegin(); i != cend(); i++) {
                    // Header
                    if (position + 1 > count) return;
                    byte_buffer[position] = i->second->get_node_code();
                    position++;

                    // Key
                    const size_t key_len = i->first.length() + 1;
                    if (position + key_len > count) return;
                    std::memcpy(reinterpret_cast<char*>(byte_buffer + position), i->first.c_str(), key_len);
                    position += i->first.length() + 1;
                    
                    // Value
                    size_t serialized_size = i->second->get_serialized_size();
                    if (position + serialized_size > count) return;
                    i->second->serialize(byte_buffer + position, count - position);
                    position += serialized_size;
                }
            }

            size_t get_serialized_size() const {
                size_t result = 0;

                for (const_iterator i = cbegin(); i != cend(); i++)
                    result += 1 + i->first.length() + 1 + i->second->get_serialized_size();

                return result;
            }

            std::unique_ptr<element_list> copy() const {
                return std::make_unique<element_list>(*this);
            }

            iterator begin() {
                return map.begin();
            }

            iterator end() {
                return map.end();
            }

            const_iterator cbegin() const {
                return map.cbegin();
            }

            const_iterator cend() const {
                return map.cend();
            }

            bool empty() const {
                return map.empty();
            }

            const_iterator find(const std::string_view key) const {
                return map.find(key);
            }

            iterator find(const std::string_view key) {
                return map.find(key);
            }

            bool contains(const std::string_view key) const {
                return (find(key) != cend());
            }
            
            template<typename T>
            bool contains(const std::string_view key) const {
                const_iterator position = find(key);
                return (position != cend()) && (position->second->get_node_code() == type_converter<T>::node_type_code);
            }

            element_list& clear() {
                map.clear();
                return (*this);
            }
            
            size_t size() const {
                return map.size();
            }

            iterator erase(const iterator& pos) {
                return map.erase(pos);
            }

            element_list& remove(const std::string_view key) {
                auto it = find(key);
                if (it != end()) {
                    erase(it);
                }
                return (*this);
            }

            template<typename result_type>
            const result_type get(const std::string_view key, const result_type& _default = result_type()) const {
                const bson_node_type node_type_code = static_cast<bson_node_type>(type_converter<result_type>::node_type_code);
                typedef typename type_converter<result_type>::node_class node_class;

                auto it = find(key);
                if ((it != cend()) && ((it->second)->get_node_code() == node_type_code))
                    return static_cast<const node_class*>(it->second.get())->get_value();
                else
                    return _default;
            }

            unsigned long long int get_numerical(const std::string_view key, const unsigned long long int& _default = 0) const {
                auto it = find(key);
                if (it == cend()) {
                    return _default;
                }

                bson_node_type type = static_cast<bson_node_type>(it->second->get_node_code());
                if (type == bson_node_type::int32_node) {
                    typedef typename type_converter<int>::node_class node_class;
                    return static_cast<unsigned long long int>(static_cast<const node_class*>(it->second.get())->get_value());
                } else if (type == bson_node_type::int64_node) {
                    typedef typename type_converter<long long int>::node_class node_class;
                    return static_cast<unsigned long long int>(static_cast<const node_class*>(it->second.get())->get_value());
                } else if (type == bson_node_type::uint64_node) {
                    typedef typename type_converter<unsigned long long int>::node_class node_class;
                    return static_cast<unsigned long long int>(static_cast<const node_class*>(it->second.get())->get_value());
                }
                
                return _default;
            }

            const MiscUtils::Buffer& get_binary(const std::string_view key) const {
                auto it = find(key);
                if ((it != cend()) && (it->second->get_node_code() == binary_node)) 
                    return static_cast<const binary*>(it->second.get())->get_value();
                else
                    return empty_buffer;
            }

            const document& get(const std::string_view key, const document& _default) const;
            document& get_noconst(const std::string_view key, document& _default);

            const std::string get(const std::string_view key, const char* _default = "") const {
                auto it = find(key);
                if ((it != cend()) && (it->second->get_node_code() == string_node))
                    return static_cast<const string*>(it->second.get())->get_value();
                else
                    return std::string(_default);
            }

            template<typename value_type>
            element_list& set(const std::string_view key, const value_type& value) {
                typedef typename type_converter<value_type>::node_class node_class;
                
                map[std::string(key)] = std::make_unique<node_class>(value);
                return (*this);
            }
            
            element_list& set(const std::string_view key, const char* value) {
                map[std::string(key)] = std::make_unique<string>(value ? value : "");
                return (*this);
            }

            element_list& set(const std::string_view key, const MiscUtils::Buffer& value) {
                map[std::string(key)] = std::make_unique<binary>(value.Copy());
                return (*this);
            }

            element_list& set(const std::string_view key, MiscUtils::Buffer&& value) {
                map[std::string(key)] = std::make_unique<binary>(std::move(value));
                return (*this);
            }
            
            element_list& set(const std::string_view key, const document& value);
            element_list& set(const std::string_view key, document&& value);
            
            element_list& set(const std::string_view key) {
                map[std::string(key)] = std::make_unique<null>();
                return (*this);
            }
        
        protected:
            inline static const MiscUtils::Buffer empty_buffer{};
    };
   
    class document : public element_list, public node {
        public:
            document() = default;

            document(const document& other) : element_list(other) { }

            document(document&& other) noexcept : element_list(std::move(other)) { }

            document& operator=(const document& other) {
                element_list::operator=(other);
                return *this;
            }

            document& operator=(document&& other) noexcept {
                element_list::operator=(std::move(other));
                return *this;
            }

            document& operator=(const element_list& other) {
                element_list::operator=(other);
                return *this;
            }

            document& operator=(element_list&& other) noexcept {
                element_list::operator=(std::move(other));
                return *this;
            }

            document(const void* const buffer, const size_t count) 
                : element_list()
                { 
                    if (count >= 4) {
                        u32 self_size = *reinterpret_cast<const u32*>(buffer);
                        if (self_size >= (4 + 1) && count >= self_size) {
                            *this = element_list(reinterpret_cast<const unsigned char*>(buffer) + 4, self_size - (4 + 1));
                        }
                    }                    
                }

            document(const MiscUtils::Buffer& buffer) 
                : document(buffer.Data(), buffer.Size())
                { }

            void serialize(void* const buffer, const size_t count) const {
                size_t serialized_size = get_serialized_size();

                if (count >= serialized_size) {
                    unsigned char* byte_buffer = reinterpret_cast<unsigned char*>(buffer);

                    *reinterpret_cast<int*>(buffer) = serialized_size;
                    element_list::serialize(byte_buffer + 4, count - (4 + 1));
                    byte_buffer[serialized_size - 1] = 0;
                }
            }

            MiscUtils::Buffer serialize_to_buffer() const {
                MiscUtils::Buffer res(get_serialized_size());
                serialize(res.Data(), res.Size());
                return res;
            }

            size_t get_serialized_size() const {
                return 4 + element_list::get_serialized_size() + 1;
            }

            unsigned char get_node_code() const {
                return document_node;
            }

            std::unique_ptr<node> copy() const {
                return std::make_unique<document>(*this);
            }

            ~document() { }
    };
    template<> struct type_converter< document > { enum { node_type_code = document_node }; typedef document node_class; };

    inline const document& element_list::get(const std::string_view key, const document& _default) const {
        auto it = find(key);
        if (it != cend() && it->second->get_node_code() == document_node)
            return *static_cast<const document*>(it->second.get());
        return _default;
    }

    inline document& element_list::get_noconst(const std::string_view key, document& _default) {
        auto it = find(key);
        if (it != cend() && it->second->get_node_code() == document_node)
            return *static_cast<document*>(it->second.get());
        return _default;
    }

    inline element_list& element_list::set(const std::string_view key, const document& value) {
        map[std::string(key)] = value.copy();
        return *this;
    }

    inline element_list& element_list::set(const std::string_view key, document&& value) {
        map[std::string(key)] = std::make_unique<document>(std::move(value));
        return *this;
    }

    class array : public document
    {
        private:
            bool is_valid_index(ssize_t index) const {
                return index >= 0 && index < document::size();
            }
            std::string to_key(ssize_t index) const {
                return std::to_string(index);
            }
        public:
            array() = default;

            array(const array& other) : document(other) { }

            array(array&& other) noexcept : document(std::move(other)) { }

            array& operator=(const array& other) {
                document::operator=(other);
                return *this;
            }

            array& operator=(array&& other) noexcept {
                document::operator=(std::move(other));
                return *this;
            }

            array& operator=(const document& other) {
                document::operator=(other);
                return *this;
            }

            array& operator=(document&& other) noexcept {
                document::operator=(std::move(other));
                return *this;
            }

            array(const void* const buffer, const size_t count) 
                : document(buffer, count) { }

            array(const MiscUtils::Buffer& buffer) 
                : array(buffer.Data(), buffer.Size()) { }
            
            void serialize(void* const buffer, const size_t count) const {
                document::serialize(buffer, count);
            }

            size_t get_serialized_size() const {
                return document::get_serialized_size();
            }

            unsigned char get_node_code() const {
                return array_node;
            }

            std::unique_ptr<node> copy() const {
                return std::make_unique<array>(*this);
            }

            template<typename result_type>
            const result_type get(ssize_t index, const result_type& _default = result_type()) const {
                if (is_valid_index(index))
                    return document::get<result_type>(to_key(index), _default);
                else
                    return _default;
            }

            unsigned long long int get_numerical(ssize_t index, const unsigned long long int& _default = 0) const {
                if (is_valid_index(index))
                    return document::get_numerical(to_key(index), _default);
                else
                    return _default;
            }

            const MiscUtils::Buffer& get_binary(ssize_t index) const {
                if (is_valid_index(index))
                    return document::get_binary(to_key(index));
                else
                    return document::empty_buffer;
            }
            
            const document& get(ssize_t index, const document& _default = document()) const {
                if (is_valid_index(index))
                    return document::get(to_key(index), _default);
                else
                    return _default;
            }

            document& get_noconst(ssize_t index, document& _default) {
                if (is_valid_index(index))
                    return document::get_noconst(to_key(index), _default);
                else
                    return _default;
            }

            const std::string get(ssize_t index, const char* _default = "") const {
                if (is_valid_index(index))
                    return document::get(to_key(index), _default);
                else
                    return _default;
            }

            template<typename value_type>
            array& set(ssize_t index, const value_type& value) {
                typedef typename type_converter<value_type>::node_class node_class;
                
                if (index == -1) {
                    index = document::size();
                } else {
                    if (!is_valid_index(index)) return (*this);
                }

                document::set<value_type>(to_key(index), value);
                return (*this);              
            }
            
            array& set(ssize_t index, const char* value) {
                if (index == -1) {
                    index = document::size();
                } else {
                    if (!is_valid_index(index)) return (*this);
                }

                document::set(to_key(index), value);
                return (*this);
            }

            array& set(ssize_t index, const MiscUtils::Buffer& value) {
                if (index == -1) {
                    index = document::size();
                } else {
                    if (!is_valid_index(index)) return (*this);
                }

                document::set(to_key(index), value);
                return (*this);
            }

            array& set(ssize_t index, MiscUtils::Buffer&& value) {
                if (index == -1) {
                    index = document::size();
                } else {
                    if (!is_valid_index(index)) return (*this);
                }

                document::set(to_key(index), std::move(value));
                return (*this);
            }
            
            array& set(ssize_t index, const document& value) {
                if (index == -1) {
                    index = document::size();
                } else {
                    if (!is_valid_index(index)) return (*this);
                }

                document::set(to_key(index), value);
                return (*this);
            }

            array& set(ssize_t index, document&& value) {
                if (index == -1) {
                    index = document::size();
                } else {
                    if (!is_valid_index(index)) return (*this);
                }

                document::set(to_key(index), std::move(value));
                return (*this);
            }
            
            array& set(ssize_t index) {
                if (index == -1) {
                    index = document::size();
                } else {
                    if (!is_valid_index(index)) return (*this);
                }

                document::set(to_key(index));
                return (*this);
            }
    };
    template<> struct type_converter< array > { enum { node_type_code = array_node }; typedef array node_class; };
    
    
    namespace crypto {
        size_t get_serialized_size(const document& document);
        std::optional<document> decrypt(const void* data, const size_t size);
        std::optional<document> decrypt(const MiscUtils::Buffer& buffer);  
        void encrypt(const document& document, void* outData, const size_t outSize);
        MiscUtils::Buffer encrypt(const document& document);
    }
    
    inline std::unique_ptr<node> node::create(bson_node_type type, const void * const buffer, const size_t count) {
        switch (type) {
            case null_node: return std::make_unique<null>();
            case int32_node: return std::make_unique<int32>(buffer, count);
            case int64_node: return std::make_unique<int64>(buffer, count);
            case uint64_node: return std::make_unique<uint64>(buffer, count);
            case double_node: return std::make_unique<Double>(buffer, count);
            case document_node: return std::make_unique<document>(buffer, count);
            case array_node: return std::make_unique<array>(buffer, count);
            case string_node: return std::make_unique<string>(buffer, count);
            case binary_node: return std::make_unique<binary>(buffer, count);
            case boolean_node: return std::make_unique<boolean>(buffer, count);
            default: return std::make_unique<null>();
        }
    }
}
