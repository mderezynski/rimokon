//  MPX
//  Copyright (C) 2003-2011 MPX Development
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

#include "config.h"

#include <utility>
#include <iostream>
#include <sstream>

#include <glibmm.h>
#include <glib/gi18n.h>
#include <gtk/gtkstock.h>
#include <gtkmm.h>
#include <libglademm.h>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include "mpx/mpx-main.hh"
#include "mpx/widgets/widgetloader.hh"

#include "mpx/mpx-i-mpd.hh"
#include "mpx/i-youki-preferences.hh"

#include "mpx/mpxpd-sql.hh"

#include "prefs-mpd.hh"

using namespace Glib;
using namespace Gtk;

namespace MPX
{
    PrefsMPD*
        PrefsMPD::create(gint64 id)
    {
        return new PrefsMPD (Gnome::Glade::Xml::create (build_filename(DATA_DIR, "glade" G_DIR_SEPARATOR_S "cppmod-prefs-mpd.glade")), id );
    }

    PrefsMPD::~PrefsMPD ()
    {
    }

    PrefsMPD::PrefsMPD(
          const Glib::RefPtr<Gnome::Glade::Xml>&    xml
        , gint64                                    id
    )
        : Gnome::Glade::WidgetLoader<Gtk::VBox>(xml, "cppmod-prefs-mpd")
        , PluginHolderBase()
    {
        show() ;

        m_Name = "PreferencesModule MPD" ;
        m_Description = "This plugin provides MPD preferences" ;
        m_Authors = "M. Derezynski" ;
        m_Copyright = "(C) 2011 MPX Project" ;
        m_IAge = 0 ;
        m_Website = "http://redmine.sivashs.org/projects/mpx" ;
        m_Active = false ;
        m_HasGUI = false ;
        m_CanActivate = false ;
        m_Hidden = true ;
        m_Id = id ;

        boost::shared_ptr<IPreferences> p = services->get<IPreferences>("mpx-service-preferences") ;
        boost::shared_ptr< ::MPXPD::IMPD> p2 = services->get< ::MPXPD::IMPD>("mpx-service-mpd") ;

        p2->signal_updating().connect(
              sigc::mem_fun( *this, &PrefsMPD::on_library_updating )) ;

        p2->signal_updated().connect(
              sigc::mem_fun( *this, &PrefsMPD::on_library_updated )) ;

        p->add_page(
              this
            , _("MPD Server")
        ) ;

        m_Xml->get_widget( "rescan-at-startup", m_Library_RescanAtStartup ) ;
        m_Xml->get_widget( "mpd-server-hostname", m_Library_MPD_Hostname ) ;
        m_Xml->get_widget( "mpd-server-port", m_Library_MPD_Port ) ;
        m_Xml->get_widget( "mpd-update-db", m_Library_MPD_Update_DB ) ;
        m_Xml->get_widget( "mpd-reconnect", m_Library_MPD_Reconnect ) ;

        p2->signal_disconnected().connect(
              sigc::mem_fun( *this, &PrefsMPD::on_server_disconnected )) ;

        p2->signal_connected().connect(
              sigc::mem_fun( *this, &PrefsMPD::on_server_connected )) ;

        mcs_bind->bind_entry(
            *m_Library_MPD_Hostname
            , "mpd"
            , "hostname"
            );

        mcs_bind->bind_spin_button(
            *m_Library_MPD_Port
            , "mpd"
            , "port"
            );

        mcs_bind->bind_toggle_button(
            *m_Library_RescanAtStartup
            , "library"
            , "rescan-at-startup"
            );

        m_Library_MPD_Update_DB->signal_clicked().connect(
              sigc::mem_fun( *this, &PrefsMPD::on_update_clicked )) ;

        m_Library_MPD_Hostname->signal_changed().connect(
              sigc::mem_fun( *this, &PrefsMPD::on_hostname_port_changed )) ;

        m_Library_MPD_Hostname->signal_changed().connect(
              sigc::mem_fun( *this, &PrefsMPD::on_hostname_port_changed )) ;

        m_Library_MPD_Reconnect->signal_clicked().connect(
              sigc::mem_fun( *this, &PrefsMPD::on_reconnect_clicked )) ;

        if( mcs->key_get<bool>("library", "rescan-at-startup"))
        {
            on_update_clicked() ;
        }
    }

    void
    PrefsMPD::on_hostname_port_changed()
    {
        boost::shared_ptr< ::MPXPD::IMPD> p2 = services->get< ::MPXPD::IMPD>("mpx-service-mpd") ;
        p2->disconnect() ;

        m_hostname = m_Library_MPD_Hostname->get_text() ;
        m_port = m_Library_MPD_Port->get_value_as_int() ;
    }

    void
    PrefsMPD::on_reconnect_clicked()
    {
        boost::shared_ptr< ::MPXPD::IMPD> p2 = services->get< ::MPXPD::IMPD>("mpx-service-mpd") ;

        p2->disconnect() ;

        p2->set_connection_data(
              m_hostname
            , m_port
        ) ;

        p2->connect() ;
    }

    void
    PrefsMPD::on_server_connected()
    {
        m_Library_MPD_Update_DB->set_sensitive( true ) ;
    }


    void
    PrefsMPD::on_server_disconnected(int)
    {
        m_Library_MPD_Update_DB->set_sensitive( false ) ;
    }

    void
    PrefsMPD::on_update_clicked()
    {
        boost::shared_ptr< ::MPXPD::IMPD> p = services->get< ::MPXPD::IMPD>("mpx-service-mpd") ;
        p->update() ;
    }

    void
    PrefsMPD::on_library_updating()
    {
        m_Library_MPD_Update_DB->set_label(_("Updating...")) ;
        m_Library_MPD_Update_DB->set_sensitive( false ) ;
        m_Library_MPD_Reconnect->set_sensitive( false ) ;
        m_Library_MPD_Hostname->set_sensitive( false ) ;
        m_Library_MPD_Port->set_sensitive( false ) ;
    } 

    void
    PrefsMPD::on_library_updated()
    {
        m_Library_MPD_Update_DB->set_label(_("Update MPD Database Manually")) ;
        m_Library_MPD_Update_DB->set_sensitive( true ) ;
        m_Library_MPD_Reconnect->set_sensitive( true ) ;
        m_Library_MPD_Hostname->set_sensitive( true ) ;
        m_Library_MPD_Port->set_sensitive( true ) ;
    }
}  // namespace MPX
