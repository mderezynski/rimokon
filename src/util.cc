
#include "config.h"
#include "util.hh"

namespace MPXPD
{
    namespace Util
    {
            void
            insert_row_tracks(
                  sql_row_t&                          row
                , MPX::View::Tracks::DataModel_sp_t   model
            )
            {
                    int64_t id ;

                    std::string album_artist ;
                    std::string album ;
                    std::string genre ;
                    int64_t     nr ;
                    int64_t     duration ;
                    int64_t     last_modified ;
                    std::string name ;
                    std::string track_artist ;
                    std::string uri ;
                    std::string mb_track_id ;
                    std::string mb_artist_id ;
                    std::string mb_album_id ;
                    std::string mb_album_artist_id ;
    
                    id              = boost::get<int64_t>(row["tracks_id"]) ; 
                    nr              = boost::get<int64_t>(row["tracks_nr"]) ;
                    duration        = boost::get<int64_t>(row["duration"]) ;
                    last_modified   = boost::get<int64_t>(row["duration"]) ;

                    if( row["tracks_name"].which() == 2 )
                        name = boost::get<std::string>(row["tracks_name"]) ;

                    if( row["tracks_genre"].which() == 2 )
                        genre = boost::get<std::string>(row["tracks_genre"]) ;

                    if( row["artists_name"].which() == 2 )
                        album_artist = boost::get<std::string>(row["artists_name"]) ;

                    if( row["artists_mpxid"].which() == 2 )
                        mb_album_artist_id = boost::get<std::string>(row["artists_mpxid"]) ;

                    if( row["albums_name"].which() == 2 )
                        album = boost::get<std::string>(row["albums_name"]) ;

                    if( row["track_artists_name"].which() == 2 )
                        track_artist = boost::get<std::string>(row["track_artists_name"]) ;

                    if( row["uri"].which() == 2 )
                        uri = boost::get<std::string>(row["uri"]) ;

                    if( row["tracks_mpxid"].which() == 2 )
                        mb_track_id = boost::get<std::string>(row["tracks_mpxid"]) ;

                    if( row["track_artists_mpxid"].which() == 2 )
                        mb_artist_id = boost::get<std::string>(row["track_artists_mpxid"]) ;

                    if( row["albums_mpxid"].which() == 2 )
                        mb_album_id = boost::get<std::string>(row["albums_mpxid"]) ;

                    MPX::Track * t = new MPX::Track ;
                    MPX::Track& tr = *t ;

                    tr[MPX::ATTRIBUTE_TIME] = duration ;
                    tr[MPX::ATTRIBUTE_MTIME] = last_modified ;
                    tr[MPX::ATTRIBUTE_LOCATION] = uri ;        
                    tr[MPX::ATTRIBUTE_TITLE] = name ;
                    tr[MPX::ATTRIBUTE_GENRE] = genre ;
                    tr[MPX::ATTRIBUTE_ARTIST] = track_artist ;
                    tr[MPX::ATTRIBUTE_ALBUM] = album ;
                    tr[MPX::ATTRIBUTE_ALBUM_ARTIST] = album_artist ;
                    tr[MPX::ATTRIBUTE_MB_TRACK_ID] = mb_track_id ;
                    tr[MPX::ATTRIBUTE_MB_ARTIST_ID] = mb_artist_id ;
                    tr[MPX::ATTRIBUTE_MB_ALBUM_ID] = mb_album_id ;
                    tr[MPX::ATTRIBUTE_MB_ALBUM_ARTIST_ID] = mb_album_artist_id ;
                    tr[MPX::ATTRIBUTE_MPX_TRACK_ID] = id ;
                    tr[MPX::ATTRIBUTE_MPX_ALBUM_ID] = boost::get<gint64>(row["albums_id"]);
                    tr[MPX::ATTRIBUTE_MPX_ARTIST_ID] = boost::get<gint64>(row["track_artists_id"]);
                    tr[MPX::ATTRIBUTE_MPX_ALBUM_ARTIST_ID] = boost::get<gint64>(row["artists_id"]);

                    MPX::Track_sp sp ( t ) ;

                    model->append_track(
                          row
                        , sp
                    ) ;
            }

            void
            insert_row_playlist(
                  sql_row_t&                            row
                , MPX::View::Playlist::DataModel_sp_t   model
                , int                                   pl_id
            )
            {
                    int64_t id ;

                    std::string album_artist ;
                    std::string album ;
                    std::string genre ;
                    int64_t     nr ;
                    int64_t     duration ;
                    int64_t     last_modified ;
                    std::string name ;
                    std::string track_artist ;
                    std::string uri ;
                    std::string mb_track_id ;
                    std::string mb_artist_id ;
                    std::string mb_album_id ;
                    std::string mb_album_artist_id ;
    
                    id              = boost::get<int64_t>(row["tracks_id"]) ; 
                    nr              = boost::get<int64_t>(row["tracks_nr"]) ;
                    duration        = boost::get<int64_t>(row["duration"]) ;
                    last_modified   = boost::get<int64_t>(row["duration"]) ;

                    if( row["tracks_name"].which() == 2 )
                        name = boost::get<std::string>(row["tracks_name"]) ;

                    if( row["tracks_genre"].which() == 2 )
                        genre = boost::get<std::string>(row["tracks_genre"]) ;

                    if( row["artists_name"].which() == 2 )
                        album_artist = boost::get<std::string>(row["artists_name"]) ;

                    if( row["artists_mpxid"].which() == 2 )
                        mb_album_artist_id = boost::get<std::string>(row["artists_mpxid"]) ;

                    if( row["albums_name"].which() == 2 )
                        album = boost::get<std::string>(row["albums_name"]) ;

                    if( row["track_artists_name"].which() == 2 )
                        track_artist = boost::get<std::string>(row["track_artists_name"]) ;

                    if( row["uri"].which() == 2 )
                        uri = boost::get<std::string>(row["uri"]) ;

                    if( row["tracks_mpxid"].which() == 2 )
                        mb_track_id = boost::get<std::string>(row["tracks_mpxid"]) ;

                    if( row["track_artists_mpxid"].which() == 2 )
                        mb_artist_id = boost::get<std::string>(row["track_artists_mpxid"]) ;

                    if( row["albums_mpxid"].which() == 2 )
                        mb_album_id = boost::get<std::string>(row["albums_mpxid"]) ;

                    MPX::Track * t = new MPX::Track ;
                    MPX::Track& tr = *t ;

                    tr[MPX::ATTRIBUTE_TIME] = duration ;
                    tr[MPX::ATTRIBUTE_MTIME] = last_modified ;
                    tr[MPX::ATTRIBUTE_LOCATION] = uri ;        
                    tr[MPX::ATTRIBUTE_TITLE] = name ;
                    tr[MPX::ATTRIBUTE_GENRE] = genre ;
                    tr[MPX::ATTRIBUTE_ARTIST] = track_artist ;
                    tr[MPX::ATTRIBUTE_ALBUM] = album ;
                    tr[MPX::ATTRIBUTE_ALBUM_ARTIST] = album_artist ;
                    tr[MPX::ATTRIBUTE_MB_TRACK_ID] = mb_track_id ;
                    tr[MPX::ATTRIBUTE_MB_ARTIST_ID] = mb_artist_id ;
                    tr[MPX::ATTRIBUTE_MB_ALBUM_ID] = mb_album_id ;
                    tr[MPX::ATTRIBUTE_MB_ALBUM_ARTIST_ID] = mb_album_artist_id ;
                    tr[MPX::ATTRIBUTE_MPX_TRACK_ID] = id ;
                    tr[MPX::ATTRIBUTE_MPX_ALBUM_ID] = boost::get<gint64>(row["albums_id"]);
                    tr[MPX::ATTRIBUTE_MPX_ARTIST_ID] = boost::get<gint64>(row["track_artists_id"]);
                    tr[MPX::ATTRIBUTE_MPX_ALBUM_ARTIST_ID] = boost::get<gint64>(row["artists_id"]);

                    MPX::Track_sp sp ( t ) ;

                    model->append_track(
                          row
                        , sp
                        , pl_id
                    ) ;
            }

            void
            set_row_playlist(
                  sql_row_t&                                row
                , MPX::View::Playlist::DataModel_sp_t       model
                , int                                       pl_id
                , MPX::View::Playlist::Model_t::iterator    iter
            )
            {
                    int64_t id ;

                    std::string album_artist ;
                    std::string album ;
                    int64_t     nr ;
                    int64_t     duration ;
                    int64_t     last_modified ;
                    std::string name ;
                    std::string track_artist ;
                    std::string uri ;
                    std::string mb_track_id ;
                    std::string mb_artist_id ;
                    std::string mb_album_id ;
                    std::string mb_album_artist_id ;
    
                    id              = boost::get<int64_t>(row["tracks_id"]) ; 
                    nr              = boost::get<int64_t>(row["tracks_nr"]) ;
                    duration        = boost::get<int64_t>(row["duration"]) ;
                    last_modified   = boost::get<int64_t>(row["duration"]) ;

                    if( row["tracks_name"].which() == 2 )
                        name = boost::get<std::string>(row["tracks_name"]) ;

                    if( row["artists_name"].which() == 2 )
                        album_artist = boost::get<std::string>(row["artists_name"]) ;

                    if( row["artists_mpxid"].which() == 2 )
                        mb_album_artist_id = boost::get<std::string>(row["artists_mpxid"]) ;

                    if( row["albums_name"].which() == 2 )
                        album = boost::get<std::string>(row["albums_name"]) ;

                    if( row["track_artists_name"].which() == 2 )
                        track_artist = boost::get<std::string>(row["track_artists_name"]) ;

                    if( row["uri"].which() == 2 )
                        uri = boost::get<std::string>(row["uri"]) ;

                    if( row["tracks_mpxid"].which() == 2 )
                        mb_track_id = boost::get<std::string>(row["tracks_mpxid"]) ;

                    if( row["track_artists_mpxid"].which() == 2 )
                        mb_artist_id = boost::get<std::string>(row["track_artists_mpxid"]) ;

                    if( row["albums_mpxid"].which() == 2 )
                        mb_album_id = boost::get<std::string>(row["albums_mpxid"]) ;

                    MPX::Track * t = new MPX::Track ;
                    MPX::Track& tr = *t ;

                    tr[MPX::ATTRIBUTE_TIME] = duration ;
                    tr[MPX::ATTRIBUTE_MTIME] = last_modified ;
                    tr[MPX::ATTRIBUTE_LOCATION] = uri ;        
                    tr[MPX::ATTRIBUTE_TITLE] = name ;
                    tr[MPX::ATTRIBUTE_ARTIST] = track_artist ;
                    tr[MPX::ATTRIBUTE_ALBUM] = album ;
                    tr[MPX::ATTRIBUTE_ALBUM_ARTIST] = album_artist ;
                    tr[MPX::ATTRIBUTE_MB_TRACK_ID] = mb_track_id ;
                    tr[MPX::ATTRIBUTE_MB_ARTIST_ID] = mb_artist_id ;
                    tr[MPX::ATTRIBUTE_MB_ALBUM_ID] = mb_album_id ;
                    tr[MPX::ATTRIBUTE_MB_ALBUM_ARTIST_ID] = mb_album_artist_id ;
                    tr[MPX::ATTRIBUTE_MPX_TRACK_ID] = id ;
                    tr[MPX::ATTRIBUTE_MPX_ALBUM_ID] = boost::get<gint64>(row["albums_id"]);
                    tr[MPX::ATTRIBUTE_MPX_ARTIST_ID] = boost::get<gint64>(row["track_artists_id"]);
                    tr[MPX::ATTRIBUTE_MPX_ALBUM_ARTIST_ID] = boost::get<gint64>(row["artists_id"]);

                    MPX::Track_sp sp ( t ) ;

                    model->set_track(
                          row
                        , sp
                        , pl_id
                        , iter
                    ) ;
            }

        }
}
