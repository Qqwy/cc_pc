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


// template<typename F>
template <typename A>
struct Parser {
    std::function<std::pair<A, std::string>(std::string&)> d_fun;
    Parser(std::function<std::pair<A, std::string>(std::string&)> const & fun)
        : d_fun(fun) {};

    auto run(std::string &in)
        -> std::pair<A, std::string>
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

template <typename A, typename B>
Parser<std::pair<A, B>> operator >>(Parser<A> parser_a, Parser<B> parser_b)
{
    auto lam = [&](std::string &in){
        std::pair<A, std::string> result_a = parser_a.run(in);
        if(result_a.first != nullptr)
        {
            std::pair<B, std::string> result_b = parser_b.run(result_a.second);
            return std::make_pair(std::make_pair(result_a.first, result_b.first), result_b.second);
        }
    };
    return Parser<std::pair<A, B>>{lam};
}

template <typename A>
Parser<A> operator >>(Parser<A> parser_a, Parser<void> parser_b)
{
    auto lam = [&](std::string &in){
        std::pair<A, std::string> result_a = parser_a.run(in);
        if(result_a.first != nullptr)
        {
            std::pair<void, std::string> result_b = parser_b.run(result_a.second);
            return std::make_pair(result_a.first, result_b.second);
        }
    };
    return Parser<A>{lam};
}

template <typename B>
Parser<B> operator >>(Parser<void> parser_a, Parser<B> parser_b)
{
    auto lam = [&](std::string &in){
        std::pair<void, std::string> result_a = parser_a.run(in);
        if(result_a.first != nullptr)
        {
            std::pair<B, std::string> result_b = parser_b.run(result_a.second);
            return std::make_pair(result_b.first, result_b.second);
        }
    };
    return Parser<B>{lam};
}

template <typename A>
Parser<void> skip(Parser<A> parser)
{
    auto lam = [&](std::string &in){
        auto result = parser.run(in);
        if(result.first != nullptr)
            return std::make_pair(false, in);
    };
    return Parser<void>{lam};
}


int main()
{
    std::string str;
    str.assign((std::istreambuf_iterator<char>(std::cin)),
               std::istreambuf_iterator<char>());
    // auto result = alpha().run(str);
    auto result = alpha() >> digit() >> alpha();
    // std::cout << result.first;
}
