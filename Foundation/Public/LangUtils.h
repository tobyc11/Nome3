#pragma once
#include <stdexcept>
namespace tc
{

class NonCopyable
{
protected:
	constexpr NonCopyable() = default;
	~NonCopyable() = default;

private:
	NonCopyable(const NonCopyable&) = delete;
	void operator=(const NonCopyable&) = delete;
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

    char const * what() const noexcept override { return "Function not yet implemented."; }
};

template <typename E>
constexpr typename std::underlying_type<E>::type to_underlying(E e)
{
	return static_cast<typename std::underlying_type<E>::type>(e);
}

}
