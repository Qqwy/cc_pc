#include <iostream>
#include <string>
#include <functional>
#include <experimental/optional>

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
    typedef std::experimental::optional<SuccessfullParse> MaybeParse;

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
Parser<bool> satisfies(F const &fun)
{
    auto lam = [&](std::string &in){
        if(in.empty())
            return std::make_pair(false, in);
        char ch = in[0];
        if (!fun(ch))
            return std::make_pair(false, in);
        return std::make_pair(true, in.substr(1, in.size()));
    };
    return Parser<bool>{lam};
}

Parser<bool> space()
{
    return satisfies([](char ch){return ch == ' ';});
}

Parser<bool> alpha()
{
    return satisfies([](char ch){return ::isalpha(ch);});
}
Parser<bool> digit()
{
    return satisfies([](char ch){return ::isdigit(ch);});
}

Parser<bool> ischar(char the_char)
{
    return satisfies([&](char real_char){return the_char == real_char;});
}

template <typename A, typename B>
Parser<std::pair<A, B>> operator >>(Parser<A> parser_a, Parser<B> parser_b)
{
    auto lam = [&](std::string &in){
        std::experimental::optional<std::pair<A, std::string>> result_a = parser_a.run(in);
        if(!result_a)
            return std::experimental::optional<std::pair<std::pair<A, B>, std::string>>{};
        std::experimental::optional<std::pair<B, std::string>> result_b = parser_b.run(result_a->second);
        if(!result_b)
            return std::experimental::optional<std::pair<std::pair<A, B>, std::string>>{};
        return std::experimental::make_optional( std::make_pair(std::make_pair(result_a->first, result_b->first), result_b->second));
    };
    return Parser<std::pair<A, B>>{lam};
}

template <typename A>
Parser<A> operator >>(Parser<A> parser_a, Parser<Empty> parser_b)
{
    auto lam = [&](std::string &in){
        std::experimental::optional<std::pair<A, std::string>> result_a = parser_a.run(in);
        if(!result_a)
            return std::experimental::optional<std::pair<A, std::string>>{};
        std::experimental::optional<std::pair<Empty, std::string>> result_b = parser_b.run(result_a->second);
        if(!result_b)
            return std::experimental::optional<std::pair<A, std::string>>{};
        return std::experimental::make_optional( std::make_pair(result_a->first, result_b->second));
    };
    return Parser<A>{lam};
}

template <typename B>
Parser<B> operator >>(Parser<Empty> parser_a, Parser<B> parser_b)
{
    auto lam = [&](std::string &in){
        std::experimental::optional<std::pair<Empty, std::string>> result_a = parser_a.run(in);
        if(!result_a)
            return std::experimental::optional<std::pair<B, std::string>>{};
        std::experimental::optional<std::pair<B, std::string>> result_b = parser_b.run(result_a->second);
        if(!result_b)
            return std::experimental::optional<std::pair<B, std::string>>{};
        return std::experimental::make_optional( std::make_pair(result_b->first, result_b->second));
    };
    return Parser<B>{lam};
}


// template <typename A>
// Parser<A> operator >>(Parser<A> parser_a, Parser<void> parser_b)
// {
//     auto lam = [&](std::string &in){
//         std::pair<A, std::string> result_a = parser_a.run(in);
//         if(result_a)
//         {
//             std::pair<void, std::string> result_b = parser_b.run(result_a->second);
//             return std::make_pair(result_a->first, result_b.second);
//         }
//     };
//     return Parser<A>{lam};
// }

// template <typename B>
// Parser<B> operator >>(Parser<Empty> parser_a, Parser<B> parser_b)
// {
//     auto lam = [&](std::string &in){
//         std::pair<Empty, std::string> result_a = parser_a.run(in);
//         if(result_a.first != nullptr)
//         {
//             std::pair<B, std::string> result_b = parser_b.run(result_a.second);
//             return std::make_pair(result_b.first, result_b.second);
//         }
//     };
//     return Parser<B>{lam};
// }

template <typename A>
Parser<Empty> skip(Parser<A> parser)
{
    auto lam = [&](std::string &in){
        auto result = parser.run(in);
        if(result)
            return std::experimental::make_optional(std::make_pair(Empty{}, in));
        return std::experimental::optional<std::pair<Empty, std::string>>{};
    };
    return Parser<Empty>{lam};
}


int main()
{
    std::string str;
    str.assign((std::istreambuf_iterator<char>(std::cin)),
               std::istreambuf_iterator<char>());
    // auto result = alpha().run(str);
    auto result = skip(digit()) >> alpha() >> digit() >> alpha() >> skip(digit());
    // std::cout << result.first;
}
