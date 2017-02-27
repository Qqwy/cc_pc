#include <deque>
#include <vector>
#include <iostream>
#include <iterator>
#include <string>
#include <functional>
#include <experimental/optional>
#include <type_traits>
#include <future>

namespace Combi
{
    // Pretty printing for tuples.
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

    // Pretty printing for vectors.
    template <typename A>
    std::ostream &operator<< (std::ostream& os, std::vector<A> const &vector)
    {
        os << "std::vector{";
        std::copy(vector.begin(), vector.end(), std::ostream_iterator<A>(os));
        os << "}";
        return os;
    }

    // Pretty printing for deques.
    template <typename A>
    std::ostream &operator<< (std::ostream& os, std::deque<A> const &deque)
    {
        os << "std::deque{";
        std::copy(deque.begin(), deque.end(), std::ostream_iterator<A>(os));
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

    template <typename A>
    inline A concatenateToTuple(A const &elem_a, std::tuple<> const &)
    {
        return elem_a;
    }
    
    template <typename B>
    inline B concatenateToTuple(std::tuple<> const &, B const &elem_b)
    {
        return elem_b;
    }


    template <typename... As>
    inline std::tuple<As...> concatenateToTuple(std::tuple<As...> const &elem_a, std::tuple<> const &_elem_b)
    {
        return elem_a;
    }
    
    template <typename... Bs>
    inline std::tuple<Bs...> concatenateToTuple(std::tuple<> const &_elem_a, std::tuple<Bs...> const &elem_b)
    {
        return elem_b;
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
        {
        };

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
    using ParseResults = std::future<std::vector<ParseResult<Result>>>;

    template <typename Result>
    class Parser
    {
        typedef std::function<ParseResults<Result>(std::string const &)> ParseFunction;
        ParseFunction d_fun;
    public:
        Parser(ParseFunction fun)
        :
            d_fun(fun)
        {
        };
        ParseResults<Result> run(std::string const &in) const
        {
            auto parser_a = *this;
            auto res = std::async(std::launch::deferred, [parser_a, in](){ return parser_a.d_fun(in); });
// return d_fun(in);
            return res;
        }



        // Simple combinators:
        template <typename OtherResult>
        auto operator>>(Parser<OtherResult> const &parser_b) const -> decltype(auto)
        {
            typedef decltype(concatenateToTuple(std::declval<Result>(), std::declval<OtherResult>())) result_t;
            auto parser_a = *this;
            auto lambda = [parser_a, parser_b](std::string const &in)
            {
                std::vector<ParseResult<result_t>> results;
                ParseResults<Result> a_results = parser_a.run(in);
                for(ParseResult<Result> result_a : a_results.get())
                {
                    ParseResults<OtherResult> b_results = parser_b.run(result_a.unparsed_rest());
                    for(ParseResult<OtherResult> result_b : b_results.get())
                    {
                        auto lam = [result_a, result_b]() {return ParseResult<result_t>{concatenateToTuple(result_a.content(), result_b.content()), result_b.unparsed_rest()}; };
                        // results.push_back(std::async(std::launch::deferred, lam));
                        results.push_back(lam());
                    }

                        // results.push_back(ParseResult<result_t>{concatenateToTuple(result_a.content(), result_b.content()), result_b.unparsed_rest()});
                }
                return std::async(std::launch::deferred, [results](){ return results; });
            };
            return Parser<result_t>{lambda};
        }


        Parser<Result> operator|(Parser<Result> const &parser_b) const
        {

            auto parser_a = *this;
            auto lambda = [parser_a, parser_b](std::string const &in)
            {
                ParseResults<Result> a_results = parser_a.run(in);
                ParseResults<Result> b_results = parser_b.run(in);
                ParseResults<Result> results = a_results;
                results.insert(results.end(), b_results.begin(), b_results.end());
                return results;
            };
            return Parser<Result>{lambda};
        }

        template<typename Function>
        auto transform(Function fun) const -> Parser<decltype(fun(std::declval<Result>()))>
        {
            auto parser_a = *this;
            auto lambda = [parser_a, fun](std::string const &in)
            {
                ParseResults<Result> a_results = parser_a.d_fun(in);
                ParseResults<decltype(fun(std::declval<Result>()))> results;
                for(ParseResult<Result> result_a : a_results)
                {
                    results.push_back(ParseResult<decltype(fun(std::declval<Result>()))>{fun(result_a.content()), result_a.unparsed_rest()});
                }
                return results;
            };
            return Parser<decltype(fun(std::declval<Result>()))>{lambda};
        };
    };

    // Helper functions we do not want to export
    namespace {
        template <typename A>
        std::deque<A> combineTupleToDeque(std::tuple<A, std::deque<A>> tuple)
        {
            std::deque<A> vec = std::get<1>(tuple);
            vec.push_front(std::get<0>(tuple));
            return vec;
        }
    }

    // Simple parsers

    // Returns matched character if given predicate function returns true.
    template <typename Fun>
    Parser<char> satisfies(Fun const &fun)
    {
        auto lambda = [fun](std::string const &in)
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


    // Matches if input starts with given character.
    Parser<char> ch(char the_char)
    {
        return satisfies([the_char](char real_char){return the_char == real_char;});
    }

    // Matches if input starts with given string.
    Parser<std::string> string(std::string const &str)
    {
        auto lambda = [str](std::string const &in)
        {
            std::cout << "Checks if " << str << " is part of " << in << "\n";
            if(in.find(str) == 0)
                return ParseResults<std::string>{ParseResult<std::string>{str, in.substr(str.size(), in.size())}};
            return ParseResults<std::string>{};
        };
        return Parser<std::string>{lambda};
    }

    // Does not consume input, returns given value.
    template <typename TAnything>
    Parser<TAnything> unit(TAnything const &value)
    {
        auto lambda = [value](std::string const &in){
            return ParseResults<TAnything>{ParseResult<TAnything>{value, in}};
        };
        return Parser<TAnything>{lambda};
    }

    // Returns an empty deque.
    template <typename A>
    Parser<std::deque<A>> nothing()
    {
        return unit(std::deque<A>{});
    };

    // Runs parser A, but returns an empty tuple
    // (which is discarded when sequenced together with other parsers)
    template <typename A>
    Parser<std::tuple<>> skip(Parser<A> const &parser_a)
    {
        auto transformation = [parser_a](A const &)
        {
            return std::tuple<>{};
        };
        return parser_a.transform(transformation);
    }

    // Returns all results of running `parser_a` zero or more times.
    template <typename A>
    Parser<std::deque<A>> many(Parser<A> const &parser_a)
    {
        auto lambda = [parser_a](std::string const &in)
        {
            return many(parser_a).run(in);
        };
       return (parser_a >> Parser<std::deque<A>>{lambda}).transform(&combineTupleToDeque<A>) | nothing<A>();
    }

    template <typename A>
    Parser<std::deque<A>> many1(Parser<A> const &parser_a)
    {
        return (parser_a >> many(parser_a)).transform(&combineTupleToDeque<A>);
    }

    static const Parser<char> space = satisfies([](char ch){return ch == ' ';});
    static const Parser<char> whitespace = satisfies([](char ch){return ::isblank(ch);});
    static const Parser<std::deque<char>> whitespaces = many(whitespace);
    static const Parser<char> digit = satisfies([](char ch){return ::isdigit(ch);});
    static const Parser<char> alpha = satisfies([](char ch){return ::isalpha(ch);});
    static const Parser<char> alnum = digit | alpha;
    static const Parser<int>  digit2 = digit.transform([](char ch){ return ch - '0';});
    static const Parser<std::deque<char>> digits = many1(digit);

    template <typename A>
    Parser<A> lexeme(Parser<A> const &parser_a)
    {
        return parser_a >> skip(whitespaces);
    }

    static const Parser<std::deque<char>> integer_str = lexeme(digits);
}

Combi::Parser<std::tuple<char, char, char>> myParser()
{
    using namespace Combi;
    return digit >> alpha >> alpha;
}

int main()
{

    using namespace Combi;
    // auto digit_parser = many(digit);
    std::string input;
    input.assign((std::istreambuf_iterator<char>(std::cin)),
                 std::istreambuf_iterator<char>());

    // auto parser = digit >> alpha >> alpha;
    // auto parser = myParser();
    // auto parser = (digit | alpha) >> digit2 >> digit2;//(digit() >> digit()) >> (digit() >> digit());
    // auto parser = many(digit);
    // auto parser = integer_str;
    auto parser = string("foo") >> digits;
    // auto digit_parser = digit >> digit >> digit >> digit >> digit;
    std::cout << "TEST\n";
    auto parse_results = parser.run(input);
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
