#ifndef MPXPD_BASIC_TYPES_HH
#define MPXPD_BASIC_TYPES_HH

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

namespace MPXPD
{
    struct TrackArtist
    {
        std::string Title ;
        std::string MPXID ;
    } ;

    struct Track
    {
        std::string Title ;
        std::string URI ;
        std::string MPXID ;
        std::string Genre ;
        int64_t     Nr ;
        int64_t     Duration ;
        int64_t     MTime ;
        TrackArtist Artist ;
    } ;
    typedef std::vector<Track>                          track_vec_t ;

    struct Album
    {
        std::string Title ;
        std::string MPXID ;
        std::string Date ;
        track_vec_t Tracks ;
    } ;

    inline bool operator<( const Album& a, const Album& b )
    {
        return a.MPXID < b.MPXID ;
    }

    typedef std::map<std::string, Album>                album_map_t ;

    struct Artist
    {
        std::string Title ;
        std::string MPXID ;
        album_map_t Albums ;
    } ;
    typedef std::map<std::string,  Artist>              artists_t ;
}

#endif // MPXPD_BASIC_TYPES_HH
