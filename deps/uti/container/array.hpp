//
//
//      uti
//      array.hpp
//

#pragma once

#include <type/traits.hpp>
#include <container/base.hpp>
#include <container/view.hpp>
#include <allocator/meta.hpp>

#ifdef UTI_HAS_STL
#include <initializer_list>
#endif // UTI_HAS_STL


namespace uti
{


template< typename T, ssize_t Capacity >
class array : public view< T >
{
        using      _self =  array               ;
        using      _base = _container_base< T > ;
        using _view_base =  view          < T > ;

        static constexpr i64_t capacity_ { Capacity } ;
public:
        using      value_type = typename _base::     value_type ;
        using       size_type = typename _base::      size_type ;
        using      ssize_type = typename _base::     ssize_type ;
        using difference_type = typename _base::difference_type ;

        using         pointer = typename _base::        pointer ;
        using   const_pointer = typename _base::  const_pointer ;
        using       reference = typename _base::      reference ;
        using const_reference = typename _base::const_reference ;

        using        iterator = typename _base::       iterator ;
        using  const_iterator = typename _base:: const_iterator ;

        constexpr array () noexcept : _view_base( data, data + capacity_ ) {}

        constexpr array ( value_type const & _val_ ) noexcept( is_nothrow_copy_assignable_v< value_type > ) ;

#ifdef UTI_HAS_STL
        constexpr array ( std::initializer_list< value_type > _list_ ) : _view_base( data, data + capacity_ )
        {
                if constexpr( is_trivially_copy_constructible_v< value_type > )
                {
                        UTI_CEXPR_ASSERT( _list_.size() <= capacity_, "uti::array: excess elements in initializer list" ) ;
                        UTI_CEXPR_ASSERT( _list_.size() >= capacity_, "uti::array: missing elements in initializer list" ) ;

                        ssize_type pos = 0 ;

                        for( auto const & val : _list_ )
                        {
                                _view_base::at( pos++ ) = val ;
                        }
                }
                else
                {

                }
        }
#endif // UTI_HAS_STL

        constexpr array             ( array const & _other_ ) noexcept( is_nothrow_copy_assignable_v< value_type > ) ;
        constexpr array & operator= ( array const & _other_ ) noexcept( is_nothrow_copy_assignable_v< value_type > &&
                                                                        is_nothrow_destructible_v   < value_type > ) ;

        constexpr array             ( array && _other_ ) noexcept( is_nothrow_move_assignable_v< value_type > ) ;
        constexpr array & operator= ( array && _other_ ) noexcept( is_nothrow_move_assignable_v< value_type > &&
                                                                   is_nothrow_destructible_v   < value_type > ) ;

        constexpr ~array () noexcept = default ;

        constexpr void fill ( value_type const & _val_ ) noexcept( is_nothrow_copy_assignable_v< value_type > &&
                                                                   is_nothrow_destructible_v   < value_type > ) ;

        UTI_NODISCARD ssize_type capacity () const noexcept { return capacity_ ; }
private:
        value_type data[ capacity_ ] {} ;
};


template< typename T, ssize_t Capacity >
constexpr
array< T, Capacity >::array ( value_type const & _val_ ) noexcept( is_nothrow_copy_assignable_v< value_type > )
        : _view_base( data, capacity_ )
{
        for( ssize_type i = 0; i < capacity_; ++i )
        {
                _view_base::at( i ) = _val_;
        }
}

template< typename T, ssize_t Capacity >
constexpr
array< T, Capacity >::array ( array const & _other_ ) noexcept( is_nothrow_copy_assignable_v< value_type > )
        : _view_base( data, capacity_ )
{
        for( ssize_type i = 0; i < capacity_; ++i )
        {
                _view_base::at( i ) = _other_.at( i );
        }
}

template< typename T, ssize_t Capacity >
constexpr
array< T, Capacity >::array ( array && _other_ ) noexcept( is_nothrow_move_assignable_v< value_type > )
        : _view_base( data, capacity_ )
{
        for( ssize_type i = 0; i < capacity_; ++i )
        {
                _view_base::at( i ) = UTI_MOVE( _other_.at( i ) );
        }
}

template< typename T, ssize_t Capacity >
constexpr array< T, Capacity > &
array< T, Capacity >::operator= ( array const & _other_ ) noexcept( is_nothrow_copy_assignable_v< value_type > &&
                                                                    is_nothrow_destructible_v   < value_type > )
{
        for( ssize_type i = 0; i < capacity_; ++i )
        {
                _view_base::at( i ) = _other_.at( i );
        }
        return *this ;
}

template< typename T, ssize_t Capacity >
constexpr array< T, Capacity > &
array< T, Capacity >::operator= ( array && _other_ ) noexcept( is_nothrow_move_assignable_v< value_type > &&
                                                               is_nothrow_destructible_v   < value_type > )
{
        for( ssize_type i = 0; i < capacity_; ++i )
        {
                _view_base::at( i ) = UTI_MOVE( _other_.at( i ) );
        }
        return *this ;
}


} // namespace uti
