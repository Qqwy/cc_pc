#include <iostream>
#include <string>
#include <functional>
#include <experimental/optional>
#include <type_traits>
using namespace std::experimental;


//! Tests if T is a specialization of Template
template <typename T, template <typename...> class Template>
struct is_specialization_of : std::false_type {};
template <template <typename...> class Template, typename... Args>
struct is_specialization_of<Template<Args...>, Template> : std::true_type {};

// template <typename F>
// struct Parser
// {
//     F d_fun;
//     Parser(F const &fun)
//         : d_fun(fun)
//         {}
//     auto run(std::string const &in) -> decltype(F(std::declval<std::string &>()))
//     {
//         return F(in);
//     };
// };

class Empty{};

// template<typename F>
template <typename A>
struct Parser {
    typedef std::pair<A, std::string> SuccessfullParse;
    typedef optional<SuccessfullParse> MaybeParse;

    std::function<MaybeParse(std::string&)> d_fun;
    Parser(std::function<MaybeParse(std::string&)> const & fun)
        : d_fun(fun) {};

    auto run(std::string &in)
        -> MaybeParse
    {
        return d_fun(in);
    }
};


// template<typename F>
// struct Parser2 {
//     F fun;

//     template<typename... Rest>
//     auto operator()(std::string &in, Rest&&... rest)
//         -> decltype( fun(std::declval<std::string &>(), std::forward<Rest>(rest)...) )
//         {
//             return fun(in, std::forward<Rest>(rest)...);
//             // Handle
//         }
// };


// auto lam = [](std::string &in){ return in;};
// auto satisfy = Parser<decltype(lam)>{lam};

template <typename F>
Parser<char> satisfies(F const &fun)
{
    auto lam = [&](std::string &in)
    {
        if(in.empty())
            return optional<std::pair<char, std::string>>{};
        char ch = in[0];
        if (!fun(ch))
            return optional<std::pair<char, std::string>>{};
        return make_optional(std::make_pair(ch, in.substr(1, in.size())));
    };
    return Parser<char>{lam};
}

Parser<char> space()
{
    return satisfies([](char ch){return ch == ' ';});
}

Parser<char> alpha()
{
    return satisfies([](char ch){return ::isalpha(ch);});
}
Parser<char> digit()
{
    return satisfies([](char ch){return ::isdigit(ch);});
}

Parser<char> ischar(char the_char)
{
    return satisfies([&](char real_char){return the_char == real_char;});
}

template <typename A, typename B, typename std::enable_if<!is_specialization_of<A, std::tuple>::value>::type = 0>
Parser<std::tuple<A, B>> operator >>(Parser<A> parser_a, Parser<B> parser_b)
{
    auto lam = [&](std::string &in){
        optional<std::pair<A, std::string>> result_a = parser_a.run(in);
        if(!result_a)
            return optional<std::pair<std::tuple<A, B>, std::string>>{};
        optional<std::pair<B, std::string>> result_b = parser_b.run(result_a->second);
        if(!result_b)
            return optional<std::pair<std::tuple<A, B>, std::string>>{};
        return make_optional( std::make_pair(std::make_tuple(result_a->first, result_b->first), result_b->second));
    };
    return Parser<std::tuple<A, B>>{lam};
}

template <typename A, typename...As, typename B>
auto operator >>(Parser<std::tuple<A, As...>> parser_a, Parser<B> parser_b) -> Parser<decltype(std::tuple_cat(std::declval<std::tuple<A, As...>>(), std::declval<std::tuple<B>>()))>
{
    auto lam = [&](std::string &in){
        optional<std::pair<std::tuple<A, As...>, std::string>> result_a = parser_a.run(in);
        if(!result_a)
            return optional<std::pair<std::tuple<A, As..., B>, std::string>>{};
        optional<std::pair<B, std::string>> result_b = parser_b.run(result_a->second);
        if(!result_b)
            return optional<std::pair<std::tuple<A, As..., B>, std::string>>{};
        return make_optional( std::make_pair(std::tuple_cat(result_a->first, std::make_tuple(result_b->first)), result_b->second));
    };
    return Parser<std::tuple<A, As..., B>>{lam};
}


template <typename A>
Parser<A> operator >>(Parser<A> parser_a, Parser<Empty> parser_b)
{
    auto lam = [&](std::string &in){
        optional<std::pair<A, std::string>> result_a = parser_a.run(in);
        if(!result_a)
            return optional<std::pair<A, std::string>>{};
        optional<std::pair<Empty, std::string>> result_b = parser_b.run(result_a->second);
        if(!result_b)
            return optional<std::pair<A, std::string>>{};
        return make_optional( std::make_pair(result_a->first, result_b->second));
    };
    return Parser<A>{lam};
}

template <typename B>
Parser<B> operator >>(Parser<Empty> parser_a, Parser<B> parser_b)
{
    auto lam = [&](std::string &in){
        optional<std::pair<Empty, std::string>> result_a = parser_a.run(in);
        if(!result_a)
            return optional<std::pair<B, std::string>>{};
        optional<std::pair<B, std::string>> result_b = parser_b.run(result_a->second);
        if(!result_b)
            return optional<std::pair<B, std::string>>{};
        return make_optional( std::make_pair(result_b->first, result_b->second));
    };
    return Parser<B>{lam};
}

template<size_t N>
struct print_tuple{
    template<typename... T>static typename std::enable_if<(N<sizeof...(T))>::type
    print(std::ostream& os, const std::tuple<T...>& t) {
        char quote = (std::is_convertible<decltype(std::get<N>(t)), std::string>::value) ? '"' : 0;
        os << ", " << quote << std::get<N>(t) << quote;
        print_tuple<N+1>::print(os,t);
    }
        template<typename... T>static typename std::enable_if<!(N<sizeof...(T))>::type
        print(std::ostream&, const std::tuple<T...>&) {
        }
};
std::ostream& operator<< (std::ostream& os, const std::tuple<>&) {
    return os << "std::tuple{}";
}
template<typename T0, typename ...T> std::ostream&
operator<<(std::ostream& os, const std::tuple<T0, T...>& t){
    char quote = (std::is_convertible<T0, std::string>::value) ? '"' : 0;
    os << "std::tuple{" << quote << std::get<0>(t) << quote;
    print_tuple<1>::print(os,t);
    return os << '}';
}

template <typename A>
Parser<Empty> skip(Parser<A> parser)
{
    auto lam = [&](std::string &in){
        auto result = parser.run(in);
        if(result)
            return make_optional(std::make_pair(Empty{}, result->second));
        return optional<std::pair<Empty, std::string>>{};
    };
    return Parser<Empty>{lam};
}

// Pretty print pair.
template <typename A, typename B>
std::ostream & operator<<(std::ostream &os, std::pair<A, B> pair)
{
    os << "std::pair{" << pair.first << ", " << pair.second << "}";
    return os;
}
// Pretty print tuple.

int main()
{
    std::string str;
    str.assign((std::istreambuf_iterator<char>(std::cin)),
               std::istreambuf_iterator<char>());
    auto parser = skip(digit()) >> alpha() >> digit() >> alpha() >> skip(digit());
    auto result = parser.run(str);
    if(result)
    {
        auto val = result.value();
        std::cout << "Parse success:\n";
        std::cout << val.first << '\n';
    }
    else
    {
        std::cout << "Parsing failed\n";
    }
}
