
#include "mpxpd-mpd.hh"
#include "mpx/mpxpd-sql.hh"

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
#include <glibmm.h>
#include <gtkmm.h>
#include <libglademm.h>

extern "C" {

#include <libmpd/libmpd.h>
#include <libmpd/libmpd-database.h>
#include <libmpd/libmpd-internal.h>
#include <libmpd/libmpd-player.h>
#include <libmpd/libmpd-playlist.h>
#include <libmpd/libmpd-status.h>
}

namespace MPXPD
{
    void
    MPD::next()
    {
        mpd_player_next( obj ) ;
    }

    void
    MPD::prev()
    {
        mpd_player_prev( obj ) ;
    }

    void
    MPD::play()
    {
        mpd_player_play( obj ) ;
    }

    void
    MPD::stop()
    {
        mpd_player_stop( obj ) ;
    }

    void
    MPD::pause()
    {
        mpd_player_pause( obj ) ;
    }

    void
    MPD::seek( int pos )
    {
        mpd_player_seek( obj, pos ) ;
    }

    void
    MPD::volume( int volume )
    {
        mpd_status_set_volume( obj, volume ) ;
    }

    void
    MPD::add_song(
          const std::string& uri
    )
    {
        mpd_playlist_add( obj, uri.c_str() ) ;
    }

    void
    MPD::queue_add_song(
          const std::string& uri
    )
    {
        mpd_playlist_queue_add( obj, uri.c_str() ) ;
    }

    void
    MPD::queue_commit(
    )
    {
        mpd_playlist_queue_commit( obj ) ;
    }

    void
    MPD::clear(
    )
    {
        int err = mpd_playlist_clear( obj ) ;

        if( err )
        {
            g_message("Error: %d", err) ;
        }
    }

    bool
    MPD::play_pos(
            unsigned pos 
    )
    {
        return mpd_player_play_songnum( obj, pos ) ; 
    }

    bool
    MPD::play_id(
            unsigned id 
    )
    {
        return mpd_player_play_id( obj, id ) ; 
    }

    void
    MPD::get_queue(
          sql_rows_t& rows
    ) const
    {
        MpdData * db = 0 ;

        for( db = mpd_playlist_get_changes( obj, -1 ) ; db != NULL ; db = mpd_data_get_next(db)) 
        {
            if( db->type == MPD_DATA_TYPE_SONG )
            {
                char * sqlstr = sqlite3_mprintf("SELECT * FROM track_view WHERE uri = '%q';", db->song->file) ;

                sql.sql_get( sqlstr, rows ) ;

                if( rows.empty() )
                {
                    g_message("No rows for URI '%s'", db->song->file) ;
                }
                else
                {
                    rows[rows.size()-1]["local_id"] = int64_t(db->song->id) ;
                }

                sqlite3_free(sqlstr);
            }
        }
    }

    const mpd_Song*
    MPD::get_current_song() const
    {
        return mpd_playlist_get_current_song( obj ) ;
    }

    const mpd_Song*
    MPD::get_song_at( int pos ) const
    {
        return mpd_playlist_get_song_from_pos( obj, pos ) ;
    }

    void
    MPD::del_song_id( int id )
    {
        mpd_playlist_delete_id( obj, id ) ;
    }

    void
    MPD::move_by_pos(int pos1, int pos2)
    {
        mpd_playlist_move_pos( obj, pos1, pos2 ) ;
    }

    void
    MPD::for_each_song(gpointer data, gpointer user_data)
    {
            MPXPD::MPD& mpd = *(reinterpret_cast<MPXPD::MPD*>(user_data)) ;

            const mpd_Song * song = (mpd_Song*)(data) ; 

            const char * mb_album_id = song->mb_album_id ; 
            const char * mb_artist_id = song->mb_albumartist_id ; 
            const char * mb_track_id = song->mb_track_id ; 
            const char * mb_track_artist_id = song->mb_artist_id ; 

            const char * album = song->album ; 
            const char * artist = song->albumartist ; 
            const char * title = song->title ; 
            const char * track_artist = song->artist ; 

            const char * track = song->track ; 

            const char * date = song->date ;

            const char * genre = song->genre ;

            unsigned duration = song->time ; 

            time_t last_modified = 0 ; 

            if ((album && artist && title ))
            {
                // TRACK ARTIST

                std::string mpx_internal_track_artist_id ;

                if( track_artist && mb_track_artist_id )
                {
                    mpx_internal_track_artist_id = mb_track_artist_id ;
                }
                else
                if( track_artist )
                {
                    mpx_internal_track_artist_id = std::string("MPX_LOCAL_ID--")+std::string(track_artist);
                }

                // TRACK
                std::string mpx_internal_track_id ;

                if( title && mb_track_id )
                {
                    mpx_internal_track_id = mb_track_id ;            
                }
                else
                if( title )
                {
                    mpx_internal_track_id = std::string("MPX_LOCAL_ID--")+std::string(title) ;
                }

                Track trk ; 
                trk.MPXID = mpx_internal_track_id ;
                trk.URI = song->file ; 
                trk.Title = title ? title : "" ;
                trk.Genre = genre ? genre : "" ;
                trk.Duration = duration ;
                trk.MTime = last_modified ;
                trk.Artist.MPXID = mpx_internal_track_artist_id ;
                trk.Artist.Title = track_artist ? track_artist : "" ;

                if( track )
                {
                    typedef std::vector<std::string> split_vec_t ;
                    split_vec_t vec ;
                    std::string str1 = track ;
                    boost::algorithm::split( vec, str1, boost::algorithm::is_any_of("/")) ; 
                    trk.Nr = atoi(vec[0].c_str()) ; 
                }
                else
                {
                    trk.Nr = 0 ;
                }

                // ALBUM ARTIST

                std::string mpx_internal_artist_id ;

                if( artist && mb_artist_id )
                {
                    mpx_internal_artist_id = mb_artist_id ;
                }
                else
                if( artist )
                {
                    mpx_internal_artist_id = std::string("MPX_LOCAL_ID--")+std::string(artist);
                }

                Artist& art = mpd.artists[mpx_internal_artist_id] ;
                art.MPXID = mpx_internal_artist_id ;
                art.Title = artist ? artist : "" ;

                // ALBUM

                std::string mpx_internal_album_id ;

                if( album && mb_album_id )
                {
                    mpx_internal_album_id = mb_album_id ;
                }
                else
                if( album )
                {
                    mpx_internal_album_id = std::string("MPX_LOCAL_ID--")+std::string(album) ;
                }

                Album& alb = art.Albums[mpx_internal_album_id] ; 
                alb.MPXID = mpx_internal_album_id ;
                alb.Title = album ? album : "" ;
                alb.Date = date ? date : "" ;
                alb.Tracks.push_back(trk) ;
            }
    }
  
    void
    MPD::fetch_mpd_data()
    {
            GList * pl = mpd_database_get_complete(obj) ; 
            g_list_foreach( pl, GFunc(for_each_song), this);
    }

    void
    MPD::insert_into_sql()
    {
        Gtk::Window * win = dynamic_cast<Gtk::Window*>(xml->get_widget("progress"));
        Gtk::ProgressBar * bar = dynamic_cast<Gtk::ProgressBar*>(xml->get_widget("progressbar")) ;

        win->present() ;

        while(gtk_events_pending())
            gtk_main_iteration() ;

        sql.sql_exec("DROP TABLE artists;") ;
        sql.sql_exec("DROP TABLE track_artists;") ;
        sql.sql_exec("DROP TABLE albums;") ;
        sql.sql_exec("DROP TABLE tracks;") ;
        sql.sql_exec("DROP VIEW track_view;") ;
        sql.init_sql_db() ;

        for(artists_t::iterator i = artists.begin(); i != artists.end(); ++i )
        {
            double a = std::distance(artists.begin(), i ) ;
            double b = artists.size() ;

            bar->set_fraction( a/b ) ;

            while(gtk_events_pending())
                gtk_main_iteration() ;

            sql.sql_exec("BEGIN;");

            int64_t id_artist = sql.sql_get_album_artist_id( i->first, i->second.Title ) ;

            const album_map_t& albs = i->second.Albums ;

            for(album_map_t::const_iterator a = albs.begin(); a != albs.end(); ++a )
            {
                const Album& alb = a->second ;

                int64_t id_album = sql.sql_get_album_id( alb.MPXID, alb.Title, id_artist, alb.Date ) ;    

                for(track_vec_t::const_iterator c = alb.Tracks.begin(); c != alb.Tracks.end(); ++c ) 
                {
                    const Track& trk = *c ; 
                    int64_t id_track_artist = sql.sql_get_track_artist_id( trk.Artist.MPXID, trk.Artist.Title ) ;
                    char * sqlstr = sqlite3_mprintf("INSERT INTO tracks (name,genre,mpxid,uri,nr,duration,mtime,album_j,track_artist_j) VALUES ('%q','%q','%q','%q',%lld,%lld,%lld,%lld,%lld);", trk.Title.c_str(), trk.Genre.c_str(), trk.MPXID.c_str(), trk.URI.c_str(), trk.Nr, trk.Duration, trk.MTime, id_album, id_track_artist); 
                    sql.sql_exec( sqlstr ) ;
                    sqlite3_free( sqlstr ) ;
                }
            }

            sql.sql_exec("COMMIT;");
        }
        win->hide() ;
    }
}
