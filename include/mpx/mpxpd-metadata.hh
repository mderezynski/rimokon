#ifndef MPXPD_METADATA_HH
#define MPXPD_METADATA_HH

#include <string>

namespace MPXPD
{
    struct Metadata
    {
        std::string     artist ;
        std::string     title ;
        std::string     album ;
        std::string     track_mpxid ;
        std::string     file ;
        int             time ;
        int             tracknr ;
        int             id ;

        bool            is_set ;

        Metadata()
        : is_set( false )
        {}
    } ;
}

#endif // MPXPD_METADATA_HH
