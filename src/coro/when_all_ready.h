#pragma once

#include "concepts.h"

namespace clu
{
    // TODO: finish this

    template <typename T>
    class when_all_task final
    {
    public:

    private:

    public:
    };

    namespace detail
    {
        template <typename T> struct is_reference_wrapper : std::false_type {};
        template <typename T> struct is_reference_wrapper<std::reference_wrapper<T>> : std::true_type {};

        template <typename T> struct unwrapped
        {
            using type = T;
        };
        template <typename T> struct unwrapped<std::reference_wrapper<T>>
        {
            using type = T;
        };
        template <typename T> using unwrapped_t = typename unwrapped<T>::type;

        template <typename T> concept wrapped_awaitable = awaitable<unwrapped_t<T>>;
    }

    template <detail::wrapped_awaitable... As>
    auto when_all_ready(As ... awaitables)
    {
        
    }
}
