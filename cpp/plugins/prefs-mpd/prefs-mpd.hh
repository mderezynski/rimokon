//  MPX
//  Copyright (C) 2005-2007 MPX development.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
//  --
//
//  The MPX project hereby grants permission for non GPL-compatible GStreamer
//  plugins to be used and distributed together with GStreamer and MPX. This
//  permission is above and beyond the permissions granted by the GPL license
//  MPX is covered by.

#ifndef MPX_PREFS_MPD_HH
#define MPX_PREFS_MPD_HH

#include "config.h"

#include "mpx/mpx-services.hh"
#include "mpx/widgets/widgetloader.hh"

#include <string>
#include <set>
#include <vector>
#include <glibmm.h>
#include <glibmm/i18n.h>
#include <gtkmm.h>

#include "mpx/plugin-types.hh"

namespace MPX
{
  class PrefsMPD
  : public Gnome::Glade::WidgetLoader<Gtk::VBox>
  , public PluginHolderBase
  {
    public:

        PrefsMPD(
              const Glib::RefPtr<Gnome::Glade::Xml>&
            , gint64
        ) ;

        static PrefsMPD*
        create(
              gint64
        ) ;

        virtual
        ~PrefsMPD(
        ) ;

        virtual bool activate(
        )
        {
            return true ;
        }

        virtual bool deactivate(
        )
        {
            return true ;
        }

        virtual Gtk::Widget* get_gui(
        )
        {
            return 0 ;
        }

    protected:

        void
        on_update_clicked() ;

        void
        on_hostname_port_changed() ;

        void
        on_reconnect_clicked() ;

        void
        on_library_updating() ;

        void
        on_library_updated() ;

        void
        on_server_connected() ;

        void
        on_server_disconnected(int) ;
    
    private:

        Gtk::CheckButton            * m_Library_RescanAtStartup ;
        Gtk::CheckButton            * m_Library_RescanInIntervals ;
        Gtk::SpinButton             * m_Library_RescanInterval ; 

        Gtk::HBox                   * m_Library_RescanIntervalBox ;

        Gtk::Entry                  * m_Library_MPD_Hostname ;
        Gtk::SpinButton             * m_Library_MPD_Port ;

        Gtk::Button                 * m_Library_MPD_Update_DB ;
        Gtk::Button                 * m_Library_MPD_Reconnect ;

        std::string                   m_hostname ;
        int                           m_port ;

  } ; // class PrefsMPD

extern "C" MPX::PluginHolderBase*
get_instance(gint64 id)
{
    return MPX::PrefsMPD::create( id ) ;
}

extern "C" bool
del_instance(MPX::PluginHolderBase* b)
{
    delete dynamic_cast<MPX::PrefsMPD*>(b) ;
    return true ;
}

} // namespace MPX

#endif // MPX_PREFS_MPD_HH
