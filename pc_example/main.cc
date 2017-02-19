#include <vector>
#include <iostream>
#include <tuple>

class AST{};

template<class ReturnType>
using ParserTuple = std::pair <ReturnType, std::istream &>;

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

ParserResult<char> parseChar(char ch, std::istream &in)
{
    if(!in || in.get() != ch)
        return ParserResult<char>{};
    return ParserResult<char>{ParserTuple<char>{ch, in}};
}

int main ()
{
    
}
