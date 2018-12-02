#pragma once
#include <assert.h>
#include <memory>
#include <iomanip>
#include <sstream>
#include <string.h>
#include <string>
#include "Cleanable.h"

// This is mimic of RocksDB Slice.
class Slice {
protected:
	const char* data_;
	size_t size_;
public:
	Slice() : data_(""), size_(0) {}
	explicit Slice(const char* bytes, size_t n) : data_(bytes), size_(n) {}
	Slice(const char* str) : data_(str) {
		this->size_ = (str == nullptr) ? 0 : strlen(str);
	}
	explicit Slice(const std::string& s) : data_(s.data()), size_(s.size()) {}

	size_t size() const { return this->size_; }
	const char* data() const {return this->data_; }
	char operator[](size_t n) const  {
		assert(n < size());
		return data_[n];
	}
 
	int compare(const char* data, size_t size) const {
		assert(this->data_ != nullptr && data != nullptr);
		if (this->size_ != size) {
			return this->size_ < size ? -1 : 1;
		}
		return memcmp(this->data_, data, size);
	}

	int compare(const Slice& s) const {
		return compare(s.data(), s.size());
	}

	bool operator==(const Slice& k) const
	{
		return compare(k) == 0;
	}

	std::string ToString() const {
		std::stringstream ss;
		for (int i = 0; i < size_; i++) {
			ss << std::setfill('0') << std::hex << data_[i];
		}
		return ss.str();
	}
};

class PinnableSlice : public Slice, public Cleanable {
public:
    PinnableSlice() { buf_ = &self_space_; }
    explicit PinnableSlice(std::string* buf) { buf_ = buf; }

    // No copy constructor and copy assignment allowed.
    PinnableSlice(PinnableSlice&) = delete;
    PinnableSlice& operator=(PinnableSlice&) = delete;

    inline void PinSlice(const Slice& s, CleanupFunction f, void* arg1, void* arg2) {
        assert(!pinned_);
        pinned_ = true;
        data_ = s.data();
        size_ = s.size();
        RegisterCleanup(f, arg1, arg2);
        assert(pinned_);
    }

    inline void PinSlice(const Slice& s, Cleanable* cleanable) {
        assert(!pinned_);
        pinned_ = true;
        data_ = s.data();
        size_ = s.size();
        cleanable->DelegateCleanupsTo(this);
        assert(pinned_);
    }

    inline void PinSelf(const Slice& slice) {
        assert(!pinned_);
        buf_->assign(slice.data(), slice.size());
        data_ = buf_->data();
        size_ = buf_->size();
        assert(!pinned_);
    }

    inline void PinSelf() {
        assert(!pinned_);
        data_ = buf_->data();
        size_ = buf_->size();
        assert(!pinned_);
    }

    void Reset() {
        Cleanable::Reset();
        pinned_ = false;
    }

    inline std::string* GetSelf() { return buf_; }

    inline bool IsPinned() { return pinned_; }

private:
    std::string self_space_;
    std::string* buf_;
    bool pinned_ = false;
};

using Key = Slice;
using Value = PinnableSlice;
using Location = uint64_t;
constexpr Location INVALID_LOC = (Location)(-1);


struct BlockContents {
    Slice data;
    std::unique_ptr<char[] > allocation;

    BlockContents() {}
    BlockContents(const Slice& _s) : data(_s) {}

    BlockContents(std::unique_ptr<char[]>&& _data, size_t _size) : data(_data.get(), _size){
        allocation.reset(_data.release());
    }
    BlockContents(BlockContents&& other) noexcept { // move constructor
        *this = std::move(other);
    }
    BlockContents& operator=(BlockContents&& other) {
        data = std::move(other.data);
        allocation = std::move(other.allocation);
        return *this;
    }
    bool own_bytes() const { return allocation.get() != nullptr; }
};
