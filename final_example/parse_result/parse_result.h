#ifndef PARSE_RESULT
#define PARSE_RESULT

#include <deque>
#include <vector>
#include <iostream>
#include <iterator>
#include <string>
#include <functional>
#include <experimental/optional>
#include <type_traits>



// Core classes
template <typename Content>
class ParseResult
{
    Content d_content;
    std::string d_unparsed_rest;

public:

ParseResult(Content const &content, std::string const &rest)
    :
    d_content(content),
        d_unparsed_rest(rest)
        {
        };

    inline std::string unparsed_rest() const
    {
        return d_unparsed_rest;
    }

    inline Content content() const &
    {
        return d_content;
    }
};
template <typename Result>
using ParseResults = std::vector<ParseResult<Result>>;

#endif
