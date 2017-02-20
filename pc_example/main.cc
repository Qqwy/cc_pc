#include <vector>
#include <iostream>
// #include <tuple>
#include <functional>

class AST{};

// template<class ReturnType>
// using ParserTuple = std::pair <ReturnType, std::istream >;

// template <class ReturnType>
// using ParserResult = std::vector<ParserTuple<ReturnType>>;
template <class ReturnType>
using ParserResult = std::vector<ReturnType>;

// ParserResult<int> parseInt(std::istream &in){
//     if(in)
//     {
//         int myInt;
//         in >> myInt;
//         return ParserResult<int>{ParserTuple<int>{myInt, in}};
//     }
//     else
//         return ParserResult<int>{};
// }

template <class ReturnType>
ParserResult<ReturnType> parsePrim(std::istream &in)
{
    if(!in)
        return ParserResult<ReturnType>{};
    ReturnType val;
    in >> val;
    return ParserResult<ReturnType>{val};
}

ParserResult<char> eatChar(std::istream &in)
{
    if(!in)
        return ParserResult<char>{};
    char ch;
    in.get(ch);
    return ParserResult<char>{ch};
}


template<class ReturnType>
ParserResult<ReturnType> unit(ReturnType &rt, std::istream &)
{
    return rt;
}

template <class ReturnType1, class ReturnType2>
ParserResult<ReturnType2> combine(std::function<ParserResult<ReturnType1>(std::istream &in)> parser1, std::function<ParserResult<ReturnType2>(ReturnType1, std::istream &in)> parser2, std::istream &in)
{
    ParserResult<ReturnType1> half_result = parser1(in);
    ParserResult<ReturnType2> result{};
    for(ReturnType1 result_elem : half_result)
    {
        ParserResult<ReturnType2> p2_result = parser2(result_elem, in);
        result.insert(result.end(), p2_result.begin(), p2_result.end());
    }
    return result;
}

template <typename Result>
struct Parser
{
    std::tuple<> operator()(std::string &in)
        {
            
        };
};

template<typename T1, typename F>
typename std::result_of<F(T1)>::type
operator>>=(Parser<T1> t, F&& f) {

    return std::forward<F>(f)(t.data);
}

ParserResult<char> matchChar(char ch, std::istream &in)
{
    return combine<char, char>(eatChar, [&](char real_char, std::istream &){
            if(ch != real_char)
                return ParserResult<char>{};
            return ParserResult<char>{ch};
        }, in);
}


int main ()
{
    matchChar('a', std::cin);
    matchChar('b', std::cin);
    int a, b, c;
    std::cin >> a = b = c;
}
