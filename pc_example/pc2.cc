#include <vector>
#include <iostream>
#include <iterator>
#include <string>
#include <functional>
#include <experimental/optional>
#include <type_traits>


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
        };
    inline Content content() const &
        {
            if(!d_success)
                throw "Error: Unsuccsessfull parse.!";
            return d_content;
        }
    inline operator bool(){
        return d_success;
    }
};

template <typename Result>
class Parser
{
    typedef std::function<ParseResult<Result>(std::string const &)> ParseFunction;
    ParseFunction d_fun;
public:
    Parser(ParseFunction fun)
        : d_fun(fun)
        {};
    ParseResult<Result> run(std::string const &in) const
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

template <typename Fun>
Parser<char> satisfies(Fun const &fun)
{
    auto lambda = [&](std::string const &in)
        {
            if(in.empty())
                return ParseResult<char>{};
            char ch = in[0];
            if(!fun(ch))
                return ParseResult<char>{};
            return ParseResult<char>{ch, in.substr(1, in.size())};
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
