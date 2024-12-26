/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: Minibson.hpp
Open source lines: 656/734 (89.37%)
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
#include <map>
#include <random>
#include <cstring>

namespace minibson {

    // Basic types

    enum bson_node_type : unsigned char {
        double_node = 0x01,
        string_node = 0x02,
        document_node = 0x03,
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
            virtual node* copy() const = 0;
            static node* create(bson_node_type type, const void* const buffer, const size_t count);
    };

    // Value types

    class null : public node {
        public:
            null() { }

            null(const void* const buffer, const size_t count) { }

            void serialize(void* const buffer, const size_t count) const { }

            size_t get_serialized_size() const { 
                return 0; 
            }

            unsigned char get_node_code() const {
                return null_node;
            }

            node* copy() const {
                return new null();
            }
    };

    template<typename T, bson_node_type N>
        class scalar : public node {
            private:
                T value;
            public:
                scalar(const T value) : value(value) { }

                scalar(const void* const buffer, const size_t count) {
                    std::memcpy(reinterpret_cast<unsigned char*>(&value), reinterpret_cast<const unsigned char*>(buffer), sizeof(T));
                };

                void serialize(void* const buffer, const size_t count) const {
                    std::memcpy(reinterpret_cast<unsigned char*>(buffer), reinterpret_cast<const unsigned char*>(&value), sizeof(T));
                }

                size_t get_serialized_size() const {
                    return sizeof(T);
                }

                unsigned char get_node_code() const {
                    return N;
                }

                node* copy() const {
                    return new scalar<T, N>(value);
                }

                const T& get_value() const { return value; }
        };

    class int32 : public scalar<int, int32_node> {
        public:
            int32(const int value) : scalar<int, int32_node>(value) { }

            int32(const void* const buffer, const size_t count) : scalar<int, int32_node>(buffer, count) { };
    };
    
    template<> struct type_converter<int> { enum { node_type_code = int32_node }; typedef int32 node_class; };
    
    class uint64 : public scalar<unsigned long long int, uint64_node> {
    public:
        uint64(const unsigned long long int value) : scalar<unsigned long long int, uint64_node>(value) { }

        uint64(const void* const buffer, const size_t count) : scalar<unsigned long long int, uint64_node>(buffer, count) { };
    };

    template<> struct type_converter<unsigned long long int> { enum { node_type_code = uint64_node }; typedef uint64 node_class; };

    class int64 : public scalar<long long int, int64_node> {
        public:
            int64(const long long int value) : scalar<long long int, int64_node>(value) { }

            int64(const void* const buffer, const size_t count) : scalar<long long int, int64_node>(buffer, count) { };
    };
    
    template<> struct type_converter<long long int> { enum { node_type_code = int64_node }; typedef int64 node_class; };

    class Double : public scalar<double, double_node> {
        public:
            Double(const double value) : scalar<double, double_node>(value) { }

            Double(const void* const buffer, const size_t count) : scalar<double, double_node>(buffer, count) { };
    };
    
    template<> struct type_converter<double> { enum { node_type_code = double_node }; typedef Double node_class; };

    class string : public node {
        private:
            std::string value;
        public:
            string(const std::string& value) : value(value) { }

            string(const void* const buffer, const size_t count) {
                if ( count >= 5 ) {
                    const size_t max = count - sizeof(unsigned int);
                    const size_t actual = *reinterpret_cast<const unsigned int*>(
                        buffer
                    );

                    value.assign(
                        reinterpret_cast<const char*>(buffer) + sizeof(unsigned int),
                        std::min( actual, max ) - 1
                    );
                }
            };

            void serialize(void* const buffer, const size_t count) const {
                unsigned int store_length = value.length() + 1;
                std::memcpy(reinterpret_cast<char*>(buffer), reinterpret_cast<char*>(&store_length), sizeof(unsigned int));
                std::memcpy(reinterpret_cast<char*>(buffer) + sizeof(unsigned int), value.c_str(), value.length());
                *(reinterpret_cast<char*>(buffer) + count - 1) = '\0';
            }

            size_t get_serialized_size() const {
                return sizeof(unsigned int) + value.length() + 1;
            }

            unsigned char get_node_code() const {
                return string_node;
            }

            node* copy() const {
                return new string(value);
            }
            
            const std::string& get_value() const { return value; }
    };
    
    template<> struct type_converter<std::string> { enum { node_type_code = string_node }; typedef string node_class; };

    class boolean : public node {
        private:
            bool value;
        public:
            boolean(const bool value) : value(value) { }

            boolean(const void* const buffer, const size_t count) {
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

            node* copy() const {
                return new boolean(value);
            }

            const bool& get_value() const { return value; }
    };
    
    template<> struct type_converter<bool> { enum { node_type_code = boolean_node }; typedef boolean node_class; };

    class binary : public node {
        public:
            struct buffer {

                buffer() : data(nullptr), length(0), owned(false) {}

                buffer(const buffer& other) : owned(true) { 
                    length = other.length;
                    if (length) {
                        data = new unsigned char[length];
                        std::memcpy(data, other.data, length);
                    } else
                        data = nullptr;
                }

                buffer(void* data, size_t length) : data(data), length(length), owned(false) { }

                ~buffer() {
                    if (owned && data)
                        delete[] reinterpret_cast<unsigned char*>(data);
                }
                
                void* data;
                size_t length;
                bool owned;
            };

        private:
            buffer value;

        public:
            binary(const buffer& buffer) : value(buffer) { }

            binary(const void* const buffer, const size_t count, const bool create = false) : value(NULL, 0) {
                const unsigned char* byte_buffer = reinterpret_cast<const unsigned char*>(buffer);

                if (create) {
                    value.length = count;
                    value.data = new unsigned char[value.length];
                    std::memcpy(value.data, byte_buffer, value.length);
                }
                else {
                    value.length = *reinterpret_cast<const int*>(byte_buffer);
                    value.data = new unsigned char[value.length];
                    std::memcpy(value.data, byte_buffer + 5, value.length);
                }
                
                value.owned = true;
            };

            void serialize(void* const buffer, const size_t count) const {
                unsigned char* byte_buffer = reinterpret_cast<unsigned char*>(buffer);

                std::memcpy(byte_buffer, &(value.length), sizeof(int));
                byte_buffer[4] = 0;
                if (value.length)
                    std::memcpy(byte_buffer + 5, value.data, value.length);
            }

            size_t get_serialized_size() const {
                return 5 + value.length;
            }

            unsigned char get_node_code() const {
                return binary_node;
            }

            node* copy() const {
                return new binary(value.data, value.length, true);
            }

            const buffer& get_value() const { return value; }
    };
    
    template<> struct type_converter< binary::buffer > { enum { node_type_code = binary_node }; typedef binary node_class; };
    
    // Composite types

    class element_list : protected std::map<std::string, node*>, public node {
        public:
            typedef std::map<std::string, node*>::const_iterator const_iterator;
            typedef std::map<std::string, node*>::iterator iterator;

            element_list() { }

            element_list(const element_list& other) { // Copy constructor
                for (const_iterator i = other.cbegin(); i != other.cend(); i++)
                    std::map<std::string, node*>::insert({i->first, i->second->copy()});
            }

            element_list(element_list&& other) noexcept { // Move constructor
                for (const_iterator i = other.begin(); i != other.end(); i++)
                    std::map<std::string, node*>::insert({i->first, i->second});
                other.std::map<std::string, node*>::clear();
            } 

            element_list& operator=(const element_list& other) { // Copy assignment
                element_list::clear();
                for (const_iterator i = other.cbegin(); i != other.cend(); i++)
                    std::map<std::string, node*>::insert({i->first, i->second->copy()});
                return *this;
            }

            element_list& operator=(element_list&& other) noexcept { // Move assignment
                element_list::clear();
                for (iterator i = other.begin(); i != other.end(); i++)
                    std::map<std::string, node*>::insert({i->first, i->second});
                other.std::map<std::string, node*>::clear();
                return *this;
            }

            element_list(const void* const buffer, const size_t count) {
                const unsigned char* byte_buffer = reinterpret_cast<const unsigned char*>(buffer);
                size_t position = 0;

                while (position < count) {
                    bson_node_type type = static_cast<bson_node_type>(byte_buffer[position++]);
                    std::string name(reinterpret_cast<const char*>(byte_buffer + position));
                    node* node = NULL;

                    position += name.length() + 1;
                    node = node::create(type, byte_buffer + position, count - position);

                    if (node != NULL) {
                        position += node->get_serialized_size();
                        (*this)[name] = node;
                    }
                    else
                        break;
                }
            }

            void serialize(void* const buffer, const size_t count) const {
                unsigned char* byte_buffer = reinterpret_cast<unsigned char*>(buffer);
                int position = 0;

                for (const_iterator i = cbegin(); i != cend(); i++) {
                    // Header
                    byte_buffer[position] = i->second->get_node_code();
                    position++;
                    // Key
                    std::strcpy(reinterpret_cast<char*>(byte_buffer + position), i->first.c_str());
                    position += i->first.length() + 1;
                    // Value
                    i->second->serialize(byte_buffer + position, count - position);
                    position += i->second->get_serialized_size();
                }
            }

            size_t get_serialized_size() const {
                size_t result = 0;

                for (const_iterator i = cbegin(); i != cend(); i++)
                    result += 1 + i->first.length() + 1 + i->second->get_serialized_size();

                return result;
            }

            node* copy() const {
                return new element_list(*this);
            }

            iterator begin() {
                return std::map<std::string, node*>::begin();
            }

            iterator end() {
                return std::map<std::string, node*>::end();
            }

            const_iterator cbegin() const {
                return std::map<std::string, node*>::cbegin();
            }

            const_iterator cend() const {
                return std::map<std::string, node*>::cend();
            }

            bool empty() const {
                return std::map<std::string, node*>::empty();
            }

            std::map<std::string, node*>::const_iterator find(const std::string& key) const {
                return std::map<std::string, node*>::find(key);
            }

            std::map<std::string, node*>::iterator find(const std::string& key) {
                return std::map<std::string, node*>::find(key);
            }

            bool contains(const std::string& key) const {
                return (std::map<std::string, node*>::find(key) != cend());
            }
            
            template<typename T>
            bool contains(const std::string& key) const {
                const_iterator position = std::map<std::string, node*>::find(key);
                return (position != cend()) && (position->second->get_node_code() == type_converter<T>::node_type_code);
            }

            element_list& clear() {
                for (iterator i = begin(); i != end(); i++)
                    delete i->second;
                std::map<std::string, node*>::clear();
                return (*this);
            }
            
            int size() const {
                return std::map<std::string, node*>::size();
            }

            iterator erase(const iterator& pos) {
                return std::map<std::string, node*>::erase(pos);
            }

            ~element_list() {
                clear();
            }

    };
   
    class document : public element_list {
        public:
            document() { }

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

            document(const void* const buffer, const size_t count) : element_list(reinterpret_cast<const unsigned char*>(buffer) + 4, *reinterpret_cast<const int*>(buffer) - 4 - 1) { }

            void serialize(void* const buffer, const size_t count) const {
                size_t serialized_size = 4 + element_list::get_serialized_size() + 1;

                if (count >= serialized_size) {
                    unsigned char* byte_buffer = reinterpret_cast<unsigned char*>(buffer);

                    *reinterpret_cast<int*>(buffer) = serialized_size;
                    element_list::serialize(byte_buffer + 4, count - 4 - 1);
                    byte_buffer[serialized_size - 1] = 0;
                }
            }

            size_t get_serialized_size() const {
                return 4 + element_list::get_serialized_size() + 1;
            }

            unsigned char get_node_code() const {
                return document_node;
            }

            node* copy() const {
                return new document(*this);
            }          

            template<typename result_type>
            const result_type get(const std::string& key, const result_type& _default = result_type()) const {
                const bson_node_type node_type_code = static_cast<bson_node_type>(type_converter<result_type>::node_type_code);
                typedef typename type_converter<result_type>::node_class node_class;

                auto it = find(key);
                if ((it != cend()) && ((it->second)->get_node_code() == node_type_code))
                    return reinterpret_cast<const node_class*>(it->second)->get_value();
                else
                    return _default;
            }

            unsigned long long int get_numerical(const std::string& key, const unsigned long long int& _default = 0) const {
                auto it = find(key);
                if (it == cend()) {
                    return _default;
                }
                bson_node_type type = static_cast<bson_node_type>(it->second->get_node_code());
                if (type == bson_node_type::int32_node) {
                    typedef typename type_converter<int>::node_class node_class;
                    return static_cast<unsigned long long int>(reinterpret_cast<const node_class*>(it->second)->get_value());
                } else if (type == bson_node_type::int64_node) {
                    typedef typename type_converter<long long int>::node_class node_class;
                    return static_cast<unsigned long long int>(reinterpret_cast<const node_class*>(it->second)->get_value());
                } else if (type == bson_node_type::uint64_node) {
                    typedef typename type_converter<unsigned long long int>::node_class node_class;
                    return static_cast<unsigned long long int>(reinterpret_cast<const node_class*>(it->second)->get_value());
                }
                
                return _default;
            }

            const binary::buffer& get_binary(const std::string& key, const binary::buffer& _default = binary::buffer()) const {
                auto it = find(key);
                if ((it != cend()) && (it->second->get_node_code() == binary_node)) 
                    return reinterpret_cast<const binary*>(it->second)->get_value();
                else
                    return _default;
            }
            
            const document& get(const std::string& key, const document& _default = document()) const {
                auto it = find(key);
                if ((it != cend()) && (it->second->get_node_code() == document_node))
                    return *reinterpret_cast<const document*>(it->second);
                else
                    return _default;
            }

            const std::string get(const std::string& key, const char* _default = "") const {
                auto it = find(key);
                if ((it != cend()) && (it->second->get_node_code() == string_node))
                    return reinterpret_cast<const string*>(it->second)->get_value();
                else
                    return std::string(_default);
            }

            template<typename value_type>
            document& set(const std::string& key, const value_type& value) {
                typedef typename type_converter<value_type>::node_class node_class;
                
                auto it = find(key);
                if (it != end()) {
                    delete it->second;
                    it->second = new node_class(value);
                } else {
                    std::map<std::string, node*>::insert({key, new node_class(value)});
                }

                return (*this);
            }
            
            document& set(const std::string& key, const char* value) {
                
                auto it = find(key);
                if (it != end()) {
                    delete it->second;
                    it->second = new string(value);
                } else {
                    std::map<std::string, node*>::insert({key, new string(value)});
                }

                return (*this);
            }

            document& set(const std::string& key, const void* buffer, size_t bufferSize) {
                
                auto it = find(key);
                if (it != end()) {
                    delete it->second;
                    it->second = new binary(buffer, bufferSize, true);
                } else {
                    std::map<std::string, node*>::insert({key, new binary(buffer, bufferSize, true)});
                }

                return (*this);
            }
            
            document& set(const std::string& key, const document& value) {
                
                auto it = find(key);
                if (it != end()) {
                    delete it->second;
                    it->second = value.copy();
                } else {
                    std::map<std::string, node*>::insert({key, value.copy()});
                }

                return (*this);
            }
            
            document& set(const std::string& key) {

                auto it = find(key);
                if (it != end()) {
                    delete it->second;
                    it->second = new null();
                } else {
                    std::map<std::string, node*>::insert({key, new null()});
                }

                return (*this);
            }

            document& remove(const std::string& key) {
                auto it = find(key);
                if (it != end()) {
                    delete it->second;
                    std::map<std::string, node*>::erase(it);
                }
                return (*this);
            }

            ~document() { }
    };
    
    class encdocument : public document
    };

    template<> struct type_converter< document > { enum { node_type_code = document_node }; typedef document node_class; };
    
    inline node* node::create(bson_node_type type, const void * const buffer, const size_t count) {
        switch (type) {
            case null_node: return new null();
            case int32_node: return new int32(buffer, count);
            case int64_node: return new int64(buffer, count);
            case uint64_node: return new uint64(buffer, count);
            case double_node: return new Double(buffer, count);
            case document_node: return new document(buffer, count);
            case string_node: return new string(buffer, count);
            case binary_node: return new binary(buffer, count);
            case boolean_node: return new boolean(buffer, count);
            default: return NULL;
        }
    }
}
