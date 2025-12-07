#ifndef NONCOPYABLE_HPP
#define NONCOPYABLE_HPP

/// Anything that inherits from this can't be copied
/// See https://github.com/mousebird/boost/blob/master/boost/noncopyable.hpp
class NonCopyable {
    public:
        NonCopyable() = default;
        ~NonCopyable() = default;
    private:
        NonCopyable(const NonCopyable&) = delete;
        const NonCopyable& operator=(const NonCopyable&) = delete;
};

#endif