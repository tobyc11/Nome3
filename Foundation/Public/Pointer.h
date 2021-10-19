/****************************************************************************
Copyright (c) 2016 Toby Chen. All rights reserved.
Copyright (c) 2014      PlayFirst Inc.
Copyright (c) 2014-2016 Chukong Technologies Inc.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#pragma once
#include "FoundationAPI.h"
#include <type_traits>
#include <functional>
#include <memory>

namespace tc
{

template <typename T>
class FUniquePtr : public std::unique_ptr<T>
{
public:
	using std::unique_ptr<T>::unique_ptr;
	T* Get() { return this->get(); }
	operator T* () const { return this->get(); }
};

class FRefCount
{
	mutable int RefCount;
protected:
	FRefCount() : RefCount(0) {}

	FRefCount(const FRefCount& other) : RefCount(0) {}

	FRefCount& operator=(const FRefCount& other)
	{
		if (this == &other)
			return *this;
		RefCount = 0;
		return *this;
	}

public:
	virtual ~FRefCount() = default;
	
	template <class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		if (Archive::is_loading::value)
		{
			// Initialize reference count to 0
			// TRefPtr will increase this upon deserialization
			RefCount = 0;
		}
	}

    int AddRef() const
	{
		return ++RefCount;
	}

	int Release() const
	{
		const auto c = --RefCount;
		if (c == 0)
			delete this;
		return c;
	}

	int GetRefCount() const
	{
		return RefCount;
	}
};

#define MACRO_SAFE_ADDREF(ptr)\
    \
    do\
    {\
        if (ptr)\
        {\
            const_cast<T*>(static_cast<const T*>(ptr))->AddRef();\
        }\
    \
    } while (0);

#define MACRO_SAFE_RELEASE(ptr)\
    \
    do\
    {\
        if (ptr)\
        {\
            const_cast<T*>(static_cast<const T*>(ptr))->Release();\
        }\
    \
    } while (0);

#define MACRO_SAFE_RELEASE_NULL(ptr)\
    \
    do\
    {\
        if (ptr)\
        {\
            const_cast<T*>(static_cast<const T*>(ptr))->Release();\
            ptr = nullptr;\
        }\
    \
    } while (0);

/*
 * Subtleties when passing around plain pointers.
 * Objects are expected to have reference count 1 upon construction.
 * TRefPtr usually increases the reference count when a plain pointer is passed in,
 * except the constructor, which you get a choice.
 */
template <typename T> class TRefPtr
{
public:
	TRefPtr() : _ptr(nullptr)
	{
	}

	TRefPtr(const TRefPtr & other) : _ptr(other._ptr)
	{
		MACRO_SAFE_ADDREF(_ptr);
	}

	TRefPtr(TRefPtr && other) noexcept
	{
		_ptr = other._ptr;
		other._ptr = nullptr;
	}

	TRefPtr(T* ptr, bool addRef = true) : _ptr(ptr)
	{
		if (_ptr && addRef)
			_ptr->AddRef();
	}

	explicit TRefPtr(std::nullptr_t ptr) : _ptr(nullptr)
	{
	}

	~TRefPtr()
	{
		MACRO_SAFE_RELEASE_NULL(_ptr);
	}

	template <class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		if (Archive::is_saving::value)
		{
			//_ptr should clear its reference count
			ar & _ptr;
		}
		else
		{
			Clear();
			ar & _ptr;
			if (_ptr)
				_ptr->AddRef();
		}
	}

	TRefPtr& operator=(const TRefPtr & other)
	{
		if (other._ptr != _ptr)
		{
			MACRO_SAFE_ADDREF(other._ptr);
			MACRO_SAFE_RELEASE(_ptr);
			_ptr = other._ptr;
		}
		return *this;
	}

	TRefPtr& operator=(TRefPtr && other) noexcept
	{
		if (&other != this)
		{
			MACRO_SAFE_RELEASE(_ptr);
			_ptr = other._ptr;
			other._ptr = nullptr;
		}
		return *this;
	}

	TRefPtr& operator=(T* other)
	{
		MACRO_SAFE_ADDREF(other);
		MACRO_SAFE_RELEASE(_ptr);
		_ptr = other;
		return *this;
	}

	TRefPtr& operator=(std::nullptr_t other)
	{
		MACRO_SAFE_RELEASE_NULL(_ptr);
		return *this;
	}

	operator T* () const { return _ptr; }

	T& operator*() const
	{
		return *_ptr;
	}

	T* operator->() const
	{
		return _ptr;
	}

	T* Get() const { return _ptr; }

	bool operator==(const TRefPtr<T> & other) const { return _ptr == other._ptr; }
	bool operator==(const T * other) const { return _ptr == other; }
	bool operator==(typename std::remove_const<T>::type * other) const { return _ptr == other; }
	bool operator==(const std::nullptr_t other) const { return _ptr == other; }

	bool operator!=(const TRefPtr<T> & other) const { return _ptr != other._ptr; }
	bool operator!=(const T * other) const { return _ptr != other; }
	bool operator!=(typename std::remove_const<T>::type * other) const { return _ptr != other; }
	bool operator!=(const std::nullptr_t other) const { return _ptr != other; }

	bool operator>(const TRefPtr<T> & other) const { return _ptr > other._ptr; }
	bool operator>(const T * other) const { return _ptr > other; }
	bool operator>(typename std::remove_const<T>::type * other) const { return _ptr > other; }

	bool operator<(const TRefPtr<T> & other) const { return _ptr < other._ptr; }
	bool operator<(const T * other) const { return _ptr < other; }
	bool operator<(typename std::remove_const<T>::type * other) const { return _ptr < other; }

	bool operator>=(const TRefPtr<T> & other) const { return _ptr >= other._ptr; }
	bool operator>=(const T * other) const { return _ptr >= other; }
	bool operator>=(typename std::remove_const<T>::type * other) const { return _ptr >= other; }

	bool operator<=(const TRefPtr<T> & other) const { return _ptr <= other._ptr; }
	bool operator<=(const T * other) const { return _ptr <= other; }
	bool operator<=(typename std::remove_const<T>::type * other) const { return _ptr <= other; }

	operator bool() const { return _ptr != nullptr; }

	void Clear()
	{
		MACRO_SAFE_RELEASE_NULL(_ptr);
	}

	void Swap(TRefPtr<T> & other)
	{
		if (&other != this)
		{
			T * tmp = _ptr;
			_ptr = other._ptr;
			other._ptr = tmp;
		}
	}

private:
	T * _ptr;
};

template<class T> inline
bool operator<(const TRefPtr<T>& r, std::nullptr_t)
{
	return std::less<T*>()(r.Get(), nullptr);
}

template<class T> inline
bool operator<(std::nullptr_t, const TRefPtr<T>& r)
{
	return std::less<T*>()(nullptr, r.Get());
}

template<class T> inline
bool operator>(const TRefPtr<T>& r, std::nullptr_t)
{
	return nullptr < r;
}

template<class T> inline
bool operator>(std::nullptr_t, const TRefPtr<T>& r)
{
	return r < nullptr;
}

template<class T> inline
bool operator<=(const TRefPtr<T>& r, std::nullptr_t)
{
	return !(nullptr < r);
}

template<class T> inline
bool operator<=(std::nullptr_t, const TRefPtr<T>& r)
{
	return !(r < nullptr);
}

template<class T> inline
bool operator>=(const TRefPtr<T>& r, std::nullptr_t)
{
	return !(r < nullptr);
}

template<class T> inline
bool operator>=(std::nullptr_t, const TRefPtr<T>& r)
{
	return !(nullptr < r);
}

template<class T, class U> TRefPtr<T> static_pointer_cast(const TRefPtr<U> & r)
{
	return TRefPtr<T>(static_cast<T*>(r.Get()));
}

template<class T, class U> TRefPtr<T> dynamic_pointer_cast(const TRefPtr<U> & r)
{
	return TRefPtr<T>(dynamic_cast<T*>(r.Get()));
}

#undef MACRO_SAFE_ADDREF
#undef MACRO_SAFE_RELEASE
#undef MACRO_SAFE_RELEASE_NULL

}
