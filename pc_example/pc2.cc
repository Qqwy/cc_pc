#include <vector>
#include <iostream>
#include <iterator>
#include <string>
#include <functional>
#include <experimental/optional>
#include <type_traits>






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


// Concatenate two datatypes to a tuple.
// If one or both already is a tuple, prepend/append the other.
// If both are tuples, combine them.
template <typename A, typename B>
inline std::tuple<A, B> concatenateToTuple(A const &elem_a, B const &elem_b)
{
    return std::make_tuple(elem_a, elem_b);
}

template <typename A, typename... Bs>
inline std::tuple<A, Bs...> concatenateToTuple(A const &elem_a, std::tuple<Bs...> const &elem_b)
{
    return std::tuple_cat(std::make_tuple(elem_a), elem_b);
}

template <typename... As, typename B>
inline std::tuple<As..., B> concatenateToTuple(std::tuple<As...> const &elem_a, B const &elem_b)
{
    return std::tuple_cat(elem_a, std::make_tuple(elem_b));
}
template <typename... As, typename... Bs>
inline std::tuple<As..., Bs...> concatenateToTuple(std::tuple<As...> const &elem_a, std::tuple<Bs...> const &elem_b)
{
    return std::tuple_cat(elem_a, elem_b);
}

// Core classes
template <typename Content>
class ParseResult
{
    bool d_success = false;
    Content d_content;
    std::string d_unparsed_rest;

public:

    ParseResult(){};
    ParseResult(Content const &content, std::string const &rest)
    :
        d_success(true),
        d_content(content),
        d_unparsed_rest(rest)
    {};

    inline std::string unparsed_rest() const
    {
        if(!d_success)
            throw "Error: Unsuccsessfull parse.!";
        return d_unparsed_rest;
    }

    inline Content content() const &
    {
        if(!d_success)
            throw "Error: Unsuccsessfull parse.!";
        return d_content;
    }

    operator bool()
    {
        return d_success;
    }
};
template <typename Result>
using ParseResults = std::vector<ParseResult<Result>>;



template <typename Result>
class Parser
{
    typedef std::function<ParseResults<Result>(std::string const &)> ParseFunction;
    ParseFunction d_fun;
public:
    Parser(ParseFunction fun)
    :
        d_fun(fun)
    {};
    ParseResults<Result> run(std::string const &in) const
    {
        return d_fun(in);
    }

    template <typename OtherResult>
    auto sequence(Parser<OtherResult> const &parser_b) const -> decltype(auto)
    {
        typedef decltype(concatenateToTuple(std::declval<Result>(), std::declval<OtherResult>())) result_t;
        auto lambda = [&](std::string const &in)
        {
            ParseResults<result_t> results;
            std::vector<ParseResult<Result>> a_results = this->run(in);
            for(ParseResult<Result> result_a : a_results)
            {
                ParseResults<OtherResult> b_results = parser_b.run(result_a.unparsed_rest());
                for(ParseResult<OtherResult> result_b : b_results)
                    results.push_back(ParseResult<result_t>{concatenateToTuple(result_a.content(), result_b.content()), result_b.unparsed_rest()});
            }
            return results;
        };
        return Parser<result_t>{lambda};
    }
};



// Simple parsers

template <typename Fun>
Parser<char> satisfies(Fun const &fun)
{
    auto lambda = [&](std::string const &in)
        {
            if(in.empty())
                return ParseResults<char>{};
            char ch = in[0];
            if(!fun(ch))
                return ParseResults<char>{};
            return ParseResults<char>{ParseResult<char>{ch, in.substr(1, in.size())}};
        };
    return Parser<char>{lambda};
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

// Combinators

template<typename A, typename B>
auto operator>>(Parser<A> parser_a, Parser<B> parser_b) -> decltype(std::declval<Parser<A>>().sequence(std::declval<Parser<B>>()))
{
    return parser_a.sequence(parser_b);
}

template <typename A>
std::ostream & operator<<(std::ostream &os, Parser<A> &parser_a)
{
    os << "Parser{}";
    return os;
}

// TODO: Fix this.
template <typename A>
Parser<std::vector<A>> many(Parser<A> const &parser_a)
{
    auto lam = [&](std::string in)
        {
            std::vector<A> vec{};
            while (true)
            {
                auto parse_result = parser_a.run(in);
                if(!parse_result)
                    break;
                vec.push_back(parse_result.content());
                in = parse_result.unparsed_rest();
            }
            return ParseResult<std::vector<A>>{vec, in};
        };
    return Parser<std::vector<A>>{lam};
}

int main()
{
    std::string input;
    input.assign((std::istreambuf_iterator<char>(std::cin)),
                 std::istreambuf_iterator<char>());

    // auto digit_parser = digit() >> alpha() >> alpha();
    auto digit_parser = (digit() >> digit()) >> (digit() >> digit());
    auto parse_results = digit_parser.run(input);
    if(!parse_results.empty())
    {
        for(auto parse_result : parse_results)
        {
        std::cout << "Parse success!\n";
        std::cout << parse_result.content() << '\n';
        std::cout << "Rest of string: " << parse_result.unparsed_rest();
        }
    }
    else
    {
        std::cout << "Parsing failed!\n";
    }
}
