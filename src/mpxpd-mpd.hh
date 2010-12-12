#ifndef MPXPD_MPD_HH
#define MPXPD_MPD_HH

#include "config.h"

#include "util.hh"

#include "mpx/mpxpd-basic-types.hh"
#include "mpx/mpxpd-sql.hh"
#include "mpx/com/view-tracks-mpxpd-playlist.hh"
#include "mpx/mpx-services.hh"
#include "mpx/mpx-i-mpd.hh"

#include <iostream>
#include <exception>
#include <stdexcept>
#include <string>
#include <set>
#include <map>
#include <vector>
#include <sqlite3.h>
#include <glib.h>
#include <glib-object.h>
#include <boost/variant.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <glibmm.h>
#include <libglademm.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

extern "C" {
#include <libmpd/libmpd.h>
#include <libmpd/libmpd-playlist.h>
}

using namespace MPX ;

namespace MPXPD
{
    class MPD
    : public Service::Base
    , public IMPD 
    {
        public:

            typedef sigc::signal<void>              signal_next_song_t ;
            typedef sigc::signal<void>              signal_playlist_changed_t ;
            typedef sigc::signal<void,int,int>      signal_pos_duration_t ; 
            typedef sigc::signal<void,int>          signal_state_t ;
            typedef sigc::signal<void,int>          signal_volume_t ;

        protected:

            MPX::View::Playlist::DataModel_sp_t m_model ;

            SQL &                               sql ;
            Glib::RefPtr<Gnome::Glade::Xml>     xml ;  
            MpdObj*                             obj ;

            artists_t                           artists ;

            int                                 m_state ;
            std::string                         m_hostname ;
            int                                 m_port ;

            signal_pos_duration_t               signal_pos_duration_s ;
            signal_next_song_t                  signal_next_song_s ;
            signal_state_t                      signal_state_s ;
            signal_playlist_changed_t           signal_playlist_changed_s ;
            signal_volume_t                     signal_volume_s ;

            sigc::connection                    m_timeout_status_cb ;

        public:

            signal_volume_t&
            signal_volume()
            {
                return signal_volume_s ;
            }

            signal_playlist_changed_t&
            signal_playlist_changed()
            {
                return signal_playlist_changed_s ;
            }

            signal_state_t&
            signal_state()
            {
                return signal_state_s ;
            }

            signal_pos_duration_t& 
            signal_position_duration()
            {
                return signal_pos_duration_s ;
            }

            signal_next_song_t& 
            signal_next_song()
            {
                return signal_next_song_s ;
            }

            MPD(
                  SQL&  sql_
                , Glib::RefPtr<Gnome::Glade::Xml> xml_
                , const std::string& hostname = "localhost"
                , const int port              = 6600
            ) 
            : Service::Base("mpx-service-mpd")
            , sql( sql_ ) 
            , xml( xml_ )
            , obj( 0 )
            , m_state( 0 )
            , m_hostname( hostname )
            , m_port( port )
            {
            }

            void
            set_connection_data(
                  const std::string&    hostname
                , int                   port
        
            )
            {
                m_hostname = hostname ; 
                m_port = port ;
            }

            virtual void
            update()
            {
                mpd_database_update_dir( obj, "/" ) ;
            }

            void
            disconnect()
            {
                if( obj )
                {
                    m_timeout_status_cb.disconnect() ;

                    mpd_disconnect( obj ) ;

                    if( obj )
                    {
                        mpd_free( obj ) ;
                    }

                    obj = 0 ;
                }
            }

            void
            connect()
            {
                char * host = g_strdup( m_hostname.c_str() ) ;
                obj = mpd_new( host, m_port, NULL ) ; 
                g_free( host ) ;

                mpd_set_connection_timeout( obj, 500 ) ;

                int connstate = mpd_connect( obj ) ;

                if( connstate != MPD_OK )
                {
                    signal_disconnected_s.emit( connstate ) ;
                    mpd_free( obj ) ;
                    obj = 0 ;
                    return ;
                }

                signal_connected_s.emit() ;

                m_state = mpd_player_get_state( obj ) ;
            }

            void
            start_monitoring()
            {
                if( obj )
                {
                    mpd_signal_connect_status_changed(obj,status_changed,this) ;
                    mpd_signal_connect_error(obj,error_cb,this) ;
                    mpd_signal_connect_connection_changed(obj,connch_cb,this) ;

                    m_timeout_status_cb = Glib::signal_timeout().connect( sigc::mem_fun( *this, &MPD::status_cb), 250) ;
                }
            }

            void
            give_model(
                MPX::View::Playlist::DataModel_sp_t model
            )
            {
                m_model = model ;
            }

            int
            get_state()
            {
                return mpd_player_get_state( obj ) ;
            }

            int
            get_volume()
            {
                if( obj )
                    return mpd_status_get_volume( obj ) ;
                else
                    return 0 ;
            }

        private:

            void
            append_mpd_data_to_playlist(
                    MpdData*  data
            )
            {
                if( m_model->m_realmodel->empty() ) 
                {
                    for( data ; data != NULL ; data=mpd_data_get_next(data))
                    {
                        sql_rows_t rows ;

                        char * sqlstr = sqlite3_mprintf( "SELECT * FROM track_view WHERE uri = '%q';", data->song->file ) ;

                        sql.sql_get( sqlstr, rows ) ;
                        sqlite3_free( sqlstr ) ;

                        if( !rows.empty() )
                        {
                            sql_rows_t::iterator i = rows.begin() ;
                            MPXPD::Util::insert_row_playlist( *i, m_model, data->song->id ) ;
                        }
                    }
                }
                else
                {
                    MPX::View::Playlist::Model_t::iterator iter = m_model->m_realmodel->begin() ;

                    for( data ; data != NULL ; data=mpd_data_get_next(data))
                    {
                        sql_rows_t rows ;

                        char * sqlstr = sqlite3_mprintf( "SELECT * FROM track_view WHERE uri = '%q';", data->song->file ) ;

                        sql.sql_get( sqlstr, rows ) ;
                        sqlite3_free( sqlstr ) ;

                        if( !rows.empty() )
                        {
                            sql_rows_t::iterator i = rows.begin() ;

                            if( std::distance( m_model->m_realmodel->begin(), iter) < m_model->m_realmodel->size()) 
                            {
                                int cur_id = data->song->id ;
                                int pls_id = boost::get<8>(*iter);

                                if( cur_id != pls_id )
                                {
                                    MPXPD::Util::set_row_playlist( *i, m_model, data->song->id, iter ) ;
                                }
                            }
                            else
                            {
                                MPXPD::Util::insert_row_playlist( *i, m_model, data->song->id ) ;
                            }
                        }

                        ++iter ;
                    }
                }
            }

            static void
            status_changed( MpdObj* mi, ChangedStatusType what, gpointer data )
            {
                MPXPD::MPD& mpd = *(reinterpret_cast<MPXPD::MPD*>(data)) ;

                if(what&MPD_CST_VOLUME)
                {
                    mpd.signal_volume_s.emit( mpd_status_get_volume( mi )) ; 
                }

                if(what&MPD_CST_STATE)
                {
                    mpd.m_state = mpd_player_get_state( mi ) ;
                    mpd.signal_state_s.emit( mpd.m_state ) ; 
                }

                if(what&MPD_CST_NEXTSONG)
                {
                    mpd.signal_next_song_s.emit() ; 
                }

                if(what&MPD_CST_PLAYLIST)
                {
                    int new_length = mpd_playlist_get_playlist_length(mi);
                    int old_length = mpd.m_model->m_realmodel->size() ; 

                    if( new_length == 0 )
                    {
                        mpd.m_model->clear();
                    }
                    else 
                    if( old_length == 0 )
                    {
                        mpd.append_mpd_data_to_playlist( mpd_playlist_get_changes(mi,-1)) ;
                    }
                    else
                    {
                        mpd.append_mpd_data_to_playlist( mpd_playlist_get_changes(mi,-1)) ;

                        if( new_length < old_length ) 
                        {
                            MPX::View::Playlist::Model_t::iterator iter1 = mpd.m_model->m_realmodel->begin() ;
                            MPX::View::Playlist::Model_t::iterator iter2 = mpd.m_model->m_realmodel->end() ;
                            std::advance( iter1, new_length ) ;
                            mpd.m_model->erase( iter1, iter2 ) ;
                        }
                    }

                    mpd.signal_playlist_changed_s.emit() ; 
                }

                if(what&MPD_CST_UPDATING)
                {
                    if( !mpd_status_db_is_updating( mpd.obj ) ) 
                    {
                        mpd.m_updating = false ;
                        mpd.signal_updated_s.emit() ;
                    }
                    else
                    {
                        mpd.m_updating = true ;
                        mpd.signal_updating_s.emit() ;
                    }
                }
            }

            bool
            status_cb()
            {
                if( !obj )
                {
                    return false ;
                }

                if( !mpd_check_connected( obj ))
                {
                    return false ;
                }

                if( mpd_check_error( obj ))
                {
                    return false ;
                }

                if( mpd_status_update( obj ))
                {
                    signal_disconnected_s.emit(0) ;
                    return false ;
                }

                int pos = mpd_status_get_elapsed_song_time(obj) ;
                int dur = mpd_status_get_total_song_time(obj) ;
    
                signal_pos_duration_s.emit( pos, dur ) ;

                return true;
            }

    
            static int
            error_cb(
                  MpdObj*   mi
                , int       id
                , char*     msg
                , gpointer  data
            )
            {
                g_message("Error Message: %s", msg ) ;

                MPD & mpd = *(reinterpret_cast<MPD*>(data)) ;

                if( id == MPD_NOT_CONNECTED || id == MPD_STATUS_FAILED || id == MPD_SERVER_ERROR || id == MPD_FATAL_ERROR )
                {
                    mpd_free( mpd.obj ) ; 
                    mpd.obj = 0 ;
                    mpd.signal_disconnected_s.emit( id ) ;
                    return true ;
                }

                return false ; 
            }

            static void
            connch_cb(
                  MpdObj*   mi
                , int       conn
                , gpointer  data
            )
            {
                MPD& mpd = *(reinterpret_cast<MPD*>(data)) ;

                if( !conn ) 
                {
                    if( mpd.obj )
                    {
                        mpd_free( mpd.obj ) ; 

                        mpd.obj = 0 ;
                    }

                    mpd.signal_disconnected_s.emit(0) ;
                }
            }

        public:

            virtual ~MPD()
            {
            }

            static void
            for_each_song(gpointer,gpointer) ;

            void
            get_queue(
                  sql_rows_t& rows
            ) const ;

            const mpd_Song*
            get_current_song() const ;

            const mpd_Song*
            get_song_at(int id) const ;

            void
            del_song_id(int id) ;

            void
            move_by_pos(int pos1, int pos2) ;

            void
            fetch_mpd_data() ;

            void
            insert_into_sql() ; //FIXME move this into the shell

            void
            add_song(
                  const std::string& uri
            ) ;

            void
            queue_add_song(
                  const std::string& uri
            ) ;

            void
            queue_commit(
            ) ;

            void
            clear(
            ) ;

            bool
            play_pos(
                  unsigned 
            ) ;

            bool
            play_id(
                  unsigned 
            ) ;

            void
            next() ;

            void
            prev() ;

            void
            play() ;

            void
            stop() ;
    
            void
            pause() ;

            void
            seek( int ) ;

            void
            volume( int ) ;
    } ;
}

#endif // MPXPD_MPD_HH
