//
//
//      uti
//      variant_vector.hpp
//

#pragma once

#include <container/vector.hpp>
#include <meta/list.hpp>
#include <type/sequence.hpp>


namespace uti
{


template< typename... Callable >
struct visitor : Callable...
{
        using Callable::operator()... ;
} ;


namespace _detail
{


template< typename T, typename... Ts > constexpr size_t max_align_of () noexcept ;
template< typename T, typename... Ts > constexpr size_t min_align_of () noexcept ;
template< typename T, typename... Ts > constexpr size_t max_size_of  () noexcept ;
template< typename T, typename... Ts > constexpr size_t min_size_of  () noexcept ;

template< typename T, typename T1, typename... Ts >
constexpr ssize_t index_of ( ssize_t _start_ = 0 ) noexcept ;


template< typename T >
using iterator_type_for = iterator_base< T, random_access_iterator_tag > ;


} // namespace _detail


template< typename Resource, typename... Ts >
class variant_vector
{
        using value_types = meta::type_list< Ts... > ;

        static constexpr ssize_t max_elem_align { _detail::max_align_of< Ts... >() } ;
        static constexpr ssize_t min_elem_align { _detail::min_align_of< Ts... >() } ;

        static constexpr ssize_t max_elem_size { _detail::max_size_of< Ts... >() } ;
        static constexpr ssize_t min_elem_size { _detail::min_size_of< Ts... >() } ;
public:
        using           resource_type = Resource ;
        using        _resource_traits = uti::resource_traits< resource_type > ;
        using              block_type = typename _resource_traits::block_type ;
        using internal_allocator_type = uti::allocator< ssize_t, resource_type > ;

        using       size_type =  size_t    ;
        using      ssize_type = ssize_t    ;
        using difference_type = ssize_type ;

        using       byte_pointer = typename block_type::      pointer ;
        using const_byte_pointer = typename block_type::const_pointer ;

        using               byte_iterator = typename block_type::      iterator ;
        using         const_byte_iterator = typename block_type::const_iterator ;
        using       reverse_byte_iterator = ::uti::reverse_iterator<       byte_iterator > ;
        using const_reverse_byte_iterator = ::uti::reverse_iterator< const_byte_iterator > ;

        constexpr variant_vector () noexcept = default ;

        template< typename T >
                requires meta::one_of< T, Ts... >
        constexpr variant_vector ( ssize_type _capacity_ ) UTI_NOEXCEPT_UNLESS_BADALLOC ;

        template< meta::forward_iterator Iter >
                requires meta::one_of< iter_value_t< Iter >, Ts... >
        constexpr variant_vector ( Iter _begin_, Iter const _end_ ) UTI_NOEXCEPT_UNLESS_BADALLOC ;

        template< meta::simple_container Other >
                requires meta::one_of< typename Other::value_type, Ts... >
        constexpr variant_vector ( Other const & _other_ ) UTI_NOEXCEPT_UNLESS_BADALLOC ;

        constexpr variant_vector             ( variant_vector const &  ) UTI_NOEXCEPT_UNLESS_BADALLOC ;
        constexpr variant_vector             ( variant_vector       && )     noexcept                 ;
        constexpr variant_vector & operator= ( variant_vector const &  ) UTI_NOEXCEPT_UNLESS_BADALLOC ;
        constexpr variant_vector & operator= ( variant_vector       && )     noexcept                 ;

        constexpr ~variant_vector () noexcept { reset() ; }

        template< typename T >
                requires meta::one_of< T, Ts... >
        constexpr void push_back ( T const & _val_ ) UTI_NOEXCEPT_UNLESS_BADALLOC ;

        template< typename T >
                requires meta::one_of< T, Ts... >
        constexpr void push_back ( T && _val_ ) UTI_NOEXCEPT_UNLESS_BADALLOC ;

        template< typename T, typename... Args >
                requires( meta::one_of< T, Ts... > && meta::constructible_from< T, Args... > )
        constexpr void emplace_back ( Args&&... _args_ ) UTI_NOEXCEPT_UNLESS_BADALLOC ;

        template< meta::forward_iterator Iter >
                requires meta::one_of< iter_value_t< Iter >, Ts... >
        constexpr void append ( Iter _begin_, Iter const _end_ ) UTI_NOEXCEPT_UNLESS_BADALLOC ;

        template< meta::simple_container Other >
                requires meta::one_of< typename Other::value_type, Ts... >
        constexpr void append ( Other const & _other_ ) UTI_NOEXCEPT_UNLESS_BADALLOC ;

        template< typename T >
                requires meta::one_of< T, Ts... >
        constexpr void insert ( ssize_type _position_, T const & _val_ ) UTI_NOEXCEPT_UNLESS_BADALLOC ;

        template< typename T >
                requires meta::one_of< T, Ts... >
        constexpr void insert ( ssize_type _position_, T && _val_ ) UTI_NOEXCEPT_UNLESS_BADALLOC ;

        template< typename T, typename... Args >
                requires meta::one_of< T, Ts... > && meta::constructible_from< T, Args... >
        constexpr void insert ( ssize_type _position_, Args&&... _args_ ) UTI_NOEXCEPT_UNLESS_BADALLOC ;

        constexpr void erase ( ssize_type _position_ ) noexcept ;

        constexpr void reserve_bytes ( ssize_type _bytes_ ) UTI_NOEXCEPT_UNLESS_BADALLOC ;

        template< typename T >
                requires meta::one_of< T, Ts... >
        constexpr void reserve ( ssize_type _count_ ) UTI_NOEXCEPT_UNLESS_BADALLOC ;

        template< typename T >
                requires meta::one_of< T, Ts... >
        constexpr void reserve_additional ( ssize_type _count_ ) UTI_NOEXCEPT_UNLESS_BADALLOC ;

        constexpr void clear () noexcept ;
        constexpr void reset () noexcept ;

        UTI_NODISCARD constexpr ssize_type capacity_bytes () const noexcept { return storage_.size_ ; }
        UTI_NODISCARD constexpr ssize_type     size_bytes () const noexcept { return _find_end( size_ - 1 ) - storage_.begin() ; }

        UTI_NODISCARD constexpr ssize_type  size () const noexcept { return size_      ; }
        UTI_NODISCARD constexpr       bool empty () const noexcept { return size_ == 0 ; }
        UTI_NODISCARD constexpr       bool  null () const noexcept { return !storage_  ; }

        template< typename T > UTI_NODISCARD constexpr T       & get ( ssize_type _idx_ )       noexcept ;
        template< typename T > UTI_NODISCARD constexpr T const & get ( ssize_type _idx_ ) const noexcept ;

        template< typename T > UTI_NODISCARD constexpr _detail::iterator_type_for< T       > get_ptr ( ssize_type _idx_ )       noexcept ;
        template< typename T > UTI_NODISCARD constexpr _detail::iterator_type_for< T const > get_ptr ( ssize_type _idx_ ) const noexcept ;

        template< typename Visitor, typename Self >
        constexpr void visit ( this Self && self, ssize_type _idx_, Visitor&& _visitor_ ) noexcept
        {
                [ & ]< ssize_type... Idxs >( uti::index_sequence< Idxs... > )
                {
                        ( ...,
                        [ & ]
                        {
                                using       raw_type =             meta::list::at_t< Idxs, value_types >   ;
                                using effective_type = like< Self, meta::list::at_t< Idxs, value_types > > ;

                                if constexpr( meta::invocable< Visitor, effective_type, ssize_type > )
                                {
                                        if( UTI_FWD( self ).types_[ _idx_ ] == Idxs )
                                        {
                                                ::uti::invoke( _visitor_, UTI_FWD( self ).template get< raw_type >( _idx_ ), _idx_ ) ;
                                        }
                                }
                                else if constexpr( meta::invocable< Visitor, effective_type > )
                                {
                                        if( UTI_FWD( self ).types_[ _idx_ ] == Idxs )
                                        {
                                                ::uti::invoke( _visitor_, UTI_FWD( self ).template get< raw_type >( _idx_ ) ) ;
                                        }
                                }
                        }() ) ;
                }( uti::make_index_sequence< sizeof...( Ts ) >{} ) ;
        }

        template< typename Visitor, typename Self >
        constexpr void for_each ( this Self && self, Visitor&& _visitor_ ) noexcept
        {
                for( ssize_type i = 0; i < UTI_FWD( self ).size(); ++i )
                {
                        UTI_FWD( self ).visit( i, _visitor_ ) ;
                }
        }

        template< typename T >
                requires meta::one_of< T, Ts... >
        constexpr void replace ( ssize_type _idx_, T const & _val_ ) UTI_NOEXCEPT_UNLESS_BADALLOC ;

        template< typename T >
                requires meta::one_of< T, Ts... >
        constexpr void replace ( ssize_type _idx_, T && _val_ ) UTI_NOEXCEPT_UNLESS_BADALLOC ;
private:
        /// TODO: do we need the vectors? can't we just store the metadata at the start of our main storage?
        /// we can make our life easier with a thin "mutable view" type for book keeping and a nice API
        ///
        /// block_type storage_ ;
        /// ssize_type size_ ;
        /// iterator begin_ ;
        /// mut_view< u8_t > types_ ;
        /// mut_view< u8_t > offsets_ ; /// replace with packed_vector
        ///
        ssize_type                                            size_ {          0 } ;
        block_type                                         storage_ { nullptr, 0 } ;
        uti::vector< ssize_type, internal_allocator_type > offsets_ {            } ;
        uti::vector< ssize_type, internal_allocator_type >   types_ {            } ;

        constexpr void _reserve ( ssize_type _bytes_ ) UTI_NOEXCEPT_UNLESS_BADALLOC ;

        template< typename T >
                requires meta::one_of< T, Ts... >
        constexpr bool _can_fit ( ssize_type _count_ ) const noexcept ;

        constexpr       byte_iterator _find_end ( ssize_type _idx_ )       noexcept ;
        constexpr const_byte_iterator _find_end ( ssize_type _idx_ ) const noexcept ;

        template< typename T > constexpr       byte_iterator _align_for (       byte_iterator _ptr_ )       noexcept ;
        template< typename T > constexpr const_byte_iterator _align_for ( const_byte_iterator _ptr_ ) const noexcept ;

        template< typename T > constexpr ssize_type _padding_for ( const_byte_iterator _ptr_ ) const noexcept ;
} ;


template< typename Resource, typename... Ts >
constexpr void
variant_vector< Resource, Ts... >::_reserve ( ssize_type _bytes_ ) UTI_NOEXCEPT_UNLESS_BADALLOC
{
        if( !storage_ )
        {
                storage_ = _resource_traits::allocate( _bytes_, max_elem_align ) ;
                return ;
        }
        if( storage_.size_ >= _bytes_ ) return ;

        if constexpr( conjunction_v< is_trivially_relocatable< Ts >... > )
        {
                _resource_traits::reallocate( storage_, _bytes_, max_elem_align ) ;
        }
        else
        {
                if( _resource_traits::realloc_inplace( storage_, _bytes_ ) ) return ;

                block_type new_block = _resource_traits::allocate( _bytes_, max_elem_align ) ;

                if( !new_block ) return ;

                ::uti::copy( storage_.begin(), storage_.begin() + size_bytes(), new_block.begin() ) ;

                /// TODO: try memcpying the whole thing first, and then only move-constructing the non-trivial elements
                for_each( [ & ]( auto & elem, ssize_type idx )
                        {
                                using type = remove_reference_t< decltype( elem ) > ;
                                using iter = _detail::iterator_type_for< remove_const_t< type > > ;

                                if constexpr( !is_trivially_relocatable_v< type > )
                                {
                                        ::uti::construct< iter >( new_block.begin() + offsets_[ idx ], UTI_MOVE( elem ) ) ;
                                        ::uti::destroy( &elem ) ;
                                }
                                else
                                {
                                        ::uti::copy( storage_.begin() + offsets_[ idx ], _find_end( idx ), new_block.begin() + offsets_[ idx ] ) ;
                                }
                        }
                ) ;
                _resource_traits::deallocate( storage_ ) ;
                storage_ = new_block ;
        }
}

template< typename Resource, typename... Ts >
template< typename T >
        requires meta::one_of< T, Ts... >
constexpr bool
variant_vector< Resource, Ts... >::_can_fit ( ssize_type _count_ ) const noexcept
{
        const_byte_iterator aligned = _align_for< T >( _find_end( size_ - 1 ) ) ;

        ssize_type free_space = storage_.end() - aligned ;

        return static_cast< ssize_type >( _count_ * sizeof( T ) ) <= free_space ;
}

template< typename Resource, typename... Ts >
constexpr
variant_vector< Resource, Ts... >::byte_iterator
variant_vector< Resource, Ts... >::_find_end ( ssize_type _idx_ ) noexcept
{
        byte_iterator elem_start = storage_.begin() + offsets_[ _idx_ ] ;

        ssize_type elem_size { 0 } ;

        visit( _idx_, [ & ]( auto const & elem ){ elem_size = sizeof( remove_cvref_t< decltype( elem ) > ) ; } ) ;

        return elem_start + elem_size ;
}

template< typename Resource, typename... Ts >
constexpr
variant_vector< Resource, Ts... >::const_byte_iterator
variant_vector< Resource, Ts... >::_find_end ( ssize_type _idx_ ) const noexcept
{
        const_byte_iterator elem_start = storage_.begin() + offsets_[ _idx_ ] ;

        ssize_type elem_size { 0 } ;

        visit( _idx_, [ & ]( auto const & elem ){ elem_size = sizeof( remove_cvref_t< decltype( elem ) > ) ; } ) ;

        return elem_start + elem_size ;
}

template< typename Resource, typename... Ts >
template< typename T >
constexpr
variant_vector< Resource, Ts... >::byte_iterator
variant_vector< Resource, Ts... >::_align_for ( byte_iterator _ptr_ ) noexcept
{
        ssize_type mask = alignof( T ) - 1 ;

        while( ( _ptr_ & mask ) != 0 )
        {
                ++_ptr_ ;
        }
        return _ptr_ ;
}

template< typename Resource, typename... Ts >
template< typename T >
constexpr
variant_vector< Resource, Ts... >::const_byte_iterator
variant_vector< Resource, Ts... >::_align_for ( const_byte_iterator _ptr_ ) const noexcept
{
        ssize_type mask = alignof( T ) - 1 ;

        while( ( _ptr_ & mask ) != 0 )
        {
                ++_ptr_ ;
        }
        return _ptr_ ;
}

template< typename Resource, typename... Ts >
template< typename T >
constexpr
variant_vector< Resource, Ts... >::ssize_type
variant_vector< Resource, Ts... >::_padding_for ( const_byte_iterator _ptr_ ) const noexcept
{
        const_byte_iterator aligned = _align_for< T >( _ptr_ ) ;

        return aligned - _ptr_ ;
}

template< typename Resource, typename... Ts >
template< typename T >
        requires meta::one_of< T, Ts... >
constexpr
variant_vector< Resource, Ts... >::variant_vector ( ssize_type _capacity_ ) UTI_NOEXCEPT_UNLESS_BADALLOC
{
        /// small memory layout optimization
        /// since we use the same memory resource for the metadata and the main storage,
        /// we can force the metadata vectors to reallocate when we want
        /// and allocate main storage right after the metadata.
        ///
        /// by overallocating the metadata vectors,
        /// we ensure that they won't have to reallocate for the first few push backs
        /// which cause a reallocation of the main storage.
        /// this increases the chances of the main storage being able to reallocate in place
        /// depending on the state of our memory resource
        ///
        /// side note - u8_ts should be enough for the types metadata for all reasonable use-cases
        /// and the offsets can be stored in a custom data structure
        /// which dynamically picks the smalles type large enough to fit the current offset values.
        /// with this taken into account, the memory overhead of the metadata will decrease
        /// making the below optimization less costly
        if( sizeof( T ) < max_elem_size )
        {
                offsets_.reserve( _capacity_ ) ;
                types_  .reserve( _capacity_ ) ;
        }
        else
        {
                offsets_.reserve( _capacity_ * 2 ) ;
                types_  .reserve( _capacity_ * 2 ) ;
        }
        _reserve( _capacity_ * sizeof( T ) ) ;
}

template< typename Resource, typename... Ts >
template< meta::forward_iterator Iter >
        requires meta::one_of< iter_value_t< Iter >, Ts... >
constexpr
variant_vector< Resource, Ts... >::variant_vector ( Iter _begin_, Iter const _end_ ) UTI_NOEXCEPT_UNLESS_BADALLOC
{
        ssize_type capacity = ::uti::distance( _begin_, _end_ ) ;

        offsets_.reserve( capacity * 2 ) ;
        types_  .reserve( capacity * 2 ) ;

        reserve< iter_value_t< Iter > >( capacity ) ;

        for( ; _begin_ != _end_; ++_begin_ )
        {
                emplace_back( *_begin_ ) ;
        }
}

template< typename Resource, typename... Ts >
template< meta::simple_container Other >
        requires meta::one_of< typename Other::value_type, Ts... >
constexpr
variant_vector< Resource, Ts... >::variant_vector ( Other const & _other_ ) UTI_NOEXCEPT_UNLESS_BADALLOC
        : variant_vector( _other_.begin(), _other_.end() )
{}

template< typename Resource, typename... Ts >
constexpr
variant_vector< Resource, Ts... >::variant_vector ( variant_vector const & _other_ ) UTI_NOEXCEPT_UNLESS_BADALLOC
{
        reserve_bytes( _other_.size_bytes() ) ;

        if( !storage_ ) return ;

        if constexpr( conjunction_v< is_trivially_relocatable< Ts >... > )
        {
                ::uti::copy( _other_.storage_.begin(), _other_.storage_.begin() + _other_.size_bytes(), storage_.begin() ) ;
        }
        else
        {
                _other_.for_each(
                        [ & ]( auto const & val, ssize_type idx )
                        {
                                using type = remove_reference_t< decltype( val ) > ;
                                using iter = _detail::iterator_type_for< type > ;

                                if constexpr( !is_trivially_relocatable_v< type > )
                                {
                                        ::uti::construct< iter >( storage_.begin() + _other_.offsets_[ idx ], val ) ;
                                }
                                else
                                {
                                        ::uti::copy( &val, &val + 1, storage_.begin() + _other_.offsets_[ idx ] ) ;
                                }
                        }
                ) ;
        }
        size_    = _other_.   size_ ;
        offsets_ = _other_.offsets_ ;
        types_   = _other_.  types_ ;
}

template< typename Resource, typename... Ts >
constexpr
variant_vector< Resource, Ts... >::variant_vector ( variant_vector && _other_ ) noexcept
        : size_   (           _other_.   size_   )
        , storage_(           _other_.storage_   )
        , offsets_( UTI_MOVE( _other_.offsets_ ) )
        , types_  ( UTI_MOVE( _other_.  types_ ) )
{
        _other_.   size_ =            0   ;
        _other_.storage_ = { nullptr, 0 } ;
}

template< typename Resource, typename... Ts >
constexpr
variant_vector< Resource, Ts... > &
variant_vector< Resource, Ts... >::operator= ( variant_vector const & _other_ ) UTI_NOEXCEPT_UNLESS_BADALLOC
{
        clear() ;
        reserve_bytes( _other_.size_bytes() ) ;

        /// TODO: direct copy, avoid push_back

        _other_.for_each( [ & ]( auto const & val ){ push_back( val ) ; } ) ;

        return *this ;
}

template< typename Resource, typename... Ts >
constexpr
variant_vector< Resource, Ts... > &
variant_vector< Resource, Ts... >::operator= ( variant_vector && _other_ ) noexcept
{
        reset() ;

        size_    =           _other_.   size_   ;
        storage_ =           _other_.storage_   ;
        offsets_ = UTI_MOVE( _other_.offsets_ ) ;
        types_   = UTI_MOVE( _other_.  types_ ) ;

        _other_.   size_ =            0   ;
        _other_.storage_ = { nullptr, 0 } ;

        return *this ;
}

template< typename Resource, typename... Ts >
template< typename T >
        requires meta::one_of< T, Ts... >
constexpr void
variant_vector< Resource, Ts... >::push_back ( T const & _val_ ) UTI_NOEXCEPT_UNLESS_BADALLOC
{
        emplace_back< T >( _val_ ) ;
}

template< typename Resource, typename... Ts >
template< typename T >
        requires meta::one_of< T, Ts... >
constexpr void
variant_vector< Resource, Ts... >::push_back ( T && _val_ ) UTI_NOEXCEPT_UNLESS_BADALLOC
{
        emplace_back< T >( UTI_MOVE( _val_ ) ) ;
}

template< typename Resource, typename... Ts >
template< typename T, typename... Args >
        requires( meta::one_of< T, Ts... > && meta::constructible_from< T, Args... > )
constexpr void
variant_vector< Resource, Ts... >::emplace_back ( Args&&... _args_ ) UTI_NOEXCEPT_UNLESS_BADALLOC
{
        using iter_t = _detail::iterator_type_for< T > ;

        iter_t dest { nullptr } ;

        if( null() || empty() )
        {
                _reserve( sizeof( T ) ) ;
                dest = storage_.begin() ;

                if( !dest ) return ;

                offsets_.push_back( 0 ) ;
                types_.push_back( _detail::index_of< T, Ts... >() ) ;
        }
        else
        {
                if( !_can_fit< T >( 1 ) )
                {
                        reserve_bytes( capacity_bytes() * 2 ) ;
                        reserve_additional< T >( 1 ) ;
                }
                if( !_can_fit< T >( 1 ) ) return ;

                dest = _align_for< T >( _find_end( size_ - 1 ) ) ;

                offsets_.push_back( static_cast< const_byte_iterator >( dest ) - storage_.begin() ) ;
                types_.push_back( _detail::index_of< T, Ts... >() ) ;
        }
        if( dest )
        {
                ::uti::construct( dest, UTI_FWD( _args_ )... ) ;
                ++size_ ;
        }
}

template< typename Resource, typename... Ts >
template< meta::forward_iterator Iter >
        requires meta::one_of< iter_value_t< Iter >, Ts... >
constexpr void
variant_vector< Resource, Ts... >::append ( Iter _begin_, Iter const _end_ ) UTI_NOEXCEPT_UNLESS_BADALLOC
{
        reserve_additional< iter_value_t< Iter > >( ::uti::distance( _begin_, _end_ ) ) ;

        for( ; _begin_ != _end_; ++_begin_ )
        {
                emplace_back( *_begin_ ) ;
        }
}

template< typename Resource, typename... Ts >
template< meta::simple_container Other >
        requires meta::one_of< typename Other::value_type, Ts... >
constexpr void
variant_vector< Resource, Ts... >::append ( Other const & _other_ ) UTI_NOEXCEPT_UNLESS_BADALLOC
{
        append( _other_.begin(), _other_.end() ) ;
}

template< typename Resource, typename... Ts >
template< typename T >
        requires meta::one_of< T, Ts... >
constexpr void
variant_vector< Resource, Ts... >::insert ( ssize_type _position_, T const & _val_ ) UTI_NOEXCEPT_UNLESS_BADALLOC
{
        insert< T, T const & >( _position_, _val_ ) ;
}

template< typename Resource, typename... Ts >
template< typename T >
        requires meta::one_of< T, Ts... >
constexpr void
variant_vector< Resource, Ts... >::insert ( ssize_type _position_, T && _val_ ) UTI_NOEXCEPT_UNLESS_BADALLOC
{
        insert< T, T && >( _position_, UTI_MOVE( _val_ ) ) ;
}

template< typename Resource, typename... Ts >
template< typename T, typename... Args >
        requires meta::one_of< T, Ts... > && meta::constructible_from< T, Args... >
constexpr void
variant_vector< Resource, Ts...>::insert ( ssize_type _position_, Args&&... _args_ ) UTI_NOEXCEPT_UNLESS_BADALLOC
{
        using iter_t = _detail::iterator_type_for< T > ;

              byte_iterator aligned    = _align_for< T >( _find_end( _position_ - 1 ) ) ;
        const_byte_iterator next_begin = storage_.begin() + offsets_[ _position_ ] ;

        if( next_begin - aligned >= static_cast< ssize_type >( sizeof( T ) ) )
        {
                ::uti::construct< iter_t >( aligned, UTI_FWD( _args_ )... ) ;

                offsets_.insert( _position_, aligned - storage_.begin() ) ;
                types_  .insert( _position_, _detail::index_of< T, Ts... >() ) ;

                ++size_ ;
        }
        else
        {
                /// lord have mercy
        }
}

template< typename Resource, typename... Ts >
constexpr void
variant_vector< Resource, Ts... >::reserve_bytes ( ssize_type _bytes_ ) UTI_NOEXCEPT_UNLESS_BADALLOC
{
        offsets_.reserve( _bytes_ / min_elem_size ) ;
        types_  .reserve( _bytes_ / min_elem_size ) ;

        _reserve( _bytes_ ) ;
}

template< typename Resource, typename... Ts >
template< typename T >
        requires meta::one_of< T, Ts... >
constexpr void
variant_vector< Resource, Ts... >::reserve ( ssize_type _count_ ) UTI_NOEXCEPT_UNLESS_BADALLOC
{
        if( sizeof( T ) < max_elem_size )
        {
                offsets_.reserve( _count_ ) ;
                types_  .reserve( _count_ ) ;
        }
        else
        {
                offsets_.reserve( _count_ * 2 ) ;
                types_  .reserve( _count_ * 2 ) ;
        }
        _reserve( _count_ * sizeof( T ) ) ;
}

template< typename Resource, typename... Ts >
template< typename T >
        requires meta::one_of< T, Ts... >
constexpr void
variant_vector< Resource, Ts... >::reserve_additional ( ssize_type _count_ ) UTI_NOEXCEPT_UNLESS_BADALLOC
{
        ssize_type padding = _padding_for< T >( _find_end( size_ - 1 ) ) ;

        _reserve( size_bytes() + padding + sizeof( T ) * _count_ ) ;
}

template< typename Resource, typename... Ts >
constexpr void
variant_vector< Resource, Ts... >::clear () noexcept
{
        for_each(
                []( auto & val )
                {
                        using type = remove_reference_t< decltype( val ) > ;

                        if constexpr( !is_trivially_destructible_v< type > )
                        {
                                ::uti::destroy( &val ) ;
                        }
                }
        ) ;
        size_ = 0 ;
        offsets_.clear() ;
        types_  .clear() ;
}

template< typename Resource, typename... Ts >
constexpr void
variant_vector< Resource, Ts... >::reset () noexcept
{
        clear() ;
        _resource_traits::deallocate( storage_ ) ;
}

template< typename Resource, typename... Ts >
template< typename T >
UTI_NODISCARD constexpr T &
variant_vector< Resource, Ts... >::get ( ssize_type _idx_ ) noexcept
{
        return *get_ptr< T >( _idx_ ) ;
}

template< typename Resource, typename... Ts >
template< typename T >
UTI_NODISCARD constexpr T const &
variant_vector< Resource, Ts... >::get ( ssize_type _idx_ ) const noexcept
{
        return *get_ptr< T >( _idx_ ) ;
}

template< typename Resource, typename... Ts >
template< typename T >
UTI_NODISCARD constexpr _detail::iterator_type_for< T >
variant_vector< Resource, Ts... >::get_ptr ( ssize_type _idx_ ) noexcept
{
        if( _detail::index_of< T, Ts... >() == types_[ _idx_ ] )
        {
                return storage_.begin() + offsets_[ _idx_ ] ;
        }
        return nullptr ;
}

template< typename Resource, typename... Ts >
template< typename T >
UTI_NODISCARD constexpr _detail::iterator_type_for< T const >
variant_vector< Resource, Ts... >::get_ptr ( ssize_type _idx_ ) const noexcept
{
        if( _detail::index_of< T, Ts... >() == types_[ _idx_ ] )
        {
                return storage_.begin() + offsets_[ _idx_ ] ;
        }
        return nullptr ;
}


namespace _detail
{


template< typename T, typename... Ts >
constexpr size_t max_align_of () noexcept
{
        if constexpr( sizeof...( Ts ) == 0 )
        {
                return alignof( T ) ;
        }
        else
        {
                size_t align_rest = max_align_of< Ts... >() ;

                return alignof( T ) > align_rest ? alignof( T ) : align_rest ;
        }
} ;

template< typename T, typename... Ts >
constexpr size_t min_align_of () noexcept
{
        if constexpr( sizeof...( Ts ) == 0 )
        {
                return alignof( T ) ;
        }
        else
        {
                size_t align_rest = min_align_of< Ts... >() ;

                return alignof( T ) < align_rest ? alignof( T ) : align_rest ;
        }
} ;

template< typename T, typename... Ts >
constexpr size_t max_size_of () noexcept
{
        if constexpr( sizeof...( Ts ) == 0 )
        {
                return sizeof( T ) ;
        }
        else
        {
                size_t size_rest = max_size_of< Ts... >() ;

                return sizeof( T ) > size_rest ? sizeof( T ) : size_rest ;
        }
} ;

template< typename T, typename... Ts >
constexpr size_t min_size_of () noexcept
{
        if constexpr( sizeof...( Ts ) == 0 )
        {
                return sizeof( T ) ;
        }
        else
        {
                size_t size_rest = min_size_of< Ts... >() ;

                return sizeof( T ) < size_rest ? sizeof( T ) : size_rest ;
        }
} ;

template< typename T, typename T1, typename... Ts >
constexpr ssize_t index_of ( ssize_t _start_ ) noexcept
{
        if constexpr( meta::same_as< T, T1 > )
        {
                return _start_ ;
        }
        else if constexpr( sizeof...( Ts ) == 0 )
        {
                return -1 ;
        }
        else
        {
                return index_of< T, Ts... >( _start_ + 1 ) ;
        }
}


} // namespace _detail


} // namespace uti
