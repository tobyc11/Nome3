#pragma once

#include <atomic>
#include <functional>

namespace tc {

class FRefCounted
{
public:
	//It seems like starting ref count from 0 is more natural,
	//  but please make sure the pointer class has matching constructor and assignment operator
	FRefCounted() : RefCount(0)
	{
	}

	unsigned int AddRef() const
	{
		return RefCount.fetch_add(1, std::memory_order_relaxed) + 1;
	}

	unsigned int Release() const
	{
		unsigned int retval = RefCount.fetch_sub(1, std::memory_order_acquire) - 1;
		if (retval == 0)
			delete this;
		return retval;
	}

	unsigned int GetRefCount() const
	{
		return RefCount;
	}

protected:
	virtual ~FRefCounted() = default;

private:
	//Disable copy and move
	FRefCounted(const FRefCounted&) = delete;
	FRefCounted(FRefCounted&&) = delete;
	FRefCounted& operator=(const FRefCounted&) = delete;
	FRefCounted& operator=(FRefCounted&&) = delete;

	mutable std::atomic_uint32_t RefCount;
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

template <typename T>
class TAutoPtr
{
public:
	//Null initialization
	TAutoPtr() : Ptr(nullptr) {}

	explicit TAutoPtr(std::nullptr_t ptr) : Ptr(nullptr) {}

	TAutoPtr& operator=(std::nullptr_t other)
	{
		MACRO_SAFE_RELEASE_NULL(Ptr);
		return *this;
	}

	//Copy
	TAutoPtr(T* ptr, bool addRef = true) : Ptr(ptr)
	{
		if (addRef)
			MACRO_SAFE_ADDREF(Ptr);
	}

	TAutoPtr(const TAutoPtr& other) : Ptr(other.Ptr)
	{
		MACRO_SAFE_ADDREF(Ptr);
	}

	//Copy assignment
	TAutoPtr& operator=(T* other)
	{
		MACRO_SAFE_ADDREF(other);
		MACRO_SAFE_RELEASE(Ptr);
		Ptr = other;
		return *this;
	}

	TAutoPtr& operator=(const TAutoPtr& other)
	{
		if (other.Ptr != Ptr)
		{
			MACRO_SAFE_ADDREF(other.Ptr);
			MACRO_SAFE_RELEASE(Ptr);
			Ptr = other.Ptr;
		}
		return *this;
	}

	//Move
	TAutoPtr(TAutoPtr&& other) noexcept
	{
		Ptr = other.Ptr;
		other.Ptr = nullptr;
	}

	//Move Assignment
	TAutoPtr& operator=(TAutoPtr&& other) noexcept
	{
		if (&other != this)
		{
			MACRO_SAFE_RELEASE(Ptr);
			Ptr = other.Ptr;
			other.Ptr = nullptr;
		}
		return *this;
	}

	//Dtor
	~TAutoPtr()
	{
		MACRO_SAFE_RELEASE_NULL(Ptr);
	}

	operator T*() const { return Ptr; }

	T& operator*() const
	{
		return *Ptr;
	}

	T* operator->() const
	{
		return Ptr;
	}

	T* Get() const { return Ptr; }

	bool operator==(const TAutoPtr<T>& other) const { return Ptr == other.Ptr; }
	bool operator==(const T* other) const { return Ptr == other; }
	bool operator==(typename std::remove_const<T>::type* other) const { return Ptr == other; }
	bool operator==(const std::nullptr_t other) const { return Ptr == other; }

	bool operator!=(const TAutoPtr<T>& other) const { return Ptr != other.Ptr; }
	bool operator!=(const T* other) const { return Ptr != other; }
	bool operator!=(typename std::remove_const<T>::type* other) const { return Ptr != other; }
	bool operator!=(const std::nullptr_t other) const { return Ptr != other; }

	bool operator>(const TAutoPtr<T>& other) const { return Ptr > other.Ptr; }
	bool operator>(const T* other) const { return Ptr > other; }
	bool operator>(typename std::remove_const<T>::type* other) const { return Ptr > other; }

	bool operator<(const TAutoPtr<T>& other) const { return Ptr < other.Ptr; }
	bool operator<(const T* other) const { return Ptr < other; }
	bool operator<(typename std::remove_const<T>::type* other) const { return Ptr < other; }

	bool operator>=(const TAutoPtr<T>& other) const { return Ptr >= other.Ptr; }
	bool operator>=(const T* other) const { return Ptr >= other; }
	bool operator>=(typename std::remove_const<T>::type* other) const { return Ptr >= other; }

	bool operator<=(const TAutoPtr<T>& other) const { return Ptr <= other.Ptr; }
	bool operator<=(const T* other) const { return Ptr <= other; }
	bool operator<=(typename std::remove_const<T>::type* other) const { return Ptr <= other; }

	operator bool() const { return Ptr != nullptr; }

	void Clear()
	{
		MACRO_SAFE_RELEASE_NULL(Ptr);
	}

private:
	T* Ptr;
};

template<class T> inline
bool operator<(const TAutoPtr<T>& r, std::nullptr_t)
{
	return std::less<T*>()(r.Get(), nullptr);
}

template<class T> inline
bool operator<(std::nullptr_t, const TAutoPtr<T>& r)
{
	return std::less<T*>()(nullptr, r.Get());
}

template<class T> inline
bool operator>(const TAutoPtr<T>& r, std::nullptr_t)
{
	return nullptr < r;
}

template<class T> inline
bool operator>(std::nullptr_t, const TAutoPtr<T>& r)
{
	return r < nullptr;
}

template<class T> inline
bool operator<=(const TAutoPtr<T>& r, std::nullptr_t)
{
	return !(nullptr < r);
}

template<class T> inline
bool operator<=(std::nullptr_t, const TAutoPtr<T>& r)
{
	return !(r < nullptr);
}

template<class T> inline
bool operator>=(const TAutoPtr<T>& r, std::nullptr_t)
{
	return !(r < nullptr);
}

template<class T> inline
bool operator>=(std::nullptr_t, const TAutoPtr<T>& r)
{
	return !(nullptr < r);
}

template<class T, class U> TAutoPtr<T> static_pointer_cast(const TAutoPtr<U>& r)
{
	return TAutoPtr<T>(static_cast<T*>(r.Get()));
}

template<class T, class U> TAutoPtr<T> dynamic_pointer_cast(const TAutoPtr<U>& r)
{
	return TAutoPtr<T>(dynamic_cast<T*>(r.Get()));
}

#undef MACRO_SAFE_ADDREF
#undef MACRO_SAFE_RELEASE
#undef MACRO_SAFE_RELEASE_NULL

} //namespace tc
