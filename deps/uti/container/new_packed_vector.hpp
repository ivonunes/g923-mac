//
//
//      uti
//      packed_vector.hpp
//

#pragma once

#include <type/traits.hpp>
#include <iterator/meta.hpp>
#include <iterator/base.hpp>
#include <allocator/meta.hpp>
#include <allocator/resource.hpp>


namespace uti
{


enum class packing_level : u8_t
{
        TINY  =  8 ,
        SMALL = 16 ,
        MID   = 32 ,
        LARGE = 64 ,
} ;


template< bool Signed, typename Resource >
class packed_vector
{
public:
        using t8  = conditional_t< Signed,  i8_t,  u8_t > ;
        using t16 = conditional_t< Signed, i16_t, u16_t > ;
        using t32 = conditional_t< Signed, i32_t, u32_t > ;
        using t64 = conditional_t< Signed, i64_t, u64_t > ;

        static constexpr t8   t8_max = []{ if constexpr( Signed ) return  i8_t_max ; else return  u8_t_max ; }() ;
        static constexpr t16 t16_max = []{ if constexpr( Signed ) return i16_t_max ; else return u16_t_max ; }() ;
        static constexpr t32 t32_max = []{ if constexpr( Signed ) return i32_t_max ; else return u32_t_max ; }() ;
        static constexpr t64 t64_max = []{ if constexpr( Signed ) return i64_t_max ; else return u64_t_max ; }() ;

        static constexpr t8   t8_min = []{ if constexpr( Signed ) return  i8_t_min ; else return  u8_t_min ; }() ;
        static constexpr t16 t16_min = []{ if constexpr( Signed ) return i16_t_min ; else return u16_t_min ; }() ;
        static constexpr t32 t32_min = []{ if constexpr( Signed ) return i32_t_min ; else return u32_t_min ; }() ;
        static constexpr t64 t64_min = []{ if constexpr( Signed ) return i64_t_min ; else return u64_t_min ; }() ;

        using  size_type =  size_t ;
        using ssize_type = ssize_t ;

        using    resource_type = Resource ;
        using       block_type = typename resource_type::block_type ;
        using _resource_traits = uti::resource_traits< resource_type > ;

//      using       iterator = packed_vector_iterator< packed_vector > ;
//      using const_iterator = packed_vector_iterator< packed_vector > ;

        using  t8_iterator = iterator_base<  t8, random_access_iterator_tag > ;
        using t16_iterator = iterator_base< t16, random_access_iterator_tag > ;
        using t32_iterator = iterator_base< t32, random_access_iterator_tag > ;
        using t64_iterator = iterator_base< t64, random_access_iterator_tag > ;

        using  const_t8_iterator = iterator_base<  t8 const, random_access_iterator_tag > ;
        using const_t16_iterator = iterator_base< t16 const, random_access_iterator_tag > ;
        using const_t32_iterator = iterator_base< t32 const, random_access_iterator_tag > ;
        using const_t64_iterator = iterator_base< t64 const, random_access_iterator_tag > ;
private:
        block_type  storage_ ;
        packing_level level_ ;
        ssize_type     size_ ;
} ;


} // namespace uti
