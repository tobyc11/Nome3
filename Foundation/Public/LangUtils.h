#pragma once
#include <stdexcept>
namespace tc
{

class FNonCopyable
{
protected:
    constexpr FNonCopyable() = default;
    ~FNonCopyable() = default;

private:
    FNonCopyable(const FNonCopyable&) = delete;
    void operator=(const FNonCopyable&) = delete;
};

class FNoCopyMove
{
protected:
    constexpr FNoCopyMove() = default;
    ~FNoCopyMove() = default;

private:
    FNoCopyMove(const FNoCopyMove&) = delete;
    void operator=(const FNoCopyMove&) = delete;
    FNoCopyMove(FNoCopyMove&&) = delete;
    void operator=(FNoCopyMove&&) = delete;
};

class FNoImplicitCopy
{
protected:
    constexpr FNoImplicitCopy() = default;
    ~FNoImplicitCopy() = default;

    explicit FNoImplicitCopy(const FNoImplicitCopy&) = default;
};

template <typename TMember> struct TMemberPtrTraits
{
};

template <typename R, typename T> struct TMemberPtrTraits<R T::*>
{
    using TReturn = R;
    using TClass = T;
};

class not_implemented_exception : public std::logic_error
{
public:
    explicit not_implemented_exception(const std::string& _Message)
        : logic_error(_Message)
    {
    }

    explicit not_implemented_exception(const char* _Message)
        : logic_error(_Message)
    {
    }

    char const* what() const noexcept override { return "Function not yet implemented."; }
};

template <typename E> constexpr typename std::underlying_type<E>::type to_underlying(E e)
{
    return static_cast<typename std::underlying_type<E>::type>(e);
}

}
