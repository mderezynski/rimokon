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

#include "mpx/mpxpd-sql.hh"

namespace MPXPD
{
    int64_t
    SQL::sql_statement_prepare(
          SQLite3Statement&     statement
        , const std::string&    sqlstr
    )
    {
        char const * tail ;

        int64_t status = sqlite3_prepare_v2(  
              sql
            , sqlstr.c_str()
            , sqlstr.length()
            , &statement
            , &tail
        ) ;

        if( status == SQLITE_SCHEMA )
        {
            sqlite3_reset( statement ) ;

            status = sqlite3_prepare_v2(
                  sql
                , sqlstr.c_str()
                , sqlstr.length()
                , &statement
                , &tail
            ) ;
        }

        if( status )
        {
            g_warning( "SQL Error: '%s', SQL Statement: '%s'", sqlite3_errmsg( sql ), sqlstr.c_str() );
            sqlite3_reset( statement ) ;
        }
   
        return status ; 
    }

    void
    SQL::sql_exec( const std::string& sqlstr )
    {
        SQLite3Statement statement  = 0 ;
        int64_t          status     = 0 ;

        sql_statement_prepare( statement, sqlstr ) ;

        for( ; status != SQLITE_DONE ; )
        {
            status = sqlite3_step( statement ) ;

            if( status == SQLITE_BUSY )
              continue ;

            if( status == SQLITE_DONE )
              break ;

            if( status != SQLITE_OK )
            {
                sqlite3_finalize( statement );
                    throw std::runtime_error("SQL Error") ;
            }
        }
    }

    void
    SQL::sql_assemble_row(
          SQLite3Statement&     statement
        , sql_rows_t&           rows
    )
    {
        unsigned int columns = sqlite3_column_count( statement ) ;

        sql_row_t row ;

        for( unsigned int n = 0 ; n < columns; ++n )
        {
              const char* name = sqlite3_column_name( statement, n ) ;

              if( name )
              {
                    switch( sqlite3_column_type( statement, n ))
                    {
                        case SQLITE_NULL:
                        case SQLITE_BLOB:
                        {
                            //
                            break;
                        }

                        case SQLITE_INTEGER:
                        {
                            row.insert(std::make_pair( name, static_cast<int64_t>( sqlite3_column_int64( statement, n )))) ;
                            break;
                        }

                        case SQLITE_FLOAT:
                        {
                            row.insert(std::make_pair( name, static_cast<double>( sqlite3_column_double( statement, n )))) ;
                            break;
                        }

                        case SQLITE_TEXT:
                        {
                            char const* value = reinterpret_cast<char const*>( sqlite3_column_text( statement, n )) ;

                            if( value )
                            {
                                row.insert( std::make_pair( name, std::string( value ))) ;
                            }
                            else
                            {
                                row.insert( std::make_pair( name, std::string(  "" ))) ;

                            }

                            break;
                        }
                    } 
                } 
            }

            rows.push_back( row ) ;
    }

    void
    SQL::sql_get(
          const std::string&    sqlstr
        , sql_rows_t&           rows
    )
    {
        SQLite3Statement statement = 0 ;

        int64_t status = SQLITE_OK ;

        do{
            status = sql_statement_prepare( statement, sqlstr ) ;

            if( status != SQLITE_OK )
            {
                throw std::runtime_error("SQL Error") ;
            }

            do{
                status = sqlite3_step( statement ) ;

                switch( status )
                {
                    case SQLITE_BUSY:
                    {
                        continue ;
                    }

                    case SQLITE_ROW:
                    {
                        sql_assemble_row( statement, rows ) ;
                        break;
                    }
                }
            } while( status != SQLITE_DONE ) ;
        } while( status == SQLITE_SCHEMA ) ; 

        status = sqlite3_finalize( statement ) ;
    }

    void
    SQL::init_sql_db()
    {
        sql_exec("CREATE TABLE IF NOT EXISTS artists (id INTEGER PRIMARY KEY, name TEXT, mpxid TEXT);") ; 
        sql_exec("CREATE TABLE IF NOT EXISTS track_artists (id INTEGER PRIMARY KEY, name TEXT, mpxid TEXT);");
        sql_exec("CREATE TABLE IF NOT EXISTS albums (id INTEGER PRIMARY KEY, name TEXT, mpxid TEXT, date TEXT, artist_j INTEGER);") ;
        sql_exec("CREATE TABLE IF NOT EXISTS tracks (id INTEGER PRIMARY KEY, name TEXT, genre TEXT, mpxid TEXT, uri TEXT, nr INTEGER, duration INTEGER, mtime INTEGER, album_j INTEGER, track_artist_j INTEGER);") ;
        sql_exec("CREATE VIEW IF NOT EXISTS track_view AS SELECT tracks.mtime AS tracks_mtime, tracks.duration AS tracks_duration, tracks.uri AS uri, tracks.nr AS tracks_nr, artists.name AS artists_name, artists.mpxid AS artists_mpxid, track_artists.name AS track_artists_name, track_artists.mpxid AS track_artists_mpxid, tracks.id AS tracks_id, albums.id AS albums_id, albums.mpxid AS albums_mpxid, tracks.mpxid AS tracks_mpxid, albums.name AS albums_name, albums.date as albums_date, tracks.name AS tracks_name, tracks.genre AS tracks_genre FROM tracks JOIN albums ON tracks.album_j = albums_id JOIN track_artists ON tracks.track_artist_j = track_artists.id JOIN artists ON albums.artist_j = artists.id ORDER BY artists_name,albums_date,albums_name,tracks_nr;") ; 
    } 

    int64_t
    SQL::sql_get_album_artist_id(
          const std::string& mpxid
        , const std::string& name
    )
    {
        mpxid_id_cache_t::const_iterator i = artist_cache.find( mpxid ) ;

        if( i != artist_cache.end() )
        {
            return i->second ;
        }

        sql_rows_t rows ;    

        char * sqlstr = sqlite3_mprintf("SELECT id FROM artists WHERE mpxid ='%q'", mpxid.c_str() ) ;

        sql_get( sqlstr, rows ) ;

        sqlite3_free( sqlstr ) ;

        if( rows.empty() ) // artist doesn't exist yet
        {
            char * sqlstr = sqlite3_mprintf("INSERT INTO artists (name, mpxid) VALUES ('%q','%q');", name.c_str(), mpxid.c_str()) ;
            sql_exec( sqlstr ) ;
            sqlite3_free( sqlstr ) ;
            int64_t id = sqlite3_last_insert_rowid( sql ) ;
            artist_cache.insert( std::make_pair( mpxid, id )) ;
            return id ;
        }
        else
        {
            sql_row_t& row = rows[0] ;
            int64_t id = boost::get<int>(row["id"]) ;
            artist_cache.insert( std::make_pair( mpxid, id )) ;
            return id ;
        }

        return 0 ; // indicates error
    }

    int64_t
    SQL::sql_get_album_id(
          const std::string& mpxid
        , const std::string& name
        , int64_t            artist_j
        , const std::string& date
    )
    {
        mpxid_id_cache_t::const_iterator i = albums_cache.find( mpxid ) ;

        if( i != albums_cache.end() )
        {
            return i->second ;
        }

        sql_rows_t rows ;    

        char * sqlstr = sqlite3_mprintf("SELECT id FROM albums WHERE mpxid ='%q'", mpxid.c_str() ) ;

        sql_get( sqlstr, rows ) ;

        sqlite3_free( sqlstr ) ;

        if( rows.empty() ) // artist doesn't exist yet
        {
            char * sqlstr = sqlite3_mprintf("INSERT INTO albums (name, mpxid, date, artist_j) VALUES ('%q','%q','%q',%lld);", name.c_str(), mpxid.c_str(), date.c_str(), artist_j) ;
            sql_exec( sqlstr ) ;
            sqlite3_free( sqlstr ) ;
            int64_t id = sqlite3_last_insert_rowid( sql ) ;
            albums_cache.insert( std::make_pair( mpxid, id )) ;
            return id ;
        }
        else
        {
            sql_row_t& row = rows[0] ;
            int64_t id = boost::get<int>(row["id"]) ;
            albums_cache.insert( std::make_pair( mpxid, id )) ;
            return id ;
        }

        return 0 ; // indicates error
    }

    int64_t
    SQL::sql_get_track_artist_id(
          const std::string& mpxid
        , const std::string& name
    )
    {
        mpxid_id_cache_t::const_iterator i = track_artists_cache.find( mpxid ) ;

        if( i != track_artists_cache.end() )
        {
            return i->second ;
        }

        sql_rows_t rows ;    

        char * sqlstr = sqlite3_mprintf("SELECT id FROM track_artists WHERE mpxid ='%q'", mpxid.c_str() ) ;

        sql_get( sqlstr, rows ) ;

        sqlite3_free( sqlstr ) ;

        if( rows.empty() ) // artist doesn't exist yet
        {
            char * sqlstr = sqlite3_mprintf("INSERT INTO track_artists (name, mpxid) VALUES ('%q','%q');", name.c_str(), mpxid.c_str()) ;
            sql_exec( sqlstr ) ;
            sqlite3_free( sqlstr ) ;
            int64_t id = sqlite3_last_insert_rowid( sql ) ;
            track_artists_cache.insert( std::make_pair( mpxid, id )) ;
            return id ;
        }
        else
        {
            sql_row_t& row = rows[0] ;
            int64_t id = boost::get<int>(row["id"]) ;
            track_artists_cache.insert( std::make_pair( mpxid, id )) ;
            return id ;
        }

        return 0 ; // indicates error
    }
}
