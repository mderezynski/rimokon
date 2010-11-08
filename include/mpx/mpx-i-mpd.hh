#ifndef MPXPD_I_MPD_HH
#define MPXPD_I_MPD_HH

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
#include <glibmm.h>

namespace MPXPD
{
    class IMPD
    {
        public:

            IMPD()
            : m_updating( false )
            {}

        protected:

            bool
                m_updating ;

        public:

            typedef sigc::signal<void>
                        signal_updating_t ;

            typedef sigc::signal<void>
                        signal_updated_t ;

            typedef sigc::signal<void,int>
                        signal_disconnected_t ;

            typedef sigc::signal<void>
                        signal_connected_t ;


        protected:

            signal_updating_t
                signal_updating_s ;

            signal_updated_t
                signal_updated_s ;

            signal_connected_t
                signal_connected_s ;

            signal_disconnected_t
                signal_disconnected_s ;

        public:

            virtual signal_updating_t&
            signal_updating()
            {
                return signal_updating_s ;
            }

            virtual signal_updated_t&
            signal_updated()
            {
                return signal_updated_s ;
            }

            virtual signal_connected_t&
            signal_connected()
            {
                return signal_connected_s ;
            }

            virtual signal_disconnected_t&
            signal_disconnected()
            {
                return signal_disconnected_s ;
            }

            virtual void
            set_connection_data(
                  const std::string&
                , int
            ) = 0 ;

            virtual void
            disconnect() = 0 ;

            virtual void
            connect() = 0 ;

            virtual void
            update() = 0 ;

    } ;
}

#endif // MPXPD_I_MPD_HH
