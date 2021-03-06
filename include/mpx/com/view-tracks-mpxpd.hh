#ifndef YOUKI_VIEW_TRACKS_HH
#define YOUKI_VIEW_TRACKS_HH

#include <gtkmm.h>
#include <cairomm/cairomm.h>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/optional.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <cmath>
#include <deque>
#include <boost/unordered_set.hpp>
#include <sigx/sigx.h>

#include "mpx/util-string.hh"
#include "mpx/util-graphics.hh"

#include "mpx/mpx-types.hh"
#include "mpx/mpx-main.hh"
#include "mpx/mpx-covers.hh"

#include "mpx/algorithm/aque.hh"
#include "mpx/algorithm/ntree.hh"
#include "mpx/algorithm/interval.hh"
#include "mpx/algorithm/limiter.hh"

#include "mpx/aux/glibaddons.hh"
#include "mpx/widgets/cairo-extensions.hh"
#include "mpx/i-youki-theme-engine.hh"

#include "mpx/mpxpd-sql.hh"

#include "glib-marshalers.h"
// ugh

typedef Glib::Property<Gtk::Adjustment*> PropAdj;

namespace MPX
{
namespace View
{
namespace Tracks
{

        using namespace boost;

        namespace
        {
            const double rounding = 4. ; 
        }

        typedef boost::tuple<std::string, std::string, std::string, gint64, Track_sp, gint64, gint64, std::string, std::string, gint64>  Row_t ;
/*
        bool operator<(const Row_t& a, const Row_t& b )
        {
            const Glib::ustring&    a1 = get<1>(a)
                                  , a2 = get<0>(a)
                                  , a3 = get<2>(a) ;

            const Glib::ustring&    b1 = get<1>(b)
                                  , b2 = get<0>(b)
                                  , b3 = get<2>(b) ;

            gint64 ta = get<5>(a) ;
            gint64 tb = get<5>(b) ;

            return (a1 < b1 && a2 < b2 && a3 < b3 && ta < tb ) ; 
        }
*/

        typedef std::vector<Row_t>                          Model_t ;
        typedef boost::shared_ptr<Model_t>                  Model_sp_t ;
        typedef sigc::signal<void, std::size_t, bool>       Signal1 ;

        struct Model_t_iterator_equal
        : std::binary_function<Model_t::iterator, Model_t::iterator, bool>
        {
            bool operator()(
                  const Model_t::iterator& a
                , const Model_t::iterator& b
            ) const
            {
                return a == b ;
            }

            bool operator()(
                  const Model_t::const_iterator& a
                , const Model_t::const_iterator& b
            ) const
            {
                return a == b ;
            }

            bool operator()(
                  const Model_t::const_iterator& a
                , const Model_t::iterator& b
            ) const
            {
                return a == b ;
            }

            bool operator()(
                  const Model_t::iterator& a
                , const Model_t::const_iterator& b
            ) const
            {
                return a == b ;
            }
        } ;

        struct Model_t_iterator_hash
        {
            std::size_t operator()( const Model_t::iterator& i ) const
            {
                return GPOINTER_TO_INT(&(*i)) ;
            }

            std::size_t operator()( Model_t::iterator& i ) const
            {
                return GPOINTER_TO_INT(&(*i)) ;
            }

            std::size_t operator()( const Model_t::const_iterator& i ) const
            {
                return GPOINTER_TO_INT(&(*i)) ;
            }

            std::size_t operator()( Model_t::const_iterator& i ) const
            {
                return GPOINTER_TO_INT(&(*i)) ;
            }
        } ;

        struct OrderFunc
        : public std::binary_function<Row_t, Row_t, bool>
        {
            bool operator() (
                  const Row_t&  a
                , const Row_t&  b
            )
            {
                const std::string&  order_artist_a = get<7>( a ) ; 
                const std::string&  order_artist_b = get<7>( b ) ; 

                const std::string&  order_album_a  = get<2>( a ) ; 
                const std::string&  order_album_b  = get<2>( b ) ; 

                const std::string&  order_date_a   = get<8>( a ) ; 
                const std::string&  order_date_b   = get<8>( b ) ; 

                gint64 order_track [2] = {
                      get<5>( a )
                    , get<5>( b )
                } ;

                if( order_artist_a < order_artist_b)
                    return true ;

                if( order_album_a < order_album_b )
                    return true ;

                if( order_date_a < order_date_b )
                    return true ;

                if( order_track[0] < order_track[1] )
                    return true ;

                return false ;
            }
        } ;

        struct DataModel
        : public sigc::trackable
        {
                Model_sp_t      m_realmodel;
                Signal1         m_changed;
                std::size_t     m_top_row ;

                DataModel()
                : m_top_row( 0 )
                {
                    m_realmodel = Model_sp_t(new Model_t); 
                }

                DataModel(Model_sp_t model)
                : m_top_row( 0 )
                {
                    m_realmodel = model; 
                }

                virtual void
                clear()
                {
                    m_realmodel->clear () ;
                    m_top_row = 0 ;
                } 

                virtual Signal1&
                signal_changed()
                {
                    return m_changed ;
                }

                virtual bool
                is_set ()
                {
                    return bool(m_realmodel) ;
                }

                virtual std::size_t
                size ()
                {
                    return m_realmodel->size() ;
                }

                virtual const Row_t&
                row (std::size_t row)
                {
                    return (*m_realmodel)[row] ;
                }

                virtual void
                set_current_row(
                    gint64 row
                )
                {
                    m_top_row = row ;
                }

                virtual void
                append_track(
                      ::MPXPD::sql_row_t&   r
                    , const MPX::Track_sp&  track
                )
                {
                    using boost::get ;

                    gint64 id = 0 ;

                    if( r.count("tracks_id") )
                    { 
                        id = get<gint64>(r["tracks_id"]) ;
                    }
                    else
                    {
                        g_critical("%s: No id for track, extremely suspicious", G_STRLOC) ;
                        throw std::logic_error("No ID given for track") ;
                    }

                    Row_t row (
                              r.count("tracks_name") ? get<std::string>(r["tracks_name"]) : ""
                            , r.count("track_artists_name") ? get<std::string>(r["track_artists_name"]) : "" 
                            , r.count("albums_name") ? get<std::string>(r["albums_name"]) : ""
                            , id 
                            , track 
                            , r.count("tracks_nr") ? get<gint64>(r["tracks_nr"]) : gint64(0)
                            , r.count("artists_id") ? get<gint64>(r["artists_id"]) : gint64(0) 
                            , r.count("artists_name") ? get<std::string>(r["artists_name"]) : ""
                            , ""
                            , r.count("tracks_duration") ? get<gint64>(r["tracks_duration"]) : 0 
                    ) ;

                    m_realmodel->push_back(row) ;

                    Model_t::iterator i = m_realmodel->end() ;
                    std::advance( i, -1 ) ;
                }

                void
                erase_track(gint64 id)
                {
                    for( Model_t::iterator i = m_realmodel->begin(); i != m_realmodel->end(); ++i )
                    {
                        gint64 model_id = get<3>( *i ) ;
                        if( model_id == id )
                        {
                            m_realmodel->erase( i ) ;
                            return ;
                        }
                    }
                }
        };

        typedef boost::shared_ptr<DataModel> DataModel_sp_t;

        struct DataModelFilter
        : public DataModel
        {
                typedef std::vector<gint64>                     IdVector_t ;
                typedef boost::shared_ptr<IdVector_t>           IdVector_sp ;
                typedef std::set<Model_t::const_iterator>       ModelIteratorSet_t ;
                typedef boost::shared_ptr<ModelIteratorSet_t>   ModelIteratorSet_sp ;
                typedef std::vector<ModelIteratorSet_sp>        IntersectVector_t ;
                typedef std::vector<Model_t::const_iterator>    RowRowMapping_t ;

                gint64  m_max_size_constraints_artist ;
                gint64  m_max_size_constraints_albums ;

                typedef std::map<std::string, ModelIteratorSet_sp>  FragmentCache_t ;

                struct IntersectSort
                    : std::binary_function<ModelIteratorSet_sp, ModelIteratorSet_sp, bool>
                {
                        bool operator() (
                              const ModelIteratorSet_sp&  a
                            , const ModelIteratorSet_sp&  b
                        )
                        {
                            return a->size() < b->size() ; 
                        }
                } ;

                RowRowMapping_t             m_mapping ;
                RowRowMapping_t             m_mapping_unfiltered ;
                FragmentCache_t             m_fragment_cache ;
                std::string                 m_current_filter ;
                StrV                        m_frags ;
                AQE::Constraints_t          m_constraints_ext ;
                AQE::Constraints_t          m_constraints_aqe ;
                boost::optional<gint64>     m_id_currently_playing ;
                boost::optional<gint64>     m_row_currently_playing_in_mapping ;
                IdVector_sp                 m_constraints_albums ;
                IdVector_sp                 m_constraints_artist ;
                bool                        m_cache_enabled ;
                Gtk::Widget*                m_widget ;

                DataModelFilter( DataModel_sp_t& model )

                    : DataModel( model->m_realmodel )
                    , m_max_size_constraints_artist( 0 )
                    , m_max_size_constraints_albums( 0 )
                    , m_cache_enabled( true )

                {
                    regen_mapping() ;
                }

                virtual ~DataModelFilter()
                {
                }

                void
                set_sizes(
                      gint64    max_size_artist
                    , gint64    max_size_albums
                )
                {
                    m_max_size_constraints_artist = max_size_artist ;
                    m_max_size_constraints_albums = max_size_albums ;
                }

                void
                get_sizes(
                      gint64&   max_size_artist
                    , gint64&   max_size_albums
                )
                {
                    max_size_artist = m_max_size_constraints_artist ; 
                    max_size_albums = m_max_size_constraints_albums ; 
                }

                virtual void
                clear ()
                {
                    DataModel::clear () ;
                    m_mapping.clear() ;
                    m_mapping_unfiltered.clear() ;
                    clear_active_track() ;
                    m_changed.emit( m_top_row, true ) ;
                } 

                void
                clear_active_track()
                {
                    m_row_currently_playing_in_mapping.reset() ;
                }

                void
                clear_fragment_cache()
                {
                    m_fragment_cache.clear() ;
                }

                void
                disable_fragment_cache()
                {
                    m_cache_enabled = false ;
                }

                void
                enable_fragment_cache()
                {
                    m_cache_enabled = true ;
                }

                void
                set_currently_playing_track(gint64 id)
                {
                    m_id_currently_playing = id ;
                    m_row_currently_playing_in_mapping.reset() ;

                    scan_for_currently_playing () ;
                }

                boost::optional<gint64>
                get_local_active_track ()
                {
                    return m_row_currently_playing_in_mapping ;
                }

                void
                add_synthetic_constraint(
                    const AQE::Constraint_t& c
                )
                {
                    m_constraints_ext.push_back( c ) ;    
                    regen_mapping() ;
                }

                void
                add_synthetic_constraint_quiet(
                    const AQE::Constraint_t& c
                )
                {
                    m_constraints_ext.push_back( c ) ;    
                }

                virtual void
                clear_synthetic_constraints(
                )
                {
                    m_constraints_ext.clear() ;
                    regen_mapping() ;
                }

                virtual void
                clear_synthetic_constraints_quiet(
                )
                {
                    m_constraints_ext.clear() ;
                }

                virtual std::size_t 
                size ()
                {
                    return m_mapping.size();
                }

                virtual const Row_t&
                row (std::size_t row)
                {
                    return *(m_mapping[row]);
                }

                void
                swap( std::size_t p1, std::size_t p2 )
                {
                    std::swap( m_mapping[p1], m_mapping[p2] ) ;

                    m_row_currently_playing_in_mapping.reset() ;
                    scan_for_currently_playing() ;

                    m_changed.emit( m_top_row, false ) ;
                }

                void
                erase( std::size_t p )
                {
                    RowRowMapping_t::iterator i = m_mapping.begin() ;

                    std::advance( i, p ) ;
                    m_mapping.erase( i ) ;

                    m_row_currently_playing_in_mapping.reset() ;
                    scan_for_currently_playing() ;

                    m_changed.emit( m_top_row, true ) ;
                }

                virtual void
                append_track(::MPXPD::sql_row_t& r, const MPX::Track_sp& track)
                {
                    DataModel::append_track(r, track);
                }

                void
                erase_track(gint64 id)
                {
                    DataModel::erase_track( id );
                }

                virtual void
                insert_track(
                      ::MPXPD::sql_row_t&   r
                    , const MPX::Track_sp&  track
                )
                {
                    const std::string&                    title             = get<std::string>(r["title"]) ;
                    const std::string&                    artist            = ""; // Util::row_get_artist_name( r ) ;
                    const std::string&                    album             = get<std::string>(r["album"]) ; 

                    const std::string&                    release_date      = get<std::string>(r["mb_release_date"]) ;

                    gint64                                id                = get<gint64>(r["tracks_id"]) ;
                    gint64                                track_n           = get<gint64>(r["track"]) ;
                    gint64                                time              = get<gint64>(r["time"]) ;
                    gint64                                id_artist         = get<gint64>(r["mpx_album_artist_id"]) ;

                    const std::string&                    order_artist      = ""; //Util::row_get_album_artist_name( r ) ; 

                    static OrderFunc order ;

                    Row_t row(
                          title
                        , artist
                        , album
                        , id
                        , track
                        , track_n 
                        , id_artist
                        , order_artist
                        , release_date
                        , time
                    ) ; 

                    Model_t::iterator i = m_realmodel->insert(
                          std::upper_bound(
                              m_realmodel->begin()
                            , m_realmodel->end()
                            , row
                            , order
                          )
                        , row
                    ) ;
                }

                virtual void
                regen_iterative_idle()
                {
                    regen_mapping_iterative() ;
                }

                virtual void
                regen_idle()
                {
                    regen_mapping() ;
                }
 
                virtual void
                set_filter(
                    const std::string& text
                )
                { 
                    using boost::get ;
                    using boost::algorithm::split;
                    using boost::algorithm::is_any_of;
                    using boost::algorithm::find_first;

                    AQE::Constraints_t aqe = m_constraints_aqe ;

                    m_constraints_aqe.clear() ;
                    m_frags.clear() ;

                    AQE::parse_advanced_query( m_constraints_aqe, text, m_frags ) ;

                    bool aqe_diff = m_constraints_aqe != aqe ;


                    if( !aqe_diff && ( text.substr( 0, text.size() - 1 ) == m_current_filter ) )
                    {
                        m_current_filter = text ;
                        Glib::signal_idle().connect_once( 
                              sigc::mem_fun( *this, &MPX::View::Tracks::DataModelFilter::regen_iterative_idle )) ; 
                    }
                    else
                    {
                        m_current_filter = text ;
                        //Util::window_set_busy( * dynamic_cast<Gtk::Window*>(m_widget->get_toplevel()) ) ;
                        //Util::window_set_idle( * dynamic_cast<Gtk::Window*>(m_widget->get_toplevel()) ) ;
                        Glib::signal_idle().connect_once( 
                              sigc::mem_fun( *this, &MPX::View::Tracks::DataModelFilter::regen_idle )) ; 
                    }
                }

                void
                scan_for_currently_playing_find_top_row(
                      const boost::optional<gint64>& id_top
                )
                {
                    using boost::get;

                    bool found_currently_playing = false, found_top_row = false ;

                    if( !m_id_currently_playing )
                    {
                        find_top_row( id_top ) ;
                        return ;
                    }

                    for( RowRowMapping_t::iterator i = m_mapping.begin(); i != m_mapping.end(); ++i )
                    {
                        gint64 val = get<3>(**i) ;

                        if( !found_currently_playing && val == m_id_currently_playing.get() )
                        {
                            m_row_currently_playing_in_mapping = std::distance( m_mapping.begin(), i ) ; 
                            found_currently_playing = true ;
                        }

                        if( !found_top_row && id_top )
                        {
                            if( val == id_top.get() )
                            {
                                m_top_row = std::distance( m_mapping.begin(), i ) ;
                                found_top_row = true ;
                            }
                        }

                        if( found_currently_playing && found_top_row )
                        {
                            return ;
                        }
                    }
                }

                void
                scan_for_currently_playing(
                )
                {
                    using boost::get;

                    if( !m_id_currently_playing )
                    {
                        m_row_currently_playing_in_mapping.reset() ;
                        return ;
                    }

                    gint64 id_realval = m_id_currently_playing.get() ; 

                    for( RowRowMapping_t::iterator i = m_mapping.begin(); i != m_mapping.end(); ++i )
                    {
                        g_message("FoundId: %lld", get<3>(**i)) ;
                        if( get<3>(**i) == id_realval ) 
                        {
                            m_row_currently_playing_in_mapping = std::distance( m_mapping.begin(), i ) ; 
                            return ;
                        }
                    }

                    m_row_currently_playing_in_mapping.reset() ;
                }

                void
                find_top_row(
                      boost::optional<gint64> id_top
                )
                {
                    if( id_top ) 
                    {
                        gint64 id_realval = id_top.get() ;

                        for( RowRowMapping_t::iterator i = m_mapping.begin() ; i != m_mapping.end() ; ++i )
                        {
                            if( boost::get<3>(**i) == id_realval )
                            {
                                m_top_row = std::distance( m_mapping.begin(), i ) ;
                                return ;
                            }
                        } 
                    }

                    m_top_row = 0 ;
                }

                virtual void
                cache_current_fragments(
                )
                {
                    using boost::get;
                    using boost::algorithm::split;
                    using boost::algorithm::is_any_of;
                    using boost::algorithm::find_first;

                    if( !m_cache_enabled )
                    {
                        return ;
                    }

                    if( m_frags.empty() )
                    {
                        return ;
                    }

                    std::vector<std::string> vec( 3 ) ;

                    for( std::size_t n = 0 ; n < m_frags.size(); ++n )
                    {
                        if( m_frags[n].empty() )
                        {
                            continue ;
                        }

                        if( m_fragment_cache.count( m_frags[n] )) 
                        {
                            continue ;
                        }
                        else
                        {
                            ModelIteratorSet_sp model_iterator_set ( new ModelIteratorSet_t ) ;

                            for( Model_t::const_iterator i = m_realmodel->begin(); i != m_realmodel->end(); ++i ) // determine all the matches
                            {
                                const Row_t& row = *i;

                                vec[0] = Glib::ustring(boost::get<0>(row)).lowercase() ;
                                vec[1] = Glib::ustring(boost::get<1>(row)).lowercase() ;
                                vec[2] = Glib::ustring(boost::get<2>(row)).lowercase() ;

                                if( Util::match_vec( m_frags[n], vec) )
                                {
                                    model_iterator_set->insert( i ) ; 
                                }
                            }
    
                            m_fragment_cache.insert( std::make_pair( m_frags[n], model_iterator_set )) ;
                        }
                    }
                }

                virtual void
                regen_mapping(
                )
                {
                    using boost::get;
                    using boost::algorithm::split;
                    using boost::algorithm::is_any_of;
                    using boost::algorithm::find_first;

                    if( m_realmodel->size() == 0 )
                        return ;

                    RowRowMapping_t new_mapping, new_mapping_unfiltered ;

                    boost::optional<gint64> id ; 
        
                    if( m_id_currently_playing )
                    {
                        id = m_id_currently_playing.get() ;
                    }
                    else if( m_top_row < m_mapping.size() )
                    {
                        id = get<3>(row( m_top_row )) ; 
                    }

                    m_row_currently_playing_in_mapping.reset() ;
                    m_top_row = 0 ;

                    if( m_frags.empty() && (m_constraints_ext.empty() && m_constraints_aqe.empty()) )
                    {
                        m_constraints_albums.reset() ;
                        m_constraints_artist.reset() ;

                        new_mapping.reserve( m_realmodel->size() ) ;
                        new_mapping_unfiltered.reserve( m_realmodel->size() ) ;

                        for( Model_t::iterator i = m_realmodel->begin(); i != m_realmodel->end(); ++i )
                        {
                            new_mapping.push_back( i ) ;
                            new_mapping_unfiltered.push_back( i ) ;
                        }
                    }
                    else
                    if( m_frags.empty() && !(m_constraints_ext.empty() && m_constraints_aqe.empty()) )
                    {
                        m_constraints_albums = IdVector_sp( new IdVector_t ) ; 
                        m_constraints_albums->resize( m_max_size_constraints_albums + 1 ) ;

                        m_constraints_artist = IdVector_sp( new IdVector_t ) ; 
                        m_constraints_artist->resize( m_max_size_constraints_artist + 1 ) ;

                        IdVector_t& constraints_artist = *(m_constraints_artist.get()) ;
                        IdVector_t& constraints_albums = *(m_constraints_albums.get()) ;

                        new_mapping.reserve( m_realmodel->size() ) ;
                        new_mapping_unfiltered.reserve( m_realmodel->size() ) ;

                        for( Model_t::const_iterator i = m_realmodel->begin(); i != m_realmodel->end(); ++i ) // determine all the matches
                        {
                            const Row_t& row = *i;

                            const MPX::Track_sp& t = get<4>(row);
                            const MPX::Track& track = *(t.get()) ;

                            if( !m_constraints_aqe.empty() && !AQE::match_track( m_constraints_aqe, t ))
                            {
                                continue ;
                            }

                            new_mapping_unfiltered.push_back( i ) ;

                            if( !m_constraints_ext.empty() && !AQE::match_track( m_constraints_ext, t ))
                            {                            
                                continue ;
                            }

                            gint64 id_album  = get<gint64>(track[ATTRIBUTE_MPX_ALBUM_ID].get()) ;
                            gint64 id_artist = get<6>(*i) ;

                            constraints_albums[id_album] = constraints_albums[id_album] + 1 ;
                            constraints_artist[id_artist] = constraints_artist[id_artist] + 1 ;

                            new_mapping.push_back( i ) ; 
                        }
                    }
                    else
                    {
                        IntersectVector_t intersect ;
                        intersect.reserve( m_frags.size() ) ; 

                        StrV vec( 3 ) ;

                        for( std::size_t n = 0 ; n < m_frags.size(); ++n )
                        {
                            if( m_frags[n].empty() )
                            {
                                continue ;
                            }

                            if( m_cache_enabled ) 
                            {
                                FragmentCache_t::iterator cache_iter = m_fragment_cache.find( m_frags[n] ) ;

                                if( cache_iter != m_fragment_cache.end() )
                                {
                                    intersect.push_back( cache_iter->second ) ;
                                    continue ;
                                }
                            }

                            ModelIteratorSet_sp model_iterator_set( new ModelIteratorSet_t ) ;

                            for( Model_t::const_iterator i = m_realmodel->begin(); i != m_realmodel->end(); ++i )
                            {
                                const Row_t& row = *i;

                                vec[0] = Glib::ustring(boost::get<0>(row)).lowercase() ;
                                vec[1] = Glib::ustring(boost::get<1>(row)).lowercase() ;
                                vec[2] = Glib::ustring(boost::get<2>(row)).lowercase() ;

                                if( Util::match_vec( m_frags[n], vec ))
                                {
                                    model_iterator_set->insert( i ) ; 
                                }
                            }

                            intersect.push_back( model_iterator_set ) ; 

                            if( m_cache_enabled && !m_fragment_cache.count( m_frags[n] ) && m_constraints_ext.empty() && m_constraints_aqe.empty() )
                            {
                                m_fragment_cache.insert( std::make_pair( m_frags[n], model_iterator_set )) ;
                            }
                        }

                        std::sort( intersect.begin(), intersect.end(), IntersectSort() ) ;
                        ModelIteratorSet_sp output( new ModelIteratorSet_t ) ; 

                        if( !intersect.empty() )
                        {
                            output = intersect[0] ;

                            for( std::size_t n = 1 ; n < intersect.size() ; ++n )
                            {
                                ModelIteratorSet_sp intersect_out( new ModelIteratorSet_t ) ;

                                std::set_intersection(

                                    // Set 1
                                      output->begin()
                                    , output->end()

                                    // Set 2 
                                    , intersect[n]->begin()
                                    , intersect[n]->end()

                                    // Result insertion 
                                    , std::inserter(
                                            *intersect_out.get()
                                           , intersect_out->end()
                                      )
                                ) ;

                                output = intersect_out ;
                            }
                        }

                        new_mapping.reserve( output->size() ) ;
                        new_mapping_unfiltered.reserve( output->size() ) ;

                        m_constraints_albums = IdVector_sp( new IdVector_t ) ; 
                        m_constraints_albums->resize( m_max_size_constraints_albums + 1 ) ;

                        m_constraints_artist = IdVector_sp( new IdVector_t ) ; 
                        m_constraints_artist->resize( m_max_size_constraints_artist + 1 ) ;

                        IdVector_t& constraints_artist = *(m_constraints_artist.get()) ;
                        IdVector_t& constraints_albums = *(m_constraints_albums.get()) ;

                        for( ModelIteratorSet_t::iterator i = output->begin() ; i != output->end(); ++i )
                        {
                            const MPX::Track_sp& t = get<4>(**i);
                            const MPX::Track& track = *(t.get()) ;

                            if( !m_constraints_aqe.empty() && !AQE::match_track( m_constraints_aqe, t ))
                            {
                                continue ;
                            }

                            new_mapping_unfiltered.push_back( *i ) ;

                            if( !m_constraints_ext.empty() && !AQE::match_track( m_constraints_ext, t ))
                            {
                                continue ;
                            }

                            gint64 id_album  = get<gint64>(track[ATTRIBUTE_MPX_ALBUM_ID].get()) ;
                            gint64 id_artist = get<6>(**i) ;

                            constraints_albums[id_album] = constraints_albums[id_album] + 1 ;
                            constraints_artist[id_artist] = constraints_artist[id_artist] + 1 ;

                            new_mapping.push_back( *i ) ;
                        }
                    }

                    std::swap( new_mapping, m_mapping ) ;
                    std::swap( new_mapping_unfiltered, m_mapping_unfiltered ) ;

                    scan_for_currently_playing_find_top_row( id ) ;

                    if( m_row_currently_playing_in_mapping )
                    {
                        m_top_row = m_row_currently_playing_in_mapping.get() ;
                    }

                    m_changed.emit( m_top_row, new_mapping.size() != m_mapping.size() ) ; 
                }

                void
                regen_mapping_iterative(
                )
                {
                    using boost::get;
                    using boost::algorithm::split;
                    using boost::algorithm::is_any_of;
                    using boost::algorithm::find_first;

                    if( m_realmodel->size() == 0 )
                        return ;

                    RowRowMapping_t new_mapping, new_mapping_unfiltered ;

                    boost::optional<gint64> id ; 
        
                    if( m_id_currently_playing )
                    {
                        id = m_id_currently_playing.get() ;
                    }
                    else
                    if( m_top_row < m_mapping.size() )
                    {
                        id = get<3>(row( m_top_row )) ; 
                    }

                    m_row_currently_playing_in_mapping.reset() ;
                    m_top_row = 0 ;

                    if( m_frags.empty() && (m_constraints_ext.empty() && m_constraints_aqe.empty()) )
                    {
                        m_constraints_albums.reset() ;
                        m_constraints_artist.reset() ;

                        new_mapping.reserve( m_mapping_unfiltered.size() ) ;
                        new_mapping_unfiltered.reserve( m_mapping_unfiltered.size() ) ;

                        for( RowRowMapping_t::iterator i = m_mapping_unfiltered.begin(); i != m_mapping_unfiltered.end(); ++i )
                        {
                            new_mapping_unfiltered.push_back( *i ) ;
                            new_mapping.push_back( *i ) ;
                        }
                    }
                    else
                    if( m_frags.empty() && !(m_constraints_ext.empty() && m_constraints_aqe.empty()) )
                    {
                        m_constraints_albums = IdVector_sp( new IdVector_t ) ; 
                        m_constraints_albums->resize( m_max_size_constraints_albums + 1 ) ;

                        m_constraints_artist = IdVector_sp( new IdVector_t ) ; 
                        m_constraints_artist->resize( m_max_size_constraints_artist + 1 ) ;

                        IdVector_t& constraints_artist = *(m_constraints_artist.get()) ;
                        IdVector_t& constraints_albums = *(m_constraints_albums.get()) ;

                        new_mapping.reserve( m_mapping_unfiltered.size() ) ;
                        new_mapping_unfiltered.reserve( m_mapping_unfiltered.size() ) ;

                        for( RowRowMapping_t::const_iterator i = m_mapping_unfiltered.begin(); i != m_mapping_unfiltered.end(); ++i )
                        {
                            const Row_t& row = **i ;

                            const MPX::Track_sp& t = get<4>(row);
                            const MPX::Track& track = *(t.get()) ;

                            if( !m_constraints_aqe.empty() && !AQE::match_track( m_constraints_aqe, t ))
                            {
                                continue ;
                            }

                            new_mapping_unfiltered.push_back( *i ) ;

                            if( !m_constraints_ext.empty() && !AQE::match_track( m_constraints_ext, t )) 
                            {
                                continue ;
                            }

                            gint64 id_album  = get<gint64>(track[ATTRIBUTE_MPX_ALBUM_ID].get()) ;
                            gint64 id_artist = get<6>(**i) ;

                            constraints_albums[id_album] = constraints_albums[id_album] + 1 ;
                            constraints_artist[id_artist] = constraints_artist[id_artist] + 1 ;

                            new_mapping.push_back( *i ) ; 
                        }
                    }
                    else
                    {
                        IntersectVector_t intersect ; 
                        intersect.reserve( m_frags.size() ) ;

                        StrV vec( 3 ) ;

                        for( std::size_t n = 0 ; n < m_frags.size(); ++n )
                        {
                            if( m_frags[n].empty() ) 
                            {
                                continue ;
                            }

                            if( m_cache_enabled ) 
                            {
                                FragmentCache_t::iterator cache_iter = m_fragment_cache.find( m_frags[n] ) ;

                                if( cache_iter != m_fragment_cache.end() )
                                {
                                    intersect.push_back( cache_iter->second ) ;
                                    continue ;
                                }
                            }

                            ModelIteratorSet_sp model_iterator_set ( new ModelIteratorSet_t ) ;

                            for( RowRowMapping_t::const_iterator i = m_mapping_unfiltered.begin(); i != m_mapping_unfiltered.end(); ++i )
                            {
                                const Row_t& row = **i ;

                                vec[0] = Glib::ustring(boost::get<0>(row)).lowercase() ;
                                vec[1] = Glib::ustring(boost::get<1>(row)).lowercase() ;
                                vec[2] = Glib::ustring(boost::get<2>(row)).lowercase() ;

                                if( Util::match_vec( m_frags[n], vec ))
                                {
                                    model_iterator_set->insert( *i ) ; 
                                }
                            }

                            intersect.push_back( model_iterator_set ) ; 

                            // We cannot cache results which are preconstrainted i.e. through ext or aqe constraints.
                            // If there is more than 1 frag then the result set, since we go through a previous projection,
                            // the controller mapping (m_mapping_unfiltered) is already constrainted through the preceding frags
                            // i.e. in previous calls to regen_mapping_iterative m_mapping_unfiltered has already become a
                            // projection of the base model through the previous frags

                            if(
                                m_cache_enabled
                                        &&
                                m_frags.size() == 1
                                        &&
                                m_constraints_ext.empty()
                                        &&
                                m_constraints_aqe.empty()
                            )
                            {
                                m_fragment_cache.insert( std::make_pair( m_frags[n], model_iterator_set )) ;
                            }
                        }

                        std::sort( intersect.begin(), intersect.end(), IntersectSort() ) ;
                        ModelIteratorSet_sp output( new ModelIteratorSet_t ) ; 

                        if( !intersect.empty() )
                        {
                            output = intersect[0] ;

                            for( std::size_t n = 1 ; n < intersect.size() ; ++n )
                            {
                                ModelIteratorSet_sp intersect_out( new ModelIteratorSet_t ) ;

                                std::set_intersection(

                                    // Set 1
                                      output->begin()
                                    , output->end()

                                    // Set 2 
                                    , intersect[n]->begin()
                                    , intersect[n]->end()

                                    // Result insertion 
                                    , std::inserter(
                                            *intersect_out.get()
                                           , intersect_out->end()
                                      )
                                ) ;

                                output = intersect_out ;
                            }
                        }

                        new_mapping.reserve( m_realmodel->size() ) ;
                        new_mapping_unfiltered.reserve( m_realmodel->size() ) ; 

                        m_constraints_albums = IdVector_sp( new IdVector_t ) ; 
                        m_constraints_albums->resize( m_max_size_constraints_albums + 1 ) ;

                        m_constraints_artist = IdVector_sp( new IdVector_t ) ; 
                        m_constraints_artist->resize( m_max_size_constraints_artist + 1 ) ;

                        IdVector_t& constraints_artist = *(m_constraints_artist.get()) ;
                        IdVector_t& constraints_albums = *(m_constraints_albums.get()) ;

                        for( ModelIteratorSet_t::iterator i = output->begin() ; i != output->end(); ++i )
                        {
                            const MPX::Track_sp& t = get<4>(**i);
                            const MPX::Track& track = *(t.get()) ;

                            if( !m_constraints_aqe.empty() && !AQE::match_track( m_constraints_aqe, t ))
                            {
                                continue ;
                            }

                            new_mapping_unfiltered.push_back( *i ) ;

                            if( !m_constraints_ext.empty() && !AQE::match_track( m_constraints_ext, t ))
                            {
                                continue ;
                            }

                            gint64 id_album  = get<gint64>(track[ATTRIBUTE_MPX_ALBUM_ID].get()) ;
                            gint64 id_artist = get<6>(**i) ;

                            constraints_albums[id_album] = constraints_albums[id_album] + 1 ;
                            constraints_artist[id_artist] = constraints_artist[id_artist] + 1 ;

                            new_mapping.push_back( *i ) ;
                        }
                    }

                    std::swap( new_mapping, m_mapping ) ;
                    std::swap( new_mapping_unfiltered, m_mapping_unfiltered ) ;

                    scan_for_currently_playing_find_top_row( id ) ;

                    m_changed.emit( m_top_row, new_mapping.size() != m_mapping.size() ) ; 
                }
        };

        typedef boost::shared_ptr<DataModelFilter> DataModelFilter_sp_t;

        class Column
        {
                int                 m_width ;
                int                 m_column ;
                std::string         m_title ;
                Pango::Alignment    m_alignment ;
                
            public:

                Column (std::string const& title)
                : m_width( 0 )
                , m_column( 0 )
                , m_title( title )
                , m_alignment( Pango::ALIGN_LEFT )
                {
                }

                ~Column ()
                {
                }

                void
                set_width (int width)
                {
                    if( m_width != width )
                    {
                        m_width = width; 
                    }
                }

                int
                get_width ()
                {
                    return m_width;
                }

                void
                set_column (int column)
                {
                    m_column = column;
                }

                int
                get_column ()
                {
                    return m_column;
                }

                void
                set_title (std::string const& title)
                {
                    m_title = title;
                }

                std::string const&
                get_title ()
                {
                    return m_title;
                }

                void
                set_alignment(
                    Pango::Alignment align
                )
                {
                    m_alignment = align ;
                }


                Pango::Alignment
                get_alignment(
                )
                {
                    return m_alignment ;
                }


                void
                render_header(
                      Cairo::RefPtr<Cairo::Context>&    cairo
                    , Gtk::Widget&                      widget
                    , int                               xpos
                    , int                               ypos
                    , int                               rowheight
                    , int                               column
                    , const ThemeColor&                 color
                )
                {
                    using boost::get;

                    cairo->save() ;

                    cairo->rectangle(
                          xpos + 5
                        , ypos + 6
                        , m_width
                        , rowheight
                    ) ;

                    cairo->clip() ;

                    cairo->move_to(
                          xpos + 5
                        , ypos + 6
                    ) ;

                    cairo->set_operator(Cairo::OPERATOR_OVER);

                    cairo->set_source_rgba(
                          color.r
                        , color.g
                        , color.b
                        , color.a 
                    ) ; 

                    Glib::RefPtr<Pango::Layout> layout = widget.create_pango_layout(""); 

                    layout->set_markup(
                          (boost::format("<b>%s</b>") % Glib::Markup::escape_text(m_title).c_str()).str()
                    ) ;

                    layout->set_ellipsize(
                          Pango::ELLIPSIZE_END
                    ) ;

                    layout->set_width(
                          (m_width-10)*PANGO_SCALE
                    ) ;

                    layout->set_alignment(
                          m_alignment
                    ) ;

                    pango_cairo_show_layout(
                          cairo->cobj()
                        , layout->gobj()
                    ) ;

                    cairo->reset_clip() ;

                    cairo->restore() ;
                }

                void
                render(
                      Cairo::RefPtr<Cairo::Context>&    cairo
                    , Gtk::Widget&                      widget
                    , const Row_t&                      datarow
                    , int                               row
                    , int                               xpos
                    , int                               ypos
                    , int                               rowheight
                    , const ThemeColor&                 color
                    , double                            alpha
                )
                {
                    using boost::get ;

                    cairo->save() ;

                    cairo->set_operator(Cairo::OPERATOR_OVER) ;

                    cairo->set_source_rgba(
                          color.r
                        , color.g
                        , color.b
                        , color.a * alpha 
                    ) ; 

                    cairo->rectangle(
                          xpos
                        , ypos
                        , m_width
                        , rowheight
                    ) ;

                    cairo->clip();

                    {
                      cairo->move_to(
                            xpos + 6
                          , ypos + 2
                      ) ;

                      std::string str ;

                      switch( m_column )
                      {
                          case 0:
                              str = get<0>(datarow);
                              break;
                          case 1:
                              str = get<1>(datarow);
                              break;
                          case 2:
                              str = get<2>(datarow);
                              break;
                          case 3:
                              str = get<3>(datarow);
                              break;
                          case 5:
                              str = boost::lexical_cast<std::string>(get<5>(datarow)) ;
                              break;
                          case 9:
                              {
                                gint64 time_ = get<9>(datarow) ;
                                str = ((boost::format("%02d:%02d") % (time_/60) % (time_ % 60)).str()) ;
                              }
                              break;
                      }

                      Glib::RefPtr<Pango::Layout> layout; 

                      layout = widget.create_pango_layout( str );

                      layout->set_ellipsize(
                            Pango::ELLIPSIZE_END
                      ) ;

                      layout->set_width(
                            (m_width - 12) * PANGO_SCALE
                      ) ;

                      layout->set_alignment(
                            m_alignment
                      ) ;

                      pango_cairo_show_layout(
                            cairo->cobj()
                          , layout->gobj()
                      ) ;

                    }

                    cairo->reset_clip();

                    cairo->restore() ;
                  }
        };

        typedef boost::shared_ptr<Column>               Column_sp_t ;
        typedef std::vector<Column_sp_t>                Columns ;

        typedef sigc::signal<void, MPX::Track_sp, bool, unsigned>   SignalTrackActivated ;
        typedef sigc::signal<void>                                  SignalVAdjChanged ;
        typedef sigc::signal<void>                                  SignalFindAccepted ;
        typedef sigc::signal<void, const std::string&>              SignalFindPropagate ;

        class Class
        : public Gtk::DrawingArea
        {
            public:

                DataModelFilter_sp_t                m_model ;

            private:

                int                                 m_row_height ;
                int                                 m_row_start ;
                int                                 m_visible_height ;

                Columns                             m_columns ;

                PropAdj                             m_prop_vadj ;
                PropAdj                             m_prop_hadj ;

                boost::optional<boost::tuple<Model_t::const_iterator, std::size_t> >  m_selection ;

                IdV                                 m_dnd_idv ;
                bool                                m_dnd ;
                gint64                              m_clicked_row ;
                bool                                m_clicked ;
                bool                                m_highlight ;

                boost::optional<gint64>             m_id_currently_playing ;
                boost::optional<gint64>             m_hover_track ;
                boost::optional<gint64>             m_terminal_track ;

                Glib::RefPtr<Gdk::Pixbuf>           m_pb_play_l ;
                Glib::RefPtr<Gdk::Pixbuf>           m_pb_hover_l ;

                std::set<int>                       m_collapsed ;
                std::set<int>                       m_fixed ;
                gint64                              m_fixed_total_width ;
        
                Gtk::Entry                        * m_SearchEntry ;
                Gtk::Window                       * m_SearchWindow ;
                Gtk::HBox                         * m_SearchHBox ;
                Gtk::Button                       * m_SearchButton ;

                sigc::connection                    m_search_changed_conn ; 
                bool                                m_search_active ;

                Glib::RefPtr<Gtk::UIManager>        m_refUIManager ;
                Glib::RefPtr<Gtk::ActionGroup>      m_refActionGroup ;
                Gtk::Menu*                          m_pMenuPopup ;

                typedef sigc::signal<void, const std::string&> SignalMBID ;

                SignalMBID _signal_0 ; 
                SignalMBID _signal_1 ; 

                SignalTrackActivated                m_SIGNAL_track_activated ;
                SignalVAdjChanged                   m_SIGNAL_vadj_changed ;
                SignalFindAccepted                  m_SIGNAL_find_accepted ;
                SignalFindPropagate                 m_SIGNAL_find_propagate ;

                Interval<std::size_t>               m_Model_I ;

                void
                initialize_metrics ()
                {
                    PangoContext *context = gtk_widget_get_pango_context (GTK_WIDGET (gobj()));

                    PangoFontMetrics *metrics = pango_context_get_metrics (context,
                                                                            GTK_WIDGET (gobj())->style->font_desc, 
                                                                            pango_context_get_language (context));

                    m_row_height = (pango_font_metrics_get_ascent (metrics)/PANGO_SCALE) + 
                                   (pango_font_metrics_get_descent (metrics)/PANGO_SCALE) + 5 ;

                    const int visible_area_pad = 2 ;

                    m_row_start = m_row_height + visible_area_pad ;
                }

                void
                on_vadj_value_changed ()
                {
                    if( m_model->size() )
                    {
                        m_model->set_current_row( get_upper_row() ) ;        
                        queue_draw ();
                        m_SIGNAL_vadj_changed.emit() ;
                    }
                }

            protected:

                virtual bool
                on_focus_in_event (GdkEventFocus* G_GNUC_UNUSED)
                {
                    if( !m_selection && m_model->size() )
                    {
                        std::size_t row = get_upper_row();
                        m_selection = (boost::make_tuple(m_model->m_mapping[row], row));
                    }

                    queue_draw() ;

                    return false ;
                }

                bool
                key_press_event (GdkEventKey * event)
                {
                    if( event->is_modifier )
                        return false ;

                    if( !m_model->size() )
                        return false ;

                    if( m_search_active )
                    {
                        switch( event->keyval )
                        {
                            case GDK_Up:
                            case GDK_KP_Up:
                                find_prev_match() ;
                                return true ;

                            case GDK_Down:
                            case GDK_KP_Down:
                                find_next_match() ;
                                return true ;

                            case GDK_Escape:
                                cancel_search() ;
                                return true ;

                            case GDK_Tab:
                                cancel_search() ;
                                return false ;

                            case GDK_Return:
                            case GDK_KP_Enter:
                            case GDK_ISO_Enter:
                            case GDK_3270_Enter:
                                cancel_search() ;
                                goto continue_matching ;
        
                            default: ;
                        }

                        GdkEvent *new_event = gdk_event_copy( (GdkEvent*)(event) ) ;
                        g_object_unref( ((GdkEventKey*)new_event)->window ) ;
                        ((GdkEventKey *) new_event)->window = GDK_WINDOW(g_object_ref(G_OBJECT(GTK_WIDGET(m_SearchWindow->gobj())->window))) ;
                        gtk_widget_event(GTK_WIDGET(m_SearchEntry->gobj()), new_event) ;
                        gdk_event_free(new_event) ;

                        return true ;
                    }

                    continue_matching:

                    int step = 0 ; 
                    int origin = m_selection ? boost::get<1>(m_selection.get()) : 0 ;

                    switch( event->keyval )
                    {
                        case GDK_Delete:
                        {
                            if( m_selection )
                            {
                                std::size_t p = origin ;
                                m_selection.reset() ;
                                m_model->erase( p ) ;
                            }
                            return true ;
                        }

                        case GDK_Return:
                        case GDK_KP_Enter:
                        case GDK_ISO_Enter:
                        case GDK_3270_Enter:
                        {
                            if( m_search_active )
                            {
                                cancel_search() ;
                            }

                            if( m_selection )
                            {
                                using boost::get;

                                MPX::Track_sp track = get<4>(*(get<0>(m_selection.get()))) ;
                                m_SIGNAL_track_activated.emit( track, !(event->state & GDK_CONTROL_MASK), get<1>(m_selection.get())) ;
                            }

                            return true;
                        }

                        case GDK_Up:
                        case GDK_KP_Up:
                        case GDK_Page_Up:
                        {
                            if( event->keyval == GDK_Page_Up )
                            {
                                step = get_page_size() ; 
                            }
                            else
                            {
                                step = 1 ;
                            }

                            if( event->state & GDK_SHIFT_MASK )
                            {
                                if( m_Model_I.in( origin - step ))
                                {
                                    m_model->swap( origin, origin-step ) ;
                                    m_selection = boost::make_tuple(m_model->m_mapping[origin-step], origin-step) ;
                                }
                        
                                return true ;
                            }

                            if( !m_selection || !get_row_is_visible( origin ))
                            {
                                select_row( get_upper_row() ) ;
                            }
                            else
                            {
                                std::size_t row = ((origin-step) < 0 ) ? 0 : (origin-step) ;

                                if( row < get_upper_row() ) 
                                {
                                    m_prop_vadj.get_value()->set_value( std::max<int>( int(get_upper_row()) - step, row )) ; 
                                }
    
                                select_row( row ) ;
                            }

                            m_hover_track.reset() ;

                            return true;
                        }

                        case GDK_Home:
                        {
                            select_row( 0 ) ;
                            scroll_to_row( 0 ) ;

                            return true ;
                        }

                        case GDK_End:
                        {
                            select_row( m_model->size() - 1 ) ;
                            scroll_to_row( m_model->size() - get_page_size() ) ;

                            return true ;
                        }

                        case GDK_Down:
                        case GDK_KP_Down:
                        case GDK_Page_Down:
                        {
                            if( event->keyval == GDK_Page_Down )
                            {
                                step = get_page_size() ; 
                            }
                            else
                            {
                                step = 1 ;
                            }

                            if( event->state & GDK_SHIFT_MASK )
                            {
                                if( m_Model_I.in( origin + step ))
                                {
                                    m_model->swap( origin, origin+step ) ;
                                    m_selection = boost::make_tuple(m_model->m_mapping[origin+step], origin+step) ;
                                }
                        
                                return true ;
                            }

                            if( !m_selection || !get_row_is_visible( origin ))
                            {
                                select_row( get_upper_row() ) ;
                            }
                            else
                            {
                                std::size_t row = std::min<std::size_t>( origin+step, m_model->size()-1 ) ;

                                if( row >= get_lower_row() ) 
                                {
                                    m_prop_vadj.get_value()->set_value( std::min<std::size_t>(get_upper_row()+step, row )) ;
                                }

                                select_row( row ) ;
                            }

                            m_hover_track.reset() ;

                            return true;
                        }

                        default:

                            if( !m_search_active )
                            {
                                int x, y, x_root, y_root ;

                                dynamic_cast<Gtk::Window*>(get_toplevel())->get_position( x_root, y_root ) ;

                                x = x_root + get_allocation().get_x() ;
                                y = y_root + get_allocation().get_y() + get_allocation().get_height() ;

                                m_SearchWindow->set_size_request( m_columns[1]->get_width(), - 1 ) ;
                                m_SearchWindow->move( x, y ) ;
                                m_SearchWindow->show() ;

                                send_focus_change( *m_SearchEntry, true ) ;

                                GdkEvent *new_event = gdk_event_copy( (GdkEvent*)(event) ) ;
                                g_object_unref( ((GdkEventKey*)new_event)->window ) ;
                                gtk_widget_realize( GTK_WIDGET(m_SearchWindow->gobj()) ) ;
                                ((GdkEventKey *) new_event)->window = GDK_WINDOW(g_object_ref(G_OBJECT(GTK_WIDGET(m_SearchWindow->gobj())->window))) ;
                                gtk_widget_event(GTK_WIDGET(m_SearchEntry->gobj()), new_event) ;
                                gdk_event_free(new_event) ;

                                m_search_active = true ;

                                return false ;
                            }
                    }

                    return false ;
                }

                void
                send_focus_change(
                      Gtk::Widget&  w
                    , bool          in
                    )
                {
                    GtkWidget * widget = w.gobj() ;

                    GdkEvent *fevent = gdk_event_new (GDK_FOCUS_CHANGE);

                    g_object_ref (widget);

                   if( in )
                      GTK_WIDGET_SET_FLAGS( widget, GTK_HAS_FOCUS ) ;
                    else
                      GTK_WIDGET_UNSET_FLAGS( widget, GTK_HAS_FOCUS ) ;

                    fevent->focus_change.type   = GDK_FOCUS_CHANGE;
                    fevent->focus_change.window = GDK_WINDOW(g_object_ref( widget->window )) ;
                    fevent->focus_change.in     = in;

                    gtk_widget_event( widget, fevent ) ;

                    g_object_notify(
                          G_OBJECT (widget)
                        , "has-focus"
                    ) ;

                    g_object_unref( widget ) ;
                    gdk_event_free( fevent ) ;
                }

                bool
                on_button_press_event (GdkEventButton * event)
                {
                    using boost::get;

                    {
                        cancel_search() ;
                        grab_focus() ;

                        int x = event->x ;

                        if( event->y < (m_row_height+4))
                        {
                            int p = 32 ;

                            for( std::size_t n = 0; n < m_columns.size() ; ++n )
                            {
                                int w = m_columns[n]->get_width() ;

                                if( (x >= p) && (x <= p + w) && !m_fixed.count(n) )
                                {
                                    column_set_collapsed( n, !m_collapsed.count( n ) ) ;
                                    break ;
                                }

                                p += w ;
                            }
                            return true;
                        }

                        Limiter<int64_t> row ( 
                              Limiter<int64_t>::ABS_ABS
                            , 0
                            , m_model->size() - 1
                            , get_upper_row() + (event->y-m_row_start) / m_row_height
                        ) ;

                        if( x >= 32 ) 
                        {
                            m_selection     = boost::make_tuple( m_model->m_mapping[row], row ) ;
                            m_clicked_row   = row ;
                            m_clicked       = true ;
                            queue_draw() ;
                        }
                        else
                        if( x >= 16 )
                        {
                            /* reserved for future use for clicking on the hover 'play' icon */
                        }
                        else
                        {
                            gint64 id = get<3>( m_model->row( row )) ;

                            if( m_terminal_track && id == m_terminal_track.get() )
                            {
                                m_terminal_track.reset() ;
                            }
                            else
                            {
                                m_terminal_track = id ;
                            }
                
                            queue_draw() ;
                        }
                    }

                    if( event->type == GDK_2BUTTON_PRESS && event->button == 1 )
                    {
                        if( event->y < m_row_height )
                            return false ;

                        Limiter<int64_t> row ( 
                              Limiter<int64_t>::ABS_ABS
                            , 0
                            , m_model->m_mapping.size()
                            , get_upper_row() + (event->y-m_row_start) / m_row_height
                        ) ;

                        Interval<std::size_t> i (
                              Interval<std::size_t>::IN_EX
                            , 0
                            , m_model->size()
                        ) ;

                        if( i.in( row )) 
                        {
                            MPX::Track_sp track = get<4>(m_model->row(row)) ;
                            m_SIGNAL_track_activated.emit( track, true, row ) ;
                        }
                    
                        return true ;
                    }
               
/* 
                    if( event->button == 3 )
                    {
                        m_clicked = false ;
                        m_pMenuPopup->popup(event->button, event->time) ;                            
                        return true ;
                    }
*/

                    return true;
                }

                bool
                on_button_release_event (GdkEventButton * event)
                {
                    m_clicked = false ;
                    return true ;
                }

                bool
                on_leave_notify_event(
                    GdkEventCrossing* G_GNUC_UNUSED
                )
                {
                    m_hover_track.reset() ;
                    queue_draw () ;

                    return true ;
                }

                bool
                on_motion_notify_event(
                    GdkEventMotion* event
                )
                {
                    int x_orig, y_orig;
                    GdkModifierType state;

                    if (event->is_hint)
                    {
                        gdk_window_get_pointer( event->window, &x_orig, &y_orig, &state ) ;
                    }
                    else
                    {
                        x_orig = int( event->x ) ;
                        y_orig = int( event->y ) ;
                        state  = GdkModifierType( event->state ) ;
                    }

                    gint64 row = get_upper_row() + ( y_orig - m_row_start ) / m_row_height ;

                    if( !m_clicked )
                    {
                            if( m_Model_I.in( row ) && y_orig >= m_row_start ) 
                            {
                                m_hover_track = row ;
                                queue_draw_area( 0, m_row_start, 32, get_allocation().get_height() - m_row_start ) ;
                            }
                            else
                            {
                                m_hover_track.reset() ;
                                queue_draw_area( 0, m_row_start, 32, get_allocation().get_height() - m_row_start ) ;
                            }
                    }
/*
                    else
                    {
                            if( row != m_clicked_row )
                            {
                                if( m_Model_I.in( row )) 
                                {
                                        m_model->swap( row, m_clicked_row ) ;
                                        m_selection = (boost::make_tuple(m_model->m_mapping[row], row));
                                        m_clicked_row = row ;
                                }
                            }
                    }
*/

                    return true ;
                }

                void
                configure_vadj(
                      std::size_t   upper
                    , std::size_t   page_size
                    , std::size_t   step_increment
                )
                {
                    if( m_prop_vadj.get_value() )
                    {
                        m_prop_vadj.get_value()->set_upper( upper ) ; 
                        m_prop_vadj.get_value()->set_page_size( page_size ) ; 
                        m_prop_vadj.get_value()->set_step_increment( step_increment ) ; 
                    }
                }

                bool
                on_configure_event(
                    GdkEventConfigure* event
                )        
                {
                    const double column_width_collapsed = 40. ;

                    m_visible_height = event->height - m_row_start ;

                    configure_vadj(
                          m_model->size()
                        , get_page_size()
                        , 1
                    ) ;

                    int width = event->width - 32 ;

                    double column_width_calculated = (double(width) - double(m_fixed_total_width) - double(column_width_collapsed*double(m_collapsed.size()))) / (m_columns.size() - m_collapsed.size() - m_fixed.size()) ;

                    for( std::size_t n = 0; n < m_columns.size(); ++n )
                    {
                        if( !m_fixed.count( n ) )
                        {
                            m_columns[n]->set_width(m_collapsed.count( n ) ? column_width_collapsed : column_width_calculated ) ; 
                        }
                    }

                    return false;
                }

                inline bool
                compare_id_to_optional(
                      const Row_t&                      row
                    , const boost::optional<gint64>&    id
                )
                {
                    if( id && id.get() == boost::get<3>( row ))
                        return true ;

                    return false ;
                }

                template <typename T>
                inline bool
                compare_val_to_optional(
                      const T&                          val
                    , const boost::optional<T>&         cmp
                )
                {
                    if( cmp && cmp.get() == val ) 
                        return true ;

                    return false ;
                }


                bool
                on_expose_event (GdkEventExpose *event)
                {
                    const Gtk::Allocation& a = get_allocation();

                    boost::shared_ptr<IYoukiThemeEngine> theme = services->get<IYoukiThemeEngine>("mpx-service-theme") ;

                    const ThemeColor& c_text        = theme->get_color( THEME_COLOR_TEXT ) ;
                    const ThemeColor& c_text_sel    = theme->get_color( THEME_COLOR_TEXT_SELECTED ) ;
                    const ThemeColor& c_rules_hint  = theme->get_color( THEME_COLOR_BASE_ALTERNATE ) ;

                    Cairo::RefPtr<Cairo::Context> cairo = get_window()->create_cairo_context(); 

                    cairo->rectangle(
                          0
                        , 0
                        , a.get_width()
                        , a.get_height() 
                    ) ;
    
                    cairo->clip() ;

                    cairo->set_operator( Cairo::OPERATOR_OVER ) ;

                    std::size_t row = get_upper_row() ;
                    std::size_t idx = 0 ;

                    int col     = 0 ;
                    int cnt     = get_page_size() + 1 ; 

                    int xpos    = 0 ;
                    int ypos    = m_row_start ;

                    const int inner_pad = 1 ;

                    if( event->area.y <= m_row_start )
                    {
                            xpos = 32 ;

                            for( Columns::iterator i = m_columns.begin(); i != m_columns.end(); ++i, ++col )
                            {
                                (*i)->render_header(
                                    cairo
                                  , *this
                                  , xpos
                                  , 0
                                  , m_row_start
                                  , col
                                  , c_text
                                ) ;

                                xpos += (*i)->get_width() ; 
                            }
                    }

                    cairo->set_operator( Cairo::OPERATOR_OVER ) ;

                    //// ROWS

                    if( event->area.width > 32 )
                    {
                            cairo->save() ;

                            cairo->rectangle(
                                  0
                                , 0
                                , a.get_width()
                                , (int(a.get_height())/m_row_height) * m_row_height
                            ) ;
            
                            cairo->clip() ;

                            while( m_model->is_set() && cnt && m_Model_I.in( row ) ) 
                            {
                                if( !(idx % 2) ) 
                                {
                                    GdkRectangle r ;

                                    r.x       = 32 + inner_pad ;
                                    r.y       = ypos + inner_pad ;
                                    r.width   = a.get_width() - 2 * inner_pad - 32 ;
                                    r.height  = m_row_height - 2 * inner_pad ;

                                    RoundedRectangle(
                                          cairo
                                        , r.x
                                        , r.y
                                        , r.width
                                        , r.height
                                        , rounding
                                    ) ;

                                    cairo->set_source_rgba(
                                          c_rules_hint.r 
                                        , c_rules_hint.g
                                        , c_rules_hint.b 
                                        , c_rules_hint.a
                                    ) ;

                                    cairo->fill() ;
                                }

                                ypos += m_row_height ;
                                row  ++ ;
                                cnt  -- ;
                                idx  ++ ;
                            }

                            boost::optional<std::size_t> row_select ;

                            if( m_selection )
                            {
                                Interval<std::size_t> i (
                                      Interval<std::size_t>::IN_IN
                                    , get_upper_row()
                                    , get_upper_row() + get_page_size()
                                ) ;

                                std::size_t row = boost::get<1>(m_selection.get()) ; 
                                row_select = row ;

                                if( i.in( row ) )
                                {
                                        GdkRectangle r ;

                                        r.x         = 32 + inner_pad ; 
                                        r.y         = inner_pad + (row - get_upper_row()) * m_row_height + m_row_start ;
                                        r.width     = a.get_width() - 2*inner_pad - 32 ; 
                                        r.height    = m_row_height  - 2*inner_pad ;

                                        theme->draw_selection_rectangle(
                                              cairo
                                            , r
                                            , has_focus()
                                        ) ;
                                    }
                            }

                            row     = get_upper_row() ;

                            col     = 0 ;
                            cnt     = get_page_size() + 1 ; 

                            ypos    = m_row_start ;

                            bool currently_playing = bool( m_id_currently_playing ) ;

                            while( m_model->is_set() && cnt && m_Model_I.in( row ) ) 
                            {
                                xpos = 32 ;

                                const Row_t& r_data = m_model->row( row ) ;

                                if( currently_playing ) 
                                {
                                    double alpha = 0.50 ;

                                    if( compare_id_to_optional( r_data, m_id_currently_playing )) 
                                    {
                                        alpha = 1.0 ;
                                    }

                                    for(Columns::const_iterator i = m_columns.begin(); i != m_columns.end(); ++i)
                                    {
                                        (*i)->render(
                                              cairo
                                            , *this
                                            , r_data 
                                            , row
                                            , xpos
                                            , ypos
                                            , m_row_height
                                            , (row_select && (row == row_select.get())) ? c_text_sel : c_text
                                            , alpha 
                                        ) ;

                                        xpos += (*i)->get_width() ; 
                                    }
                                }
                                else
                                {
                                    for(Columns::const_iterator i = m_columns.begin(); i != m_columns.end(); ++i)
                                    {
                                        (*i)->render(
                                              cairo
                                            , *this
                                            , r_data 
                                            , row
                                            , xpos
                                            , ypos
                                            , m_row_height
                                            , (row_select && (row == row_select.get())) ? c_text_sel : c_text
                                            , 1.0
                                        ) ;

                                        xpos += (*i)->get_width() ; 
                                    }
                                }

                                ypos += m_row_height ;
                                row  ++ ;
                                cnt  -- ;
                            }

                            cairo->restore() ;
    
                            if( true /*m_render_dashes*/)
                            {
                                std::valarray<double> dashes ( 3 ) ;
                                dashes[0] = 0. ;
                                dashes[1] = 3. ;
                                dashes[2] = 0. ;

                                xpos = 32 ;

                                cairo->save() ;

                                const ThemeColor& c_treelines = theme->get_color( THEME_COLOR_TREELINES ) ;

                                cairo->set_line_width(
                                      .3
                                ) ;
                                cairo->set_dash(
                                      dashes
                                    , 0
                                ) ;

                                Columns::iterator i2 = m_columns.end() ;
                                --i2 ;

                                for( Columns::const_iterator i = m_columns.begin(); i != i2; ++i )
                                {
                                    xpos += (*i)->get_width() ; 

                                    cairo->set_source_rgba(
                                          c_treelines.r
                                        , c_treelines.g
                                        , c_treelines.b
                                        , c_treelines.a
                                    ) ;

                                    cairo->move_to(
                                          xpos
                                        , 0 
                                    ) ; 

                                    cairo->line_to(
                                          xpos
                                        , a.get_height()
                                    ) ;

                                    cairo->stroke() ;
                                }

                                cairo->restore(); 
                            }
                    }
    
                    //// ICONS

                    const int icon_lateral = 16 ;

                    ypos    = m_row_start ;
                    cnt     = get_page_size() + 1 ; 
                    row     = get_upper_row() ;

                    while( m_model->is_set() && cnt && m_Model_I.in( row ) )
                    {
                        const Row_t& r_data = m_model->row( row ) ;

                        enum Skip
                        {
                              SKIP_SKIP_NONE
                            , SKIP_SKIP_PLAY        = 1 << 0
                            , SKIP_SKIP_TERMINAL    = 1 << 1
                        } ;

                        enum Icon
                        {
                              ICON_PLAY
                            , ICON_TERMINAL
                        } ;

                        int skip = SKIP_SKIP_NONE ;

                        const int icon_y = ypos + (m_row_height - icon_lateral) / 2 ;

                        const int icon_x[2] = { 16, 0 } ;

                        if( compare_id_to_optional( r_data, m_id_currently_playing )) 
                        {
                            Gdk::Cairo::set_source_pixbuf(
                                  cairo
                                , m_pb_play_l
                                , icon_x[ICON_PLAY]
                                , icon_y 
                            ) ;
                            cairo->rectangle(
                                  icon_x[ICON_PLAY]
                                , icon_y 
                                , icon_lateral
                                , icon_lateral
                            ) ;
                            cairo->fill () ;

                            skip &= SKIP_SKIP_PLAY ;
                        }

                        if( compare_val_to_optional<gint64>( row, m_hover_track )) 
                        {
                            if( !( skip & SKIP_SKIP_PLAY))
                            {
                                Gdk::Cairo::set_source_pixbuf(
                                      cairo
                                    , m_pb_hover_l
                                    , icon_x[ICON_PLAY]
                                    , icon_y 
                                ) ;
                                cairo->rectangle(
                                      icon_x[ICON_PLAY] 
                                    , icon_y 
                                    , icon_lateral
                                    , icon_lateral
                                ) ;
                                cairo->fill () ;
                            }

/*
                            if( !( skip & SKIP_SKIP_TERMINAL))
                            {
                                Gdk::Cairo::set_source_pixbuf(
                                      cairo
                                    , m_pb_terminal
                                    , icon_x[ICON_TERMINAL]
                                    , icon_y 
                                ) ;
                                cairo->rectangle(
                                      icon_x[ICON_TERMINAL] 
                                    , icon_y 
                                    , icon_lateral
                                    , icon_lateral
                                ) ;
                                cairo->clip () ;
                                cairo->paint_with_alpha( 0.5 ) ;
                                cairo->reset_clip() ;
                            }
*/
                        }

                        ypos += m_row_height ;
                        row  ++ ;
                        cnt  -- ;
                    }

                    return true;
                }

                void
                on_model_changed(
                      std::size_t   position
                    , bool          size_changed
                )
                {
                    if( size_changed ) 
                    {
                        m_Model_I = Interval<std::size_t> (
                                 Interval<std::size_t>::IN_EX
                                , 0
                                , m_model->size()
                        ) ;

                        configure_vadj(
                              m_model->size()
                            , get_page_size()
                            , 1
                        ) ;
                    }

                    scroll_to_row( position ) ;
                    m_selection.reset() ;
                    queue_draw() ;
                }

                static gboolean
                list_view_set_adjustments(
                    GtkWidget*obj,
                    GtkAdjustment*hadj,
                    GtkAdjustment*vadj, 
                    gpointer data
                )
                {
                    if( vadj )
                    {
                            g_object_set(G_OBJECT(obj), "vadjustment", vadj, NULL); 
                            g_object_set(G_OBJECT(obj), "hadjustment", hadj, NULL);

                            Class & view = *(reinterpret_cast<Class*>(data));

                            view.m_prop_vadj.get_value()->signal_value_changed().connect(
                                sigc::mem_fun(
                                    view,
                                    &Class::on_vadj_value_changed
                            ));
                    }

                    return TRUE;
                }

                bool
                query_tooltip(
                      int                                   tooltip_x
                    , int                                   tooltip_y
                    , bool                                  keypress
                    , const Glib::RefPtr<Gtk::Tooltip>&     tooltip
                )
                {
                    std::size_t row = (double( tooltip_y ) - m_row_start) / double(m_row_height) ;

                    MPX::Track_sp t = boost::get<4>( m_model->row(row) ) ;
                    const MPX::Track& track = *(t.get()) ;

                    boost::shared_ptr<Covers> covers = services->get<Covers>("mpx-service-covers") ;
                    Glib::RefPtr<Gdk::Pixbuf> cover ;

                    const std::string& mbid = boost::get<std::string>( track[ATTRIBUTE_MB_ALBUM_ID].get() ) ;

                    Gtk::Image * image = Gtk::manage( new Gtk::Image ) ;

                    if( covers->fetch(
                          mbid
                        , cover
                    ))
                    {   
                        image->set( cover ) ;
                        tooltip->set_custom( *image ) ;
                        return true ;
                    }

                    return false ;
                }

            public:

                inline void
                set_terminal_id(
                    gint64    id
                )
                {
                    m_terminal_track = id ; 
                    queue_draw() ;
                }

                inline boost::optional<gint64>
                get_terminal_id(
                )
                {
                    return m_terminal_track ; 
                }

                inline void
                clear_terminal_id(
                )
                {
                    m_terminal_track.reset() ;
                    queue_draw() ;
                }
    
                inline std::size_t
                get_page_size(
                )
                {
                    if( m_visible_height && m_row_height )
                        return m_visible_height / m_row_height ; 
                    else
                        return 0 ;
                }

                inline std::size_t
                get_upper_row(
                )
                {
                    if( m_prop_vadj.get_value() )
                        return m_prop_vadj.get_value()->get_value() ;
                    else
                        return 0 ;
                }

                inline std::size_t
                get_lower_row(
                )
                {
                    return m_prop_vadj.get_value()->get_value() + get_page_size() ;
                }

                inline bool
                get_row_is_visible(
                      std::size_t   row
                )
                {
                    std::size_t up = get_upper_row() ;

                    Interval<std::size_t> i (
                          Interval<std::size_t>::IN_IN
                        , up 
                        , up + get_page_size()
                    ) ;
            
                    return i.in( row ) ;
                }

                void
                set_highlight(bool highlight)
                {
                    m_highlight = highlight;
                    queue_draw ();
                }

                void
                set_model(DataModelFilter_sp_t model)
                {
                    if( m_model )
                    {
                        boost::optional<gint64> active_track = m_model->m_id_currently_playing ;

                        m_model = model;
                        m_model->m_widget = this ;

                        m_model->m_id_currently_playing = active_track ;
                        m_model->scan_for_currently_playing() ;
                    }
                    else
                    {
                        m_model = model;
                        m_model->m_widget = this ;
                    }

                    m_model->signal_changed().connect(
                        sigc::mem_fun(
                            *this,
                            &Class::on_model_changed
                    ));

                    on_model_changed( 0, true ) ;
                }

                void
                append_column(
                      Column_sp_t   column
                )
                {
                    m_columns.push_back(column) ;
                }

                SignalTrackActivated&
                signal_track_activated()
                {
                    return m_SIGNAL_track_activated ;
                }

                SignalVAdjChanged&
                signal_vadj_changed()
                {
                    return m_SIGNAL_vadj_changed ;
                }

                SignalFindAccepted&
                signal_find_accepted()
                {
                    return m_SIGNAL_find_accepted ;
                }

                SignalFindPropagate&
                signal_find_propagate()
                {
                    return m_SIGNAL_find_propagate ;
                }

                void
                clear_active_track()
                {
                    m_id_currently_playing.reset() ;
                    m_model->clear_active_track() ;
                }

                void
                set_currently_playing_track(gint64 id)
                {
                    m_id_currently_playing = id ;
                    m_model->set_currently_playing_track( id ) ;
                    queue_draw () ;
                }

                const boost::optional<gint64>&
                get_local_active_track()
                {
                    return m_model->m_row_currently_playing_in_mapping ;
                }

                void
                column_set_collapsed(
                      int       column
                    , bool      collapsed
                )
                {
                    if( collapsed )
                    {
                        m_collapsed.insert( column ) ;
                        queue_resize () ;
                    }
                    else
                    {
                        m_collapsed.erase( column ) ;
                        queue_resize () ;
                    }
                }

                void
                column_set_fixed(
                      int       column
                    , bool      fixed
                    , int       width = 0
                )
                {
                    if( fixed )
                    {
                        m_fixed.insert( column ) ;
                        m_fixed_total_width += width ;
                        m_columns[column]->set_width( width ) ;
                        queue_resize () ;
                    }
                    else
                    {
                        m_fixed.erase( column ) ;
                        m_fixed_total_width -= m_columns[column]->get_width() ; 
                        queue_resize () ;
                    }
                }

                void
                scroll_to_id(
                      gint64 id
                )
                {
                    for( DataModelFilter::RowRowMapping_t::iterator i = m_model->m_mapping.begin() ; i != m_model->m_mapping.end(); ++i )
                    {
                        const Row_t& row = **i ;
                        if( boost::get<3>(row) == id )
                        {
                            Limiter<std::size_t> d ( 
                                  Limiter<std::size_t>::ABS_ABS
                                , 0
                                , m_model->m_mapping.size() - get_page_size()
                                , std::distance( m_model->m_mapping.begin(), i )
                            ) ;

                            m_prop_vadj.get_value()->set_value( d ) ; 
                            break ;
                        }
                    } 
                }

                void
                scroll_to_row(
                      std::size_t row
                )
                {
                    if( m_visible_height && m_row_height && m_prop_vadj.get_value() && m_model )
                    {
                        Limiter<std::size_t> d ( 
                              Limiter<std::size_t>::ABS_ABS
                            , 0
                            , m_model->m_mapping.size() - get_page_size()
                            , row 
                        ) ;

                        if( m_model->m_mapping.size() < get_page_size()) 
                            m_prop_vadj.get_value()->set_value( 0 ) ; 
                        else
                        if( row > (m_model->m_mapping.size() - get_page_size()) )
                            m_prop_vadj.get_value()->set_value( m_model->m_mapping.size() - get_page_size() ) ; 
                        else
                            m_prop_vadj.get_value()->set_value( d ) ; 

                        m_hover_track.reset() ;
                    }
                }

                void
                select_row(
                      std::size_t row
                )
                {
                    Interval<std::size_t> i (
                          Interval<std::size_t>::IN_EX
                        , 0
                        , m_model->size()
                    ) ;

                    if( i.in( row ))
                    {
                        m_selection = (boost::make_tuple(m_model->m_mapping[row], row));
                        queue_draw() ;
                    }
                }

                void
                clear_selection(
                )
                {
                    m_selection.reset() ;
                    queue_draw() ;
                }

            protected:

                void
                find_next_match()
                {
                    using boost::get ;

                    Glib::ustring text = m_SearchEntry->get_text().casefold() ;

                    if( text.empty() )
                    {
                        return ;
                    }

                    DataModelFilter::RowRowMapping_t::iterator i = m_model->m_mapping.begin(); 

                    if( m_selection )
                    {
                        std::advance( i, get<1>(m_selection.get()) ) ;
                        ++i ;
                    }

                    for( ; i != m_model->m_mapping.end(); ++i )
                    {
                        Glib::ustring match = Glib::ustring(get<0>(**i)).casefold() ;

                        if( match.length() && match.substr( 0, text.length()) == text.substr( 0, text.length()) )
                        {
                            std::size_t d = std::distance( m_model->m_mapping.begin(), i ) ; 
                            scroll_to_row( d ) ;
                            select_row( d ) ;
                            return ;
                        }
                    }
                }

                void
                find_prev_match()
                {
                    using boost::get ;

                    Glib::ustring text = m_SearchEntry->get_text().casefold() ;

                    if( text.empty() )
                    {
                        return ;
                    }

                    DataModelFilter::RowRowMapping_t::iterator i = m_model->m_mapping.begin(); 

                    if( m_selection )
                    {
                        std::advance( i, get<1>(m_selection.get()) ) ;
                        --i ; 
                    }

                    for( ; i >= m_model->m_mapping.begin(); --i )
                    {
                        Glib::ustring match = Glib::ustring(get<0>(**i)).casefold() ;

                        if( match.length() && match.substr( 0, text.length()) == text.substr( 0, text.length()) )
                        {
                            std::size_t d = std::distance( m_model->m_mapping.begin(), i ) ; 
                            scroll_to_row( d ) ;
                            select_row( d ) ;
                            return ;
                        }
                    }
                }

                void
                on_search_entry_changed()
                {
                    using boost::get ;

                    Glib::ustring text = m_SearchEntry->get_text().casefold() ;

                    if( text.empty() )
                    {
                        return ;
                    }

                    DataModelFilter::RowRowMapping_t::iterator i = m_model->m_mapping.begin(); 
               
                    for( ; i != m_model->m_mapping.end(); ++i )
                    {
                        Glib::ustring match = Glib::ustring(get<0>(**i)).casefold() ;

                        if( match.length() && match.substr( 0, text.length()) == text.substr( 0, text.length()) )
                        {
                            std::size_t d = std::distance( m_model->m_mapping.begin(), i ) ; 
                            scroll_to_row( d ) ; 
                            select_row( d ) ;
                            return ;
                        }
                    }

                    clear_selection() ;
                    scroll_to_row( 0 ) ;
                }

                void
                on_search_entry_activated()
                {
                    cancel_search() ;

                    m_SIGNAL_find_accepted.emit() ;
                }

                bool
                on_search_window_focus_out(
                      GdkEventFocus* G_GNUC_UNUSED
                )
                {
                    cancel_search() ;
                    return false ;
                }

                void
                on_search_button_clicked(
                )
                {
                    std::string text = m_SearchEntry->get_text() ;
                    cancel_search() ;

                    m_SIGNAL_find_propagate.emit( text ) ;
                }

                void
                on_show_only_this_album() 
                {
                    if( m_selection )
                    {
                        const Row_t& row = *(boost::get<0>(m_selection.get())) ;
                        const MPX::Track_sp& t = get<4>(row);
                        const MPX::Track& track = *(t.get()) ;

                        _signal_0.emit( get<std::string>(track[ATTRIBUTE_MB_ALBUM_ID].get()));
                    }
                }

                void
                on_show_only_this_artist() 
                {
                    if( m_selection )
                    {
                        const Row_t& row = *(boost::get<0>(m_selection.get())) ;
                        const MPX::Track_sp& t = get<4>(row);
                        const MPX::Track& track = *(t.get()) ;

                        _signal_1.emit( get<std::string>(track[ATTRIBUTE_MB_ALBUM_ARTIST_ID].get()));
                    }
                }

            public:

                void
                cancel_search()
                {
                    if( !m_search_active )
                        return ;

                    send_focus_change( *m_SearchEntry, false ) ;

                    m_SearchWindow->hide() ;
                    m_search_changed_conn.block () ;
                    m_SearchEntry->set_text("") ;
                    m_search_changed_conn.unblock () ;
                    m_search_active = false ;
                }

            protected:

                virtual void
                on_realize()
                {
                    Gtk::DrawingArea::on_realize() ;
                    initialize_metrics();
                    queue_resize();
                }

            public:

                SignalMBID&
                signal_only_this_album_mbid()
                {
                    return _signal_0 ;
                }

                SignalMBID&
                signal_only_this_artist_mbid()
                {
                    return _signal_1 ;
                }

                Class ()

                        : ObjectBase( "YoukiClassTracks" )
                        , m_prop_vadj( *this, "vadjustment", (Gtk::Adjustment*)( 0 ))
                        , m_prop_hadj( *this, "hadjustment", (Gtk::Adjustment*)( 0 ))
                        , m_dnd( false )
                        , m_clicked_row( 0 ) 
                        , m_clicked( false )
                        , m_highlight( false )
                        , m_fixed_total_width( 0 )
                        , m_search_active( false )

                {
                    boost::shared_ptr<IYoukiThemeEngine> theme = services->get<IYoukiThemeEngine>("mpx-service-theme") ;
                    const ThemeColor& c = theme->get_color( THEME_COLOR_BASE ) ;
                    Gdk::Color cgdk ;
                    cgdk.set_rgb_p( c.r, c.g, c.b ) ; 
                    modify_bg( Gtk::STATE_NORMAL, cgdk ) ;
                    modify_base( Gtk::STATE_NORMAL, cgdk ) ;

                    m_pb_play_l  = Gdk::Pixbuf::create_from_file( Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "row-play.png" )) ;
                    m_pb_hover_l = Gdk::Pixbuf::create_from_file( Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "row-hover.png" )) ;

                    set_flags(Gtk::CAN_FOCUS);
                    add_events(Gdk::EventMask(GDK_KEY_PRESS_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK ));

                    ((GtkWidgetClass*)(G_OBJECT_GET_CLASS(G_OBJECT(gobj()))))->set_scroll_adjustments_signal = 
                            g_signal_new ("set_scroll_adjustments",
                                      G_OBJECT_CLASS_TYPE (G_OBJECT_CLASS (G_OBJECT_GET_CLASS(G_OBJECT(gobj())))),
                                      GSignalFlags (G_SIGNAL_RUN_FIRST),
                                      0,
                                      NULL, NULL,
                                      g_cclosure_user_marshal_VOID__OBJECT_OBJECT, G_TYPE_NONE, 2, GTK_TYPE_ADJUSTMENT, GTK_TYPE_ADJUSTMENT);

                    g_signal_connect(G_OBJECT(gobj()), "set_scroll_adjustments", G_CALLBACK(list_view_set_adjustments), this);

                    /*
                    signal_query_tooltip().connect(
                        sigc::mem_fun(
                              *this
                            , &Class::query_tooltip
                    )) ;

                    set_has_tooltip( true ) ;
                    */

                    m_SearchEntry = Gtk::manage( new Gtk::Entry ) ;
                    //gtk_widget_realize( GTK_WIDGET(m_SearchEntry->gobj()) ) ;
                    m_SearchEntry->show() ;

                    m_SearchHBox = Gtk::manage( new Gtk::HBox ) ;
                    m_SearchButton = Gtk::manage( new Gtk::Button ) ;
                    m_SearchButton->signal_clicked().connect(
                        sigc::mem_fun(
                              *this
                            , &Class::on_search_button_clicked
                    )) ;

                    Gtk::Image * img = Gtk::manage( new Gtk::Image ) ;
                    img->set( Gtk::Stock::FIND, Gtk::ICON_SIZE_MENU ) ;
                    img->show() ;
    
                    m_SearchButton->set_image( *img ) ;

                    m_search_changed_conn = m_SearchEntry->signal_changed().connect(
                            sigc::mem_fun(
                                  *this
                                , &Class::on_search_entry_changed
                    )) ;
    
                    m_SearchEntry->signal_activate().connect(
                            sigc::mem_fun(
                                  *this
                                , &Class::on_search_entry_activated
                    )) ;

                    m_SearchWindow = new Gtk::Window( Gtk::WINDOW_POPUP ) ;
                    m_SearchWindow->set_decorated( false ) ;

                    m_SearchWindow->signal_focus_out_event().connect(
                            sigc::mem_fun(
                                  *this
                                , &Class::on_search_window_focus_out
                    )) ;

                    signal_focus_out_event().connect(
                            sigc::mem_fun(
                                  *this
                                , &Class::on_search_window_focus_out
                    )) ;

                    signal_key_press_event().connect(
                            sigc::mem_fun(
                                  *this
                                , &Class::key_press_event
                    ), true ) ;

                    m_SearchHBox->pack_start( *m_SearchEntry, true, true, 0 ) ;
                    m_SearchHBox->pack_start( *m_SearchButton, true, true, 0 ) ;

                    m_SearchWindow->add( *m_SearchHBox ) ;
                    m_SearchHBox->show_all() ;

                    property_can_focus() = true ;

/*
                    m_refActionGroup = Gtk::ActionGroup::create() ;
                    m_refActionGroup->add( Gtk::Action::create("ContextMenu", "Context Menu")) ;

                    m_refActionGroup->add( Gtk::Action::create("ContextShowAlbum", "Show only this Album"),
                        sigc::mem_fun(*this, &Class::on_show_only_this_album)) ;
                    m_refActionGroup->add( Gtk::Action::create("ContextShowArtist", "Show only this Artist"),
                        sigc::mem_fun(*this, &Class::on_show_only_this_artist)) ;
    
                    m_refUIManager = Gtk::UIManager::create() ;
                    m_refUIManager->insert_action_group(m_refActionGroup) ;

                    std::string ui_info =
                    "<ui>"
                    "   <popup name='PopupMenu'>"
                    "       <menuitem action='ContextShowAlbum'/>"
                    "       <menuitem action='ContextShowArtist'/>"
                    "   </popup>"
                    "</ui>" ;

                    m_refUIManager->add_ui_from_string( ui_info ) ;
                    m_pMenuPopup = dynamic_cast<Gtk::Menu*>(m_refUIManager->get_widget("/PopupMenu")) ;
*/
                }

                virtual ~Class ()
                {
                }
        };
}
}
}

#endif // _YOUKI_TRACK_LIST_HH
