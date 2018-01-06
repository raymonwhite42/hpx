//  Copyright (c) 2017 Agustin Berge
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef HPX_UTIL_FORMAT_HPP
#define HPX_UTIL_FORMAT_HPP

#include <hpx/config.hpp>

#include <boost/utility/string_ref.hpp>

#include <cctype>
#include <cstddef>
#include <cstdio>
#include <ostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace hpx { namespace util
{
    namespace detail
    {
        ///////////////////////////////////////////////////////////////////////
        template <typename T>
        struct type_specifier
        {
            static char const* value() noexcept;
        };

#       define DECL_TYPE_SPECIFIER(Type, Spec)                                \
        template <> struct type_specifier<Type>                               \
        { static char const* value() noexcept { return #Spec; } }             \
        /**/

        DECL_TYPE_SPECIFIER(char, c);
        DECL_TYPE_SPECIFIER(wchar_t, lc);

        DECL_TYPE_SPECIFIER(signed char, hhd);
        DECL_TYPE_SPECIFIER(short, hd);
        DECL_TYPE_SPECIFIER(int, d);
        DECL_TYPE_SPECIFIER(long, ld);
        DECL_TYPE_SPECIFIER(long long, lld);

        DECL_TYPE_SPECIFIER(unsigned char, hhu);
        DECL_TYPE_SPECIFIER(unsigned short, hu);
        DECL_TYPE_SPECIFIER(unsigned int, u);
        DECL_TYPE_SPECIFIER(unsigned long, lu);
        DECL_TYPE_SPECIFIER(unsigned long long, llu);

        DECL_TYPE_SPECIFIER(float, f);
        DECL_TYPE_SPECIFIER(double, lf);
        DECL_TYPE_SPECIFIER(long double, Lf);

        template <typename T>
        struct type_specifier<T*>
        {
            static char const* value() noexcept { return "p"; }
        };

#       undef DECL_TYPE_SPECIFIER

        ///////////////////////////////////////////////////////////////////////
        template <typename T, bool IsFundamental = std::is_fundamental<T>::value>
        struct formatter
        {
            static void call(
                std::ostream& os, boost::string_ref spec, void const* ptr)
            {
                // conversion specifier
                char const* conv_spec = "";
                if (spec.empty() || !std::isalpha(spec.back()))
                    conv_spec = type_specifier<T>::value();

                // copy spec to a null terminated buffer
                char format[16];
                std::sprintf(format, "%%%.*s%s",
                    (int)spec.size(), spec.data(), conv_spec);

                T const& value = *static_cast<T const*>(ptr);
                std::size_t length = std::snprintf(nullptr, 0, format, value);
                std::vector<char> buffer(length + 1);
                length = std::snprintf(buffer.data(), length + 1, format, value);

                os << boost::string_ref(buffer.data(), length);
            }
        };

        template <>
        struct formatter<bool>; // missing

        template <typename T>
        void format_value(std::ostream& os, boost::string_ref spec, T const& value)
        {
            if (!spec.empty())
                throw std::runtime_error("Not a valid format specifier");

            os << value;
        }

        template <typename T>
        struct formatter<T, /*IsFundamental=*/false>
        {
            static void call(
                std::ostream& os, boost::string_ref spec, void const* value)
            {
                // ADL customization point
                format_value(os, spec, *static_cast<T const*>(value));
            }
        };

        struct format_arg
        {
            template <typename T>
            format_arg(T const& arg)
              : _data(&arg)
              , _formatter(&detail::formatter<T>::call)
            {}

            void operator()(std::ostream& os, boost::string_ref spec) const
            {
                _formatter(os, spec, _data);
            }

            void const* _data;
            void (*_formatter)(std::ostream&, boost::string_ref spec, void const*);
        };

        ///////////////////////////////////////////////////////////////////////
        HPX_EXPORT void format_to(
            std::ostream& os,
            boost::string_ref format_str,
            format_arg const* args, std::size_t count);

        HPX_EXPORT std::string format(
            boost::string_ref format_str,
            format_arg const* args, std::size_t count);
    }

    template <typename ...Args>
    std::string format(
        boost::string_ref format_str, Args const&... args)
    {
        detail::format_arg const format_args[] = { args..., 0 };
        return detail::format(format_str, format_args, sizeof...(Args));
    }

    template <typename ...Args>
    std::ostream& format_to(
        std::ostream& os,
        boost::string_ref format_str, Args const&... args)
    {
        detail::format_arg const format_args[] = { args..., 0 };
        detail::format_to(os, format_str, format_args, sizeof...(Args));

        return os;
    }
}}

#endif /*HPX_UTIL_FORMAT_HPP*/
