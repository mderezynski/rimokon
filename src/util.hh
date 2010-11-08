#ifndef UTIL_HH
#define UTIL_HH

#include "config.h"

#include "mpx/mpxpd-basic-types.hh"
#include "mpx/mpxpd-sql.hh"
#include "mpx/com/view-tracks-mpxpd.hh"
#include "mpx/com/view-tracks-mpxpd-playlist.hh"


namespace MPXPD
{
    namespace Util
    {
            void
            insert_row_tracks(
                  sql_row_t&                          row
                , MPX::View::Tracks::DataModel_sp_t   model
            ) ;

            void
            insert_row_playlist(
                  sql_row_t&                            row
                , MPX::View::Playlist::DataModel_sp_t   model
                , int                                   pos
            ) ;

            void
            set_row_playlist(
                  sql_row_t&                                row
                , MPX::View::Playlist::DataModel_sp_t       model
                , int                                       pos
                , MPX::View::Playlist::Model_t::iterator    iter
            ) ;

    }
}

#endif //UTIL_HH
