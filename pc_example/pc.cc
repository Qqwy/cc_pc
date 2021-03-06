#include <vector>
#include <iostream>
#include <iterator>
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


// Pretty print pair.
template <typename A, typename B>
std::ostream & operator<<(std::ostream &os, std::pair<A, B> pair)
{
    os << "std::pair{" << pair.first << ", " << pair.second << "}";
    return os;
}

// Pretty print tuple.
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

// Pretty print vector.
template <typename A>
std::ostream &operator<< (std::ostream& os, std::vector<A> const &vector)
{
    os << "std::vector{";
    std::copy(vector.begin(), vector.end(), std::ostream_iterator<A>(os));
    os << "}";
    return os;
}

// ACTUAL INTERESTING CODE:

// Returned by parsers that return 'nothing', such as `skip`.
// gets discarded when combined with other parsers.
class Empty{};

template <typename A>
struct Parser {
    typedef std::pair<A, std::string> SuccessfullParse;
    typedef optional<SuccessfullParse> MaybeParse;

    std::function<MaybeParse(std::string&)> d_fun;
    Parser(std::function<MaybeParse(std::string&)> const &fun)
        : d_fun(fun) {};

    auto run(std::string &in) const
        -> MaybeParse
    {
        return d_fun(in);
    }
};

template <typename A>
std::ostream & operator<<(std::ostream &os, Parser<A> &parser_a)
{
    os << "Parser{}";
    return os;
}


template <typename F>
Parser<char> satisfies(F const &fun)
{
    auto lam = [&](std::string &in)
    {
        if(in.empty())
            return Parser<char>::MaybeParse{};
        char ch = in[0];
        if (!fun(ch))
            return Parser<char>::MaybeParse{};
        return Parser<char>::MaybeParse{Parser<char>::SuccessfullParse(ch, in.substr(1, in.size()))};
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

//template <typename A, typename B, typename std::enable_if<!is_specialization_of<A, std::tuple>::value, bool>::type = 0>
template <typename A, typename B>
Parser<std::tuple<A, B>> operator >>(Parser<A> const &parser_a, Parser<B> const &parser_b)
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

// template <typename...As, typename B, typename std::enable_if<!std::is_base_of<B, Empty>::value, bool>::type = 0>
// auto operator >>(Parser<std::tuple<As...>> const &parser_a, Parser<B> const &parser_b) -> Parser<decltype(std::tuple_cat(std::declval<std::tuple<As...>>(), std::declval<std::tuple<B>>()))>
// {
//     auto lam = [&](std::string &in){
//         optional<std::pair<std::tuple<As...>, std::string>> result_a = parser_a.run(in);
//         if(!result_a)
//             return optional<std::pair<std::tuple<As..., B>, std::string>>{};
//         optional<std::pair<B, std::string>> result_b = parser_b.run(result_a->second);
//         if(!result_b)
//             return optional<std::pair<std::tuple<As..., B>, std::string>>{};
//         return make_optional( std::make_pair(std::tuple_cat(result_a->first, std::make_tuple(result_b->first)), result_b->second));
//     };
//     return Parser<std::tuple<As..., B>>{lam};
// }


template <typename A>
Parser<A> operator >>(Parser<A> const &parser_a, Parser<Empty> const &parser_b)
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
Parser<B> operator >>(Parser<Empty> const &parser_a, Parser<B> const &parser_b)
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

template <typename A, typename Fun>
auto fmap(Parser<A> parser_a, Fun const &fun) -> Parser<decltype(fun(std::declval<A>()))>
{
    std::cout << "OUTSIDE OF LAMBDA!\n" << parser_a;
    auto lam = [&](std::string &in)
        {
            std::cout << "INSIDE OF LAMBDA!\n" << parser_a;
            optional<std::pair<A, std::string>> result_a;
            result_a = parser_a.run(in);
            std::cout << "AFTER RUNNING PARSER\n";
            // std::cout << "Result:"<< !!result_a;
            if(!result_a)
                return optional<std::pair<decltype(fun(std::declval<A>())), std::string>>{};
            return make_optional(std::make_pair(fun(result_a->first), result_a->second));
        };
    return Parser<decltype(fun(std::declval<A>()))>{lam};
}

template <typename A, typename ReturnType>
Parser<ReturnType> fmap2(Parser<A> parser_a, std::function<ReturnType(A)> const &fun)
{
    std::cout << "OUTSIDE OF LAMBDA2!\n" << parser_a;
    auto lam = [&](std::string &in)
        {
            std::cout << "INSIDE OF LAMBDA!\n" << parser_a;
            auto result = parser_a.run(in);
            if(!result)
                return optional<std::pair<ReturnType, std::string>>{};
            return make_optional(std::make_pair(fun(result->first), result->second));
        };
    return Parser<ReturnType>{lam};
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

template <typename A>
Parser<A> operator|(Parser<A> const &parser_a, Parser<A> const &parser_b)
{
    auto lam = [&](std::string &in)
        {
            auto result = parser_a.run(in);
            if(result)
                return result;
            else
                return parser_b.run(in);
        };
    return Parser<A>{lam};
}


Parser<char> alnum()
{
    return alpha() | digit();
}

template <typename A>
Parser<std::vector<A>> many(Parser<A> const &parser_a)
{
    auto lam = [&](std::string in)
        {
            std::vector<A> vec{};
            while (true)
            {
                auto parse_result = parser_a.run(in);//many1(parser_a).run(in);
                if(!parse_result)
                    break;
                vec.push_back(parse_result->first);
                in = parse_result->second;
            }
            return make_optional(std::make_pair(vec, in));
        };
    return Parser<std::vector<A>>{lam};
}

// template <typename A>
// Parser<std::vector<A>> many1(Parser<A> parser_a)
// {
//     auto lam = [&](std::string &in)
//         {
//             auto result_head = parser_a.run(in);
//             std::vector<A> result;
//             if(result_head)
//             result.push_back(result_head->first);
//             auto rest = many(parser_a).run(result_head->second);
//             if(!rest)
//                 return result;

//             result.insert(result.end(), rest->first.begin(), rest->first.end());
//             return result;
//         };
//     return Parser<std::vector<A>>{lam};
// }

// template <typename A>
// // Parser<std::vector<A>> many1(Parser<A> parser_a)
// Parser<std::tuple<A, A, A>> many1(Parser<A> parser_a)
// {
//     // auto prepend_a = [&](std::tuple<A, std::vector<A>> tup){
//     //     // std::cout << "TEST!\n";
//     //     // A first;
//     //     // std::vector<A> rest;
//     //     // std::tie(first, rest) = tup;
//     //     // std::cout << "Prepending" << first << " to " << rest << '\n';
//     //     // rest.insert(rest.begin(), first);
//     //     // return rest;
//     //     return std::get<1>(tup);
//     // };
//     // auto parser1 = parser_a >> parser_a >> parser_a;//many(parser_a);
//     // std::function<char(std::tuple<char, char, char>)> lam = [&](std::tuple<char, char, char> chars) -> char {return std::get<0>(chars);};
//     // return fmap2(parser1, lam);
//     auto parser2 = digit() >> digit() >> digit();
//     return parser2;
// }

template <typename A>
Parser<std::tuple<A, A>> many1(Parser<A> parser_a)
{
    //return parser_a >> parser_a >> parser_a;
    return digit() >> digit();
}

int main()
{
    std::string str;
    str.assign((std::istreambuf_iterator<char>(std::cin)),
               std::istreambuf_iterator<char>());
    auto parser = skip(digit()) >> alpha() >> digit() >> alpha() >> skip(digit());
    std::function<int(std::tuple<std::tuple<char,char>, char>)> convert_lambda = [](std::tuple<std::tuple<char, char>, char> digits){return static_cast<int>(std::get<1>(digits) - '0');};
    auto digit_parser = fmap2(space() >> digit() >> digit(), convert_lambda);
    auto digit_parser2 = (many1(digit()));
    auto result = digit_parser2.run(str);
    if(result)
    {
        std::cout << "Parse success:\n";
        std::cout << result->first << '\n';
        std::cout << "Unmatched rest string: \n";
        std::cout << result->second;
    }
    else
        std::cout << "Parsing failed\n";
}
