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
class packed_vector ;


template< typename Vector >
class packed_vector_iterator
{
public:
        using vector_type = Vector                           ;
        using vector_ref  = vector_type const &              ;
        using  ssize_type = typename vector_type::ssize_type ;

        constexpr packed_vector_iterator ( vector_type const & _vector_                              ) : vector_{ _vector_ }, position_{         0  } {}
        constexpr packed_vector_iterator ( vector_type const & _vector_, ssize_type const _position_ ) : vector_{ _vector_ }, position_{ _position_ } {}


private:
        vector_ref   vector_ ;
        ssize_type position_ ;
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
        using _resource_traits = uti::resource_traits< resource_type > ;

        using       iterator = packed_vector_iterator< packed_vector > ;
        using const_iterator = packed_vector_iterator< packed_vector > ;

        using  t8_iterator = iterator_base<  t8, random_access_iterator_tag > ;
        using t16_iterator = iterator_base< t16, random_access_iterator_tag > ;
        using t32_iterator = iterator_base< t32, random_access_iterator_tag > ;
        using t64_iterator = iterator_base< t64, random_access_iterator_tag > ;

        using  const_t8_iterator = iterator_base<  t8 const, random_access_iterator_tag > ;
        using const_t16_iterator = iterator_base< t16 const, random_access_iterator_tag > ;
        using const_t32_iterator = iterator_base< t32 const, random_access_iterator_tag > ;
        using const_t64_iterator = iterator_base< t64 const, random_access_iterator_tag > ;

        constexpr packed_vector () noexcept : iter_8_{ nullptr }, level_{ packing_level::TINY }, capacity_{ 0 }, size_{ 0 } {}

        constexpr void push_back ( t64 const _val_ ) ;

        constexpr void reserve (                             ) ;
        constexpr void reserve ( ssize_type const _capacity_ ) ;

        UTI_NODISCARD constexpr ssize_type capacity () const noexcept { return          capacity_ ; }
        UTI_NODISCARD constexpr ssize_type     size () const noexcept { return              size_ ; }
        UTI_NODISCARD constexpr       bool    empty () const noexcept { return size_ ==         0 ; }
        UTI_NODISCARD constexpr       bool     full () const noexcept { return size_ == capacity_ ; }

        template< typename Self >
        UTI_NODISCARD constexpr t64 operator[] ( this Self && self, ssize_type const _idx_ ) noexcept
        {
                switch( UTI_FWD( self ).level_ )
                {
                        case packing_level:: TINY : return t64( *( UTI_FWD( self ).iter_8_  + _idx_ ) ) ;
                        case packing_level::SMALL : return t64( *( UTI_FWD( self ).iter_16_ + _idx_ ) ) ;
                        case packing_level::  MID : return t64( *( UTI_FWD( self ).iter_32_ + _idx_ ) ) ;
                        case packing_level::LARGE : return t64( *( UTI_FWD( self ).iter_64_ + _idx_ ) ) ;
                        default:
                                UTI_UNREACHABLE ;
                }
        }

        template< typename Self >
        UTI_NODISCARD constexpr t64 at ( this Self && self, ssize_type const _idx_ ) noexcept
        {
                return UTI_FWD( self ).operator[]( _idx_ ) ;
        }

        template< typename Self >
        UTI_NODISCARD constexpr t64 front ( this Self && self ) noexcept
        {
                return UTI_FWD( self ).at( 0 ) ;
        }

        template< typename Self >
        UTI_NODISCARD constexpr t64 back ( this Self && self ) noexcept
        {
                return UTI_FWD( self ).at( UTI_FWD( self ).size() - 1 ) ;
        }

        UTI_NODISCARD constexpr       iterator  begin ()       noexcept { return       iterator{ *this, 0 } ; }
        UTI_NODISCARD constexpr const_iterator  begin () const noexcept { return const_iterator{ *this, 0 } ; }
        UTI_NODISCARD constexpr const_iterator cbegin () const noexcept { return const_iterator{ *this, 0 } ; }

        UTI_NODISCARD constexpr       iterator  end ()       noexcept { return       iterator{ *this, size() } ; }
        UTI_NODISCARD constexpr const_iterator  end () const noexcept { return const_iterator{ *this, size() } ; }
        UTI_NODISCARD constexpr const_iterator cend () const noexcept { return const_iterator{ *this, size() } ; }
private:
        union
        {
                t8_iterator  iter_8_  ;
                t16_iterator iter_16_ ;
                t32_iterator iter_32_ ;
                t64_iterator iter_64_ ;
        } ;
        packing_level level_ ;
        ssize_type capacity_ ;
        ssize_type     size_ ;

        constexpr void _reserve_8  ( ssize_type const _capacity_ ) ;
        constexpr void _reserve_16 ( ssize_type const _capacity_ ) ;
        constexpr void _reserve_32 ( ssize_type const _capacity_ ) ;
        constexpr void _reserve_64 ( ssize_type const _capacity_ ) ;

        constexpr void _push_back  ( t64 const _val_ ) ;
        constexpr void _repack_for ( t64 const _val_ ) ;

        constexpr void _repack ( packing_level const _level_ ) ;

        template< meta::bidirectional_iterator  SrcIter ,
                  meta::bidirectional_iterator DestIter >
        constexpr void _repack ( SrcIter _begin_, SrcIter const _end_, DestIter _dest_ ) noexcept ;

        constexpr t64 _current_max () const noexcept ;
        constexpr t64 _current_min () const noexcept ;
} ;


template< bool Signed, typename Resource >
constexpr void
packed_vector< Signed, Resource >::push_back ( t64 const _val_ )
{
        if( _val_ > _current_max() || _val_ < _current_min() )
        {
                _repack_for( _val_ ) ;
        }
        if( full() ) reserve() ;

        _push_back( _val_ ) ;
}

template< bool Signed, typename Resource >
constexpr void
packed_vector< Signed, Resource >::reserve ()
{
        capacity_ == 0 ? reserve(             1 )
                       : reserve( capacity_ * 2 ) ;
}

template< bool Signed, typename Resource >
constexpr void
packed_vector< Signed, Resource >::reserve ( ssize_type const _capacity_ )
{
        if( _capacity_ <= capacity_ ) return ;

        switch( level_ )
        {
                case packing_level:: TINY : return _reserve_8 ( _capacity_ ) ;
                case packing_level::SMALL : return _reserve_16( _capacity_ ) ;
                case packing_level::  MID : return _reserve_32( _capacity_ ) ;
                case packing_level::LARGE : return _reserve_64( _capacity_ ) ;
                default :
                        UTI_UNREACHABLE ;
        }
}

template< bool Signed, typename Resource >
constexpr void
packed_vector< Signed, Resource >::_reserve_8 ( ssize_type const _capacity_ )
{
        block_t< t8 > block{ iter_8_, capacity_ } ;

        _resource_traits::reallocate( block, _capacity_, alignof( t64 ) ) ;

        iter_8_   = block.begin() ;
        capacity_ = block. size() ;
}

template< bool Signed, typename Resource >
constexpr void
packed_vector< Signed, Resource >::_reserve_16 ( ssize_type const _capacity_ )
{
        block_t< t16 > block{ iter_16_, capacity_ } ;

        _resource_traits::reallocate( block, _capacity_, alignof( t64 ) ) ;

        iter_16_  = block.begin() ;
        capacity_ = block. size() ;
}

template< bool Signed, typename Resource >
constexpr void
packed_vector< Signed, Resource >::_reserve_32 ( ssize_type const _capacity_ )
{
        block_t< t32 > block{ iter_32_, capacity_ } ;

        _resource_traits::reallocate( block, _capacity_, alignof( t64 ) ) ;

        iter_32_  = block.begin() ;
        capacity_ = block. size() ;
}

template< bool Signed, typename Resource >
constexpr void
packed_vector< Signed, Resource >::_reserve_64 ( ssize_type const _capacity_ )
{
        block_t< t64 > block{ iter_64_, capacity_ } ;

        _resource_traits::reallocate( block, _capacity_, alignof( t64 ) ) ;

        iter_64_  = block.begin() ;
        capacity_ = block. size() ;
}


template< bool Signed, typename Resource >
constexpr void
packed_vector< Signed, Resource >::_push_back ( t64 const _val_ )
{
        switch( level_ )
        {
                case packing_level:: TINY : *( iter_8_  + size_ ) = static_cast< t8  >( _val_ ) ;
                case packing_level::SMALL : *( iter_16_ + size_ ) = static_cast< t16 >( _val_ ) ;
                case packing_level::  MID : *( iter_32_ + size_ ) = static_cast< t32 >( _val_ ) ;
                case packing_level::LARGE : *( iter_64_ + size_ ) = static_cast< t64 >( _val_ ) ;
                default :
                        UTI_UNREACHABLE ;
        }
        ++size_ ;
}

template< bool Signed, typename Resource >
constexpr void
packed_vector< Signed, Resource >::_repack_for ( t64 const _val_ )
{
        if     (  t8_min <= _val_ && _val_ <=  t8_max ) _repack( packing_level:: TINY ) ;
        else if( t16_min <= _val_ && _val_ <= t16_max ) _repack( packing_level::SMALL ) ;
        else if( t32_min <= _val_ && _val_ <= t32_max ) _repack( packing_level::  MID ) ;
        else if( t64_min <= _val_ && _val_ <= t64_max ) _repack( packing_level::LARGE ) ;
        else
        {
                UTI_UNREACHABLE ;
        }
}

template< bool Signed, typename Resource >
constexpr void
packed_vector< Signed, Resource >::_repack ( packing_level const _level_ )
{
        if( level_ > _level_ ) return ;

        ssize_type expected_bytes = size_ * ( to_underlying( _level_ ) / 8 ) ;

        reserve( expected_bytes / ( to_underlying( level_ ) / 8 ) ) ;

        ssize_type scaling = to_underlying( _level_ ) / to_underlying( level_ ) ;

        if( level_ == packing_level::TINY )
        {
                if( _level_ == packing_level::SMALL )
                {
                        _repack( iter_8_ + size_, iter_8_, iter_16_( iter_8_ + size_ * scaling ) ) ;
                }
                else if( _level_ == packing_level::MID )
                {
                        _repack( iter_8_ + size_, iter_8_, iter_32_( iter_8_ + size_ * scaling ) ) ;
                }
                else if( _level_ == packing_level::LARGE )
                {
                        _repack( iter_8_ + size_, iter_8_, iter_64_( iter_8_ + size_ * scaling ) ) ;
                }

        }
        else if( level_ == packing_level::SMALL )
        {
                if( _level_ == packing_level::MID )
                {
                        _repack( iter_16_ + size_, iter_16_, iter_32_( iter_16_ + size_ * scaling ) ) ;
                }
                else if( _level_ == packing_level::LARGE )
                {
                        _repack( iter_16_ + size_, iter_16_, iter_64_( iter_16_ + size_ * scaling ) ) ;
                }
        }
        else if( level_ == packing_level::MID )
        {
                _repack( iter_32_ + size_, iter_32_, iter_64_( iter_32_ + size_ * scaling ) ) ;
        }
        else if( level_ == packing_level::LARGE )
        {
                return ;
        }
        else
        {
                UTI_UNREACHABLE ;
        }
}

template< bool Signed, typename Resource >
template< meta::bidirectional_iterator  SrcIter ,
          meta::bidirectional_iterator DestIter >
constexpr void
packed_vector< Signed, Resource >::_repack ( SrcIter _begin_, SrcIter const _end_, DestIter _dest_ ) noexcept
{
        for( ; _begin_ != _end_; --_begin_, --_dest_ )
        {
                *_dest_ = *_begin_ ;
        }
}

template< bool Signed, typename Resource >
constexpr
packed_vector< Signed, Resource >::t64
packed_vector< Signed, Resource >::_current_max () const noexcept
{
        switch( level_ )
        {
                case packing_level:: TINY : return t8_max  ;
                case packing_level::SMALL : return t16_max ;
                case packing_level::  MID : return t32_max ;
                case packing_level::LARGE : return t64_max ;
                default                   : return t64_max ;
        }
}

template< bool Signed, typename Resource >
constexpr
packed_vector< Signed, Resource >::t64
packed_vector< Signed, Resource >::_current_min () const noexcept
{
        if constexpr( !Signed )
        {
                return 0 ;
        }
        else
        {
                switch( level_ )
                {
                        case packing_level:: TINY : return t8_min  ;
                        case packing_level::SMALL : return t16_min ;
                        case packing_level::  MID : return t32_min ;
                        case packing_level::LARGE : return t64_min ;
                        default                   : return t64_min ;
                }
        }
}


} // namespace uti
