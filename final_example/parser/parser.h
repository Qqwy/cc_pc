#ifndef PARSER_H
#define PARSER_H

#include "../parse_result/parse_result.h"
#include "helpers.h"

namespace Combi{

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
            return d_fun(in);
        }


        // Simple combinators:
        template <typename OtherResult>
        auto operator>>(Parser<OtherResult> const &parser_b) const -> decltype(auto)
        {
            typedef decltype(concatenateToTuple(std::declval<Result>(), std::declval<OtherResult>())) result_t;
            auto parser_a = *this;
            auto lambda = [parser_a, parser_b](std::string const &in)
            {
                ParseResults<result_t> results;
                std::vector<ParseResult<Result>> a_results = parser_a.run(in);
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
        auto transform(Function fun) const // -> Parser<decltype(fun(std::declval<Result>()))>
        {
            auto parser_a = *this;
            auto lambda = [parser_a, fun](std::string const &in)
            {
                ParseResults<Result> a_results = parser_a.run(in);
                ParseResults<decltype(fun(std::declval<Result>()))> results;
                for(ParseResult<Result> result_a : a_results)
                {
                    results.push_back(ParseResult<decltype(fun(std::declval<Result>()))>{fun(result_a.content()), result_a.unparsed_rest()});
                }
                return results;
            };
            return Parser<decltype(fun(std::declval<Result>()))>{lambda};
        };

        template <bool IsTuple = is_specialization_of<Result, std::tuple>::value>
        auto tup2deq() const
        {
            auto lambda = [](Result content){
                return concatDequeTuple(content);
            };
            return this->transform(lambda);
        };

        template <bool IsTuple = is_specialization_of<Result, std::tuple>::value>
        auto concat2deq() const
        {
            auto lambda = [](Result content){
                return combineTupleToDeque(content);
            };
            return this->transform(lambda);
        };

        Parser<std::deque<Result>> singletonDeque() const
        {
            return this->transform(&::singletonDeque<Result>);
        };

    };


    template <typename TResultA, typename TResultB>
    auto operator>>(Parser<TResultA> (*parser_a)(), Parser<TResultB> const &parser_b)
    {
        return parser_a() >> parser_b;
    }
    
    template <typename TResultA, typename TResultB>
    auto operator>>(Parser<TResultA> const &parser_a, Parser<TResultB> (*parser_b)())
    {
        return parser_a >> parser_b();
    }

}

#endif
