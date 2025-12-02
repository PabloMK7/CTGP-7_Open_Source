/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: MiscUtils.hpp
Open source lines: 122/122 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"

namespace CTRPluginFramework {
    namespace MiscUtils {
        bool CopyFile(const std::string &dst, const std::string &ori);
        bool CopyDirectory(const std::string& dst, const std::string& ori, bool recursive = true);

        class Buffer {
        public:
            Buffer() = default;

            explicit Buffer(size_t size)
                : storage_(size ? new Storage(size) : nullptr)
            {}

            explicit Buffer(const void* data, size_t size)
                : Buffer(size)
            {
                std::memcpy(Data(), data, size);
            }

            void Clear() {
                storage_.reset();
            }

            void Set(const void* data, size_t size) {
                *this = Buffer(data, size);
            }

            void Resize(size_t newSize) {
                if (!storage_) {
                    if (newSize == 0) return;
                    storage_ = std::make_unique<Storage>(newSize);
                    return;
                }

                if (newSize == storage_->Size())
                    return;

                if (newSize == 0) {
                    Clear();
                    return;
                }

                // Create new storage and copy min(size, newSize)
                auto newStorage = std::make_unique<Storage>(newSize);
                std::memcpy(newStorage->Data(), storage_->Data(),
                            std::min(storage_->Size(), newSize));
                storage_ = std::move(newStorage);
            }

            size_t Size() const {
                return storage_ ? storage_->Size() : 0;
            }

            bool Empty() const {
                return Size() == 0;
            }

            uint8_t* Data()       { return storage_ ? storage_->Data() : nullptr; }
            const uint8_t* Data() const { return storage_ ? storage_->Data() : nullptr; }

            explicit operator bool() const {
                return storage_ != nullptr;
            }

            Buffer Copy() const {
                Buffer out;

                if (Size()) {
                    out.Resize(Size());
                    std::memcpy(out.Data(), Data(), Size());
                }
                
                return out;
            }

            void CloneTo(Buffer& buf) const {
                buf = Copy();
            }

        private:
            class Storage {
            public:
                Storage() = delete;

                explicit Storage(size_t s)
                    : size(s),
                    data(std::make_unique_for_overwrite<u8[]>(s))
                {}

                Storage(const Storage&)            = delete;
                Storage& operator=(const Storage&) = delete;
                Storage(Storage&&)                 = delete;
                Storage& operator=(Storage&&)      = delete;

                u8* Data() {
                    return data.get();
                }

                size_t Size() const {
                    return size;
                }
            private:
                size_t size;
                std::unique_ptr<u8[]> data;
            };

            std::unique_ptr<Storage> storage_;
        };
    }
}