#include "parser/parser.h"

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

    Parser<std::deque<char>> chd(char the_char)
    {
        return ch(the_char).singletonDeque();
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
    template <typename TResult>
    Parser<std::deque<TResult>> nothing()
    {
        return unit(std::deque<TResult>{});
    };

    template <typename A>
    Parser<std::deque<A>> maybe(Parser<A> parser_a)
    {
        auto transformation = [](A const &val)
            {
                std::deque<A> deque;
                deque.push_back(val);
                return deque;
            };
        return parser_a.transform(transformation) | nothing<A>();
    }

    template <typename TResult>
    Parser<std::deque<TResult>> maybe(Parser<std::deque<TResult>> parser_a)
    {
        return parser_a | nothing<TResult>();
    }


    // Runs parser A, but returns an empty tuple
    // (which is discarded when sequenced together with other parsers)
    template <typename A>
    Parser<std::tuple<>> skip(Parser<A> const &parser_a)
    {
        auto transformation = [](A const &)
        {
            return std::tuple<>{};
        };
        return parser_a.transform(transformation);
    }

    // Returns all results of running `parser_a` zero or more times.
    template <typename TResult>
    Parser<std::deque<TResult>> many(Parser<TResult> const &parser_a)
    {
        auto lambda = [parser_a](std::string const &in)
        {
            return many(parser_a).run(in);
        };
        Parser<std::deque<TResult>> one_or_more = (parser_a >> Parser<std::deque<TResult>>{lambda}).concat2deq();
        return one_or_more | nothing<TResult>();
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

    static const Parser<std::deque<char>> integer_str = (maybe(ch('-')) >> digits).tup2deq();

    namespace {
        Parser<std::deque<char>> float_exponent = ((ch('e') | ch('E')).singletonDeque() >> maybe(ch('+') | ch('-')) >> digits).tup2deq();
    }
    static const Parser<std::deque<char>> float_str =
                                        (integer_str >> ch('.').singletonDeque() >> digits >> maybe(float_exponent)).tup2deq();

    Parser<std::deque<char>> integer_str2()
    {
        return (maybe(ch('-')) >> digits).tup2deq();
    }


    Parser<std::deque<char>> float_str2()
    {
        auto plus_or_min = chd('+') | chd('-');
        Parser<std::deque<char>> float_exponent = (
            (chd('e') | chd('E')) >> maybe(plus_or_min) >> digits).tup2deq();
        return (
            integer_str2
            >> chd('.')
            >> digits
            >> maybe(float_exponent)
        ).tup2deq();
    }
}
