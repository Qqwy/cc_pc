#include <vector>
#include <iostream>
#include <iterator>
#include <string>
#include <functional>
#include <experimental/optional>
#include <type_traits>

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
        {
            std::cout << "Parser " << this << " was created!\n";
        };
        ParseResults<Result> run(std::string const &in) const
        {
            return d_fun(in);
        }

        ~Parser()
        {
            std::cout << "Parser " << this << " is getting destroyed!\n";
        }



        // Simple combinators:

        template <typename OtherResult>
        auto operator>>(Parser<OtherResult> const &parser_b) const -> decltype(auto)
        {
            typedef decltype(concatenateToTuple(std::declval<Result>(), std::declval<OtherResult>())) result_t;
            std::cout << this << " Running operator>> \n";
            auto lambda = [this, parser_b](std::string const &in)
            {
                std::cout << this << " Running operator>> lambda \n";
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


        Parser<Result> operator|(Parser<Result> const &parser_b) const
        {

            std::cout << this << " Running operator| \n";
            auto lambda = [this, parser_b](std::string const &in)
            {
                std::cout << this << " Running operator| lambda \n";
                ParseResults<Result> a_results = this->run(in);
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
            std::cout << this << " Running transform \n";
            auto lambda = [this, fun](std::string const &in)
            {
                std::cout << "Running transform lambda\n";
                ParseResults<Result> a_results = this->d_fun(in);
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


    // More Combinators

    // TODO: Fix this.
    // template <typename A>
    // Parser<std::vector<A>> many(Parser<A> const &parser_a)
    // {
    //     auto lam = [&](std::string in)
    //     {
    //         std::vector<A> vec{};
    //         while (true)
    //         {
    //             auto parse_result = parser_a.run(in);
    //             if(!parse_result)
    //                 break;
    //             vec.push_back(parse_result.content());
    //             in = parse_result.unparsed_rest();
    //         }
    //         return ParseResult<std::vector<A>>{vec, in};
    //     };
    //     return Parser<std::vector<A>>{lam};
    // }



    // Simple parsers

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

    static const Parser<char> space = satisfies([](char ch){return ch == ' ';});
    static const Parser<char> digit = satisfies([](char ch){return ::isdigit(ch);});
    static const Parser<char> alpha = satisfies([](char ch){return ::isalpha(ch);});
    // static const Parser<char> alnum = digit | alpha;
    static const Parser<int>  digit2 = digit.transform([](char ch){ return ch - '0';});

    Parser<char> ischar(char the_char)
    {
        return satisfies([the_char](char real_char){return the_char == real_char;});
    }
    // static const Parser<std::vector<char>> digits = many(digit);

    template <typename TAnything>
    Parser<TAnything> unit(TAnything const &value)
    {
        std::cout << "Running unit \n";
        auto lambda = [value](std::string const &in){
            std::cout << "Running unit lambda \n";
            return ParseResults<TAnything>{ParseResult<TAnything>{value, in}};
        };
        return Parser<TAnything>{lambda};
    }

    template <typename A>
    Parser<std::vector<A>> nothing()
    {
        std::cout << "Running nothing \n";
        return unit(std::vector<A>{});
    };

    template <typename A>
    Parser<std::tuple<>> skip(Parser<A> const &parser_a)
    {
        auto transformation = [parser_a](A const & _value)
        {
            return std::tuple<>{};
        };
        return parser_a.transform(transformation);
    }

    // template <typename A>
    // Parser<std::vector<A>> many(Parser<A> const &parser_a)
    // {
    //     auto lambda = [&](std::string const &in)
    //         {
    //             // ParseResults<A> results = parser_a.parse(in);
    //             ParseResults<std::tuple<A, std::vector<A>>> temp_results = (parser_a >> many(parser_a)).run(in);
    //             ParseResults<std::vector<A>> real_results;
    //             for(ParseResult<std::tuple<A, std::vector<A>>> result : temp_results)
    //             {
    //                 std::vector<A> result_vec;
    //                 A first_elem = std::get<0>(result.content());
    //                 std::cout << "FIRST ELEM: " << first_elem << '\n';
    //                 std::vector<A> rest_elems = std::get<1>(result.content());
    //                 std::cout << "REST: " << rest_elems << '\n';
    //                 result_vec.push_back(first_elem);
    //                 result_vec.insert(result_vec.end(), rest_elems.begin(), rest_elems.end());
    //                 real_results.push_back(ParseResult<std::vector<A>>(result_vec, result.unparsed_rest()));
    //             }
    //             return ParseResults<std::vector<A>>{real_results};
    //         };
    //     return Parser<std::vector<A>>{lambda} | nothing<A>();
    // }
    template <typename A>
    // Parser<std::vector<A>> many(Parser<A> const &parser_a)

    Parser<std::tuple<A, std::vector<A>>> many(Parser<A> const &parser_a)
    {
        auto lambda = [parser_a](std::string const &in)
        {
            std::cout << "Running recursively!\n";
            return many(parser_a).run(in);
        };
        auto transformation = [](std::tuple<A, std::vector<A>> foo)
        {
            std::cout << "Running many transformation lambda \n";
            return std::get<1>(foo);
        };
        Parser<std::vector<A>> foo{lambda};
        std::cout << "foo: " &foo << '\n';
        std::cout << "parser_a: " &parser_a << '\n';
        Parser<std::tuple<A, std::vector<A>>> result = parser_a >> foo;
        std::cout << "result: " &result << '\n';
        return (result);//.transform(transformation); // | nothing<A>();
    }
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

    // auto digit_parser = digit >> alpha >> alpha;
    auto digit_parser = myParser();
    // auto digit_parser = (digit | alpha) >> digit2 >> digit2;//(digit() >> digit()) >> (digit() >> digit());
    // auto digit_parser = nothing<char>();
    // auto digit_parser = digit >> digit >> digit >> digit >> digit;
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
