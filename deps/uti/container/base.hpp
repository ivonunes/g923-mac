//
//
//      uti
//      container_base.hpp
//

#pragma once

#include <type/traits.hpp>
#include <iterator/meta.hpp>
#include <iterator/base.hpp>
#include <iterator/reverse_iterator.hpp>


namespace uti
{


template< typename T >
class _container_base
{
public:
        using      value_type =       T    ;
        using       size_type =  size_t    ;
        using      ssize_type = ssize_t    ;
        using difference_type = ssize_type ;

        using         pointer = value_type       * ;
        using   const_pointer = value_type const * ;
        using       reference = value_type       & ;
        using const_reference = value_type const & ;

        using               iterator = iterator_base< value_type      , random_access_iterator_tag > ;
        using         const_iterator = iterator_base< value_type const, random_access_iterator_tag > ;
        using       reverse_iterator = ::uti::reverse_iterator<       iterator > ;
        using const_reverse_iterator = ::uti::reverse_iterator< const_iterator > ;
};


} // namespace uti
