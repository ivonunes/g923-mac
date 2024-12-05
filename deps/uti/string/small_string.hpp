//
//
//      uti
//      small_string.hpp
//

#pragma once

#include <type/traits.hpp>
#include <meta/concepts.hpp>
#include <iterator/meta.hpp>
#include <iterator/base.hpp>
#include <iterator/reverse_iterator.hpp>
#include <container/meta.hpp>
#include <algo/mem.hpp>
#include <algo/distance.hpp>


namespace uti
{


template< ssize_t Capacity, meta::trivial CharType = char >
class small_string
{
        static constexpr ssize_t capacity_ { Capacity } ;
public:
        using      value_type =   CharType ;
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

        constexpr small_string ( const_pointer _cstr_ ) noexcept ;

        template< meta::forward_iterator Iter >
                requires meta::convertible_to< iter_value_t< Iter >, value_type >
        constexpr small_string ( Iter _begin_, Iter const _end_ ) noexcept ;

        template< meta::simple_container Other >
                requires meta::convertible_to< typename Other::value_type, value_type >
        constexpr small_string ( Other const & _other_ ) noexcept ;

        constexpr small_string             ( small_string const &  ) noexcept = default ;
        constexpr small_string             ( small_string       && ) noexcept = default ;
        constexpr small_string & operator= ( small_string const &  ) noexcept = default ;
        constexpr small_string & operator= ( small_string       && ) noexcept = default ;

        constexpr ~small_string () noexcept = default ;

        constexpr void push_back ( value_type const & _char_ ) noexcept ;

        template< typename... Args >
        constexpr void emplace_back ( Args&&... _args_ ) noexcept ;

        constexpr void insert ( value_type const & _char_, ssize_type _position_ ) noexcept ;

        template< meta::forward_iterator Iter >
                requires meta::convertible_to< iter_value_t< Iter >, value_type >
        constexpr void append ( Iter _begin_, Iter const _end_ ) noexcept ;

        template< meta::simple_container Other >
                requires meta::convertible_to< typename Other::value_type, value_type >
        constexpr void append ( Other const & _other_ ) noexcept ;

        UTI_NODISCARD constexpr ssize_type capacity () const noexcept { return  capacity_ ; }
        UTI_NODISCARD constexpr ssize_type     size () const noexcept { return      size_ ; }
        UTI_NODISCARD constexpr bool          empty () const noexcept { return size_ == 0 ; }

        UTI_NODISCARD constexpr       reference operator[] ( ssize_type _index_ )       noexcept { return data_[ _index_ ] ; }
        UTI_NODISCARD constexpr const_reference operator[] ( ssize_type _index_ ) const noexcept { return data_[ _index_ ] ; }

        UTI_NODISCARD constexpr       reference at ( ssize_type _index_ )       noexcept { return data_[ _index_ ] ; }
        UTI_NODISCARD constexpr const_reference at ( ssize_type _index_ ) const noexcept { return data_[ _index_ ] ; }

        UTI_NODISCARD constexpr       iterator  begin ()       noexcept { return data_ ; }
        UTI_NODISCARD constexpr const_iterator  begin () const noexcept { return data_ ; }
        UTI_NODISCARD constexpr const_iterator cbegin () const noexcept { return begin() ; }

        UTI_NODISCARD constexpr       iterator  end ()       noexcept { return data_ + size_ ; }
        UTI_NODISCARD constexpr const_iterator  end () const noexcept { return data_ + size_ ; }
        UTI_NODISCARD constexpr const_iterator cend () const noexcept { return end() ; }

        UTI_NODISCARD constexpr       reverse_iterator  rbegin ()       noexcept { return  --end() ; }
        UTI_NODISCARD constexpr const_reverse_iterator  rbegin () const noexcept { return  --end() ; }
        UTI_NODISCARD constexpr const_reverse_iterator crbegin () const noexcept { return rbegin() ; }

        UTI_NODISCARD constexpr       reverse_iterator  rend ()       noexcept { return --begin() ; }
        UTI_NODISCARD constexpr const_reverse_iterator  rend () const noexcept { return --begin() ; }
        UTI_NODISCARD constexpr const_reverse_iterator crend () const noexcept { return    rend() ; }
private:
        value_type data_ [ capacity_ ] ;
        ssize_type size_ ;

        UTI_NODISCARD constexpr bool       _can_fit ( ssize_type const _count_ ) const noexcept ;
        UTI_NODISCARD constexpr ssize_type _can_fit (                          ) const noexcept ;

        template< typename... Args >
        constexpr void _emplace ( Args&&... _args_ ) noexcept ;
} ;


template< ssize_t Capacity, meta::trivial CharType >
UTI_NODISCARD constexpr bool
small_string< Capacity, CharType >::_can_fit ( ssize_type const _count_ ) const noexcept
{
        return ( size() + _count_ + 1 ) <= capacity() ;
}

template< ssize_t Capacity, meta::trivial CharType >
UTI_NODISCARD constexpr
small_string< Capacity, CharType >::ssize_type
small_string< Capacity, CharType >::_can_fit () const noexcept
{
        return capacity() - size() - 1 ;
}

template< ssize_t Capacity, meta::trivial CharType >
template< typename... Args >
constexpr void
small_string< Capacity, CharType >::_emplace ( Args&&... _args_ ) noexcept
{
        at( size_++ ) = value_type( _args_... ) ;
        at( size_   ) = value_type( 0 ) ;
}


template< ssize_t Capacity, meta::trivial CharType >
constexpr
small_string< Capacity, CharType >::small_string ( const_pointer _cstr_ ) noexcept
        : data_{}, size_{ 0 }
{
        ssize_type strlen = ::uti::strlen( _cstr_ ) ;
        ssize_type len = strlen + 1 > capacity() ? capacity() - 1 : strlen ;

        for( ssize_type i = 0; i < len; ++i )
        {
                at( i ) = _cstr_[ i ] ;
        }
        size_ = len ;
        at( size_ ) = value_type( 0 ) ;
}

template< ssize_t Capacity, meta::trivial CharType >
template< meta::forward_iterator Iter >
        requires meta::convertible_to< iter_value_t< Iter >, CharType >
constexpr
small_string< Capacity, CharType >::small_string ( Iter _begin_, Iter const _end_ ) noexcept
        : data_{}, size_{ 0 }
{
        ssize_type strlen = ::uti::distance( _begin_, _end_ ) ;
        ssize_type len = strlen + 1 > capacity() ? capacity() - 1 : strlen ;

        for( ssize_type i = 0; i < len; ++i )
        {
                at( i ) = *_begin_++ ;
        }
        size_ = len ;
        at( size_ ) = value_type( 0 ) ;
}

template< ssize_t Capacity, meta::trivial CharType >
template< meta::simple_container Other >
        requires meta::convertible_to< typename Other::value_type, CharType >
constexpr
small_string< Capacity, CharType >::small_string ( Other const & _other_ ) noexcept
        : data_{}, size_{ 0 }
{
        ssize_type strlen = ::uti::distance( _other_.begin(), _other_.end() ) ;
        ssize_type len = strlen + 1 > capacity() ? capacity() - 1 : strlen ;

        auto iter = _other_.begin() ;

        for( ssize_type i = 0; i < len; ++i )
        {
                at( i ) = *iter++ ;
        }
        size_ = len ;
        at( size_ ) = value_type( 0 ) ;
}

template< ssize_t Capacity, meta::trivial CharType >
constexpr void
small_string< Capacity, CharType >::push_back ( value_type const & _char_ ) noexcept
{
        if( !_can_fit( 1 ) ) return ;

        _emplace( _char_ ) ;
}

template< ssize_t Capacity, meta::trivial CharType >
template< typename... Args >
constexpr void
small_string< Capacity, CharType >::emplace_back ( Args&&... _args_ ) noexcept
{
        if( !_can_fit( 1 ) ) return ;

        _emplace( UTI_FWD( _args_ )... ) ;
}

template< ssize_t Capacity, meta::trivial CharType >
constexpr void
small_string< Capacity, CharType >::insert ( value_type const & _char_, ssize_type _position_ ) noexcept
{
        if( !_can_fit( 1 ) ) return ;

        if( _position_ >= size() ) return _emplace( _char_ ) ;

        for( ssize_type i = size(); i > _position_; --i )
        {
                at( i ) = at( i - 1 ) ;
        }
        at( _position_ ) = _char_ ;
        at( ++size_ ) = value_type( 0 ) ;
}

template< ssize_t Capacity, meta::trivial CharType >
template< meta::forward_iterator Iter >
        requires meta::convertible_to< iter_value_t< Iter >, CharType >
constexpr void
small_string< Capacity, CharType >::append ( Iter _begin_, Iter const _end_ ) noexcept
{
        ssize_type strlen = ::uti::distance( _begin_, _end_ ) ;
        ssize_type len = strlen > _can_fit() ? _can_fit() : strlen ;

        for( ssize_type i = 0; i < len; ++i )
        {
                at( size_++ ) = *_begin_++ ;
        }
        at( size_ ) = value_type( 0 ) ;
}

template< ssize_t Capacity, meta::trivial CharType >
template< meta::simple_container Other >
        requires meta::convertible_to< typename Other::value_type, CharType >
constexpr void
small_string< Capacity, CharType >::append ( Other const & _other_ ) noexcept
{
        append( _other_.begin(), _other_.end() ) ;
}


} // namespace uti
