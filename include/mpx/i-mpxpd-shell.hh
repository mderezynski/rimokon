#ifndef I_MPXPD_SHELL_HH
#define I_MPXPD_SHELL_HH

#include <gtkmm.h>
#include "mpx/mpxpd-metadata.hh"

namespace MPXPD
{
    class IShell
    {
        public:

            virtual const MPXPD::Metadata&
            get_metadata() = 0 ;

            virtual void
            on_b_prev() = 0 ;

            virtual void
            on_b_next() = 0 ;

            virtual void
            on_b_play() = 0 ;

            virtual void
            on_b_stop() = 0 ;

            virtual Gtk::Widget*
            get_widget() = 0 ;
    } ;
}

#endif // I_MPXPD_SHELL_HH
