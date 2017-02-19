#include <vector>
#include <iostream>
#include <tuple>
#include <functional>

class AST{};

template<class ReturnType>
using ParserTuple = std::pair <ReturnType, std::istream >;

template <class ReturnType>
using ParserResult = std::vector<ParserTuple<ReturnType>>;

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
    return ParserResult<ReturnType>{ParserTuple<ReturnType>{val, in}};
}

ParserResult<char> eatChar(std::istream &in)
{
    if(!in)
        return ParserResult<char>{};
    return ParserResult<char>{ParserTuple<char>{in.get(), in}};
}


template<class ReturnType>
ParserResult<ReturnType> unit(ReturnType &rt, std::istream &)
{
    return rt;
}

template <class ReturnType1, class ReturnType2>
ParserResult<ReturnType2> bind(std::function<ParserResult<ReturnType1>(std::istream &in)> parser1, std::function<ParserResult<ReturnType2>(ReturnType1, std::istream &in)> parser2, std::istream &in)
{
    ParserResult<ReturnType1> half_result = parser1(in);
    ParserResult<ReturnType2> result{};
    for(ParserTuple<ReturnType1> result_tuple : half_result)
    {
        ParserResult<ReturnType2> p2_result = parser2(result_tuple.first, in);
        result.insert(result.end(), p2_result.begin(), p2_result.end());
    }
    return result;
}

ParserResult<char> matchChar(char ch, std::istream &in)
{
    // if(!in || in.get() != ch)
    //     return ParserResult<char>{};
    // return ParserResult<char>{ParserTuple<char>{ch, in}};
    // ParserResult<char> temp = eatChar(in);
    // if(temp.empty() || temp[0].first != ch)
    //     return ParserResult<char>{};
    // return ParserResult<char>{ParserTuple<char>{ch, in}};


    return bind<char, char>(eatChar, [&](char real_char, std::istream &in){
            if(ch != real_char)
                return ParserResult<char>{};
            return ParserResult<char>{ParserTuple<char>{ch, in}};
        }, in);
}


int main ()
{
    matchChar('a', std::cin);
    matchChar('b', std::cin);
}
