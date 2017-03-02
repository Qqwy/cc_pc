#include <deque>
#include <vector>
#include <iostream>
#include <iterator>
#include <string>
#include <functional>
#include <experimental/optional>
#include <type_traits>

namespace {

    template <typename TResult>
    std::deque<TResult> combineTupleToDeque(std::tuple<TResult, std::deque<TResult>> tuple)
    {
        std::deque<TResult> vec = std::get<1>(tuple);
        vec.push_front(std::get<0>(tuple));
        return vec;
    }

    template<typename T>
    std::deque<T> flatten(const std::deque<std::deque<T>> &orig)
    {
        std::deque<T> ret;
        for(const auto &v: orig)
            ret.insert(ret.end(), v.begin(), v.end());
        return ret;
    }

    template<typename first_type, typename tuple_type, size_t ...index>
    auto tupleToDequeImpl(tuple_type const &tuple, std::index_sequence<index...>)
    {
        return std::deque<first_type>{
            std::get<index>(tuple)...
                };
    }

    template<typename first_type, typename ...others>
    auto tupleToDeque(std::tuple<first_type, others...> const &tuple)
    {
        typedef typename std::remove_reference<decltype(tuple)>::type tuple_type;

        constexpr auto size =
            std::tuple_size<tuple_type>::value;

        return tupleToDequeImpl<first_type, tuple_type>
            (tuple, std::make_index_sequence<size>{});
    }

    template <typename A, typename ...As>
    auto concatDequeTuple(std::tuple<std::deque<A>, As...> const &tuple)
    {
        std::deque<std::deque<A>> nested_deque = tupleToDeque(tuple);
        return flatten(nested_deque);
    }

    template <typename A>
    std::deque<A> singletonDeque(A const &elem)
    {
        return std::deque<A>{elem};
    }
}



namespace Combi
{
    //! Tests if T is a specialization of Template
    template <typename T, template <typename...> class Template>
    struct is_specialization_of : std::false_type {};
    template <template <typename...> class Template, typename... Args>
    struct is_specialization_of<Template<Args...>, Template> : std::true_type {};


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

}
