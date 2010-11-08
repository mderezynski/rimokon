#ifndef MPXPD_SQL_HH
#define MPXPD_SQL_HH

#include <config.h>

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
#include "mpxpd-basic-types.hh"
#include "mpx/mpx-services.hh"

typedef sqlite3_stmt* SQLite3Statement ;

namespace MPXPD
{
    typedef boost::variant<int64_t,double,std::string>  row_variant_t ;
    typedef std::map<std::string, row_variant_t>        sql_row_t ;
    typedef std::vector<sql_row_t>                      sql_rows_t ;
    typedef std::map<std::string, int64_t>              mpxid_id_cache_t ;

    class SQL
    : public ::MPX::Service::Base
    {
        private:

            std::string m_filename ;

            sqlite3* sql ;
        
            mpxid_id_cache_t artist_cache ;
            mpxid_id_cache_t albums_cache ;
            mpxid_id_cache_t track_artists_cache ;

            int64_t
            sql_statement_prepare(
                  SQLite3Statement&     /*statement*/
                , const std::string&    /*sqlstr*/
            ) ;

            void
            sql_assemble_row(
                  SQLite3Statement&     /*statement*/
                , sql_rows_t&           /*rows*/
            ) ;

        public:

            sqlite3*
            get_sql_db()
            {
                return sql ;
            }

            void
            open_db(
                  const std::string& hostname
            )
            {
                if( sql )
                {
                    sqlite3_close( sql ) ;
                    sql = 0 ;
                }

                m_filename = Glib::build_filename(

                                Glib::build_filename(
                                        g_get_user_data_dir()
                                      , PACKAGE
                                )

                                , std::string("mpxpd-music-cache-") 
                                + hostname 
                                + std::string(".sql")
                ) ;

                sqlite3_open( m_filename.c_str(), &sql ) ;

                init_sql_db() ;
            }

            SQL( sqlite3* sql_ )
            : ::MPX::Service::Base("mpx-service-sql")
            , sql( sql_ )
            {
            }

            SQL( const std::string& hostname )
            : ::MPX::Service::Base("mpx-service-sql")
            , sql( 0 )
            {
            }

            virtual ~SQL()
            {
                sqlite3_close( sql ) ;
            }

            void
            sql_exec( const std::string& sqlstr ) ; 

            void
            sql_get(
                  const std::string&    /*sqlstr*/
                , sql_rows_t&           /*rows*/
            ) ;

            void
            init_sql_db() ;

            int64_t
            sql_get_album_artist_id(
                  const std::string&    /*mpxid*/
                , const std::string&    /*name*/
            ) ;

            int64_t
            sql_get_album_id(
                  const std::string&    /*mpxid*/
                , const std::string&    /*name*/
                , int64_t               /*artist_j*/
                , const std::string&    /*date*/
            ) ;

            int64_t
            sql_get_track_artist_id(
                  const std::string&    /*mpxid*/
                , const std::string&    /*name*/
            ) ;
    } ;
}

#endif // MPXPD_SQL_HH
