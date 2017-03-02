#include "parser/parser.h"
#include "parsers.h"

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
    auto parser = float_str2();
    // auto parser = integer_str;
    // auto parser = string("foo") >> digits;
    // auto digit_parser = digit >> digit >> digit >> digit >> digit;
    std::tuple<int, int, int, int, int> tup = std::make_tuple(1,2,3,4,5);
    std::deque<int> deq = tupleToDeque(tup);
    std::cout << deq << '\n';
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
