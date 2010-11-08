#include <glib.h>
#include <glib-object.h>
#include <glibmm.h>
#include <gtkmm.h>
#include <libglademm.h>

#include "mpx/mpx-services.hh"
#include "mpx/mpx-paths.hh"
#include "mpx/mpx-covers.hh"
#include "mpx/mpx-plugin.hh"
#include "mpx/mpx-i-mpd.hh"
#include "mpx/mpxpd-sql.hh"

#include "mpxpd-mpd.hh"
#include "mpxpd-shell.hh"

#include "youki-theme-engine.hh"
#include "preferences.hh"
#include "plugin-manager-gui.hh"

#include <mcs/mcs.h>

namespace MPX
{
	Mcs::Config         * mcs;
    Mcs::Bind        * mcs_bind;
	Service::Manager * services;
}

namespace MPXPD
{
    Glib::RefPtr<Gnome::Glade::Xml> xml ;
    SQL *                 sql = 0 ; 
    MPD *                 mpd = 0 ;
    Shell *               shl = 0 ;
}

int main(int argc, char* argv[])
{
    using namespace MPXPD ;

    Glib::thread_init(0) ; 
    Glib::init() ;
    g_type_init() ;

    Gtk::Main kit (argc, argv) ;

    MPX::mcs = new Mcs::Config( Glib::build_filename( MPX::get_app_config_dir(), "config.xml"), "K7", 0.01);
    MPX::mcs_bind = new Mcs::Bind( MPX::mcs ) ;
    MPX::mcs->load( Mcs::Config::VERSION_IGNORE ) ;

    {
        using namespace MPX ;

        mcs->domain_register ("main-window");
        mcs->key_register ("main-window", "width", 800);
        mcs->key_register ("main-window", "height", 600);
        mcs->key_register ("main-window", "pos_x", 0);
        mcs->key_register ("main-window", "pos_y", 0);
        mcs->key_register ("main-window", "paned1", 0);
        mcs->key_register ("main-window", "paned2", 0);

        mcs->domain_register ("mpx");
        mcs->key_register ("mpx", "display-notifications", true);
        mcs->key_register ("mpx", "volume", 50);
        mcs->key_register ("mpx", "follow-current-track", false);
        mcs->key_register ("mpx", "window-x", 20);
        mcs->key_register ("mpx", "window-y", 20);
        mcs->key_register ("mpx", "window-w", 400);
        mcs->key_register ("mpx", "window-h", 500);
        mcs->key_register ("mpx", "window-prefs-w", 700);
        mcs->key_register ("mpx", "window-prefs-h", 600);
        mcs->key_register ("mpx", "window-prefs-x", 120);
        mcs->key_register ("mpx", "window-prefs-y", 120);
        mcs->key_register ("mpx", "window-plugins-w", 500);
        mcs->key_register ("mpx", "window-plugins-h", 400);
        mcs->key_register ("mpx", "window-plugins-x", 140);
        mcs->key_register ("mpx", "window-plugins-y", 140);
        mcs->key_register ("mpx", "theme", std::string("popularfruit"));

        mcs->domain_register ("library");
        mcs->key_register ("library", "rescan-at-startup", false);

        mcs->domain_register ("pyplugs");
    
        mcs->domain_register("mpd") ;
        mcs->key_register("mpd","hostname",std::string("localhost")) ;
        mcs->key_register("mpd","port",int(6600)) ;

        mcs->domain_register("Preferences-CoverArtSources");
        mcs->key_register("Preferences-CoverArtSources", "Source0", int(0));
        mcs->key_register("Preferences-CoverArtSources", "Source1", int(1));
        mcs->key_register("Preferences-CoverArtSources", "Source2", int(2));
        mcs->key_register("Preferences-CoverArtSources", "Source3", int(3));
        mcs->key_register("Preferences-CoverArtSources", "SourceActive0", bool(true));
        mcs->key_register("Preferences-CoverArtSources", "SourceActive1", bool(true));
        mcs->key_register("Preferences-CoverArtSources", "SourceActive2", bool(true));
        mcs->key_register("Preferences-CoverArtSources", "SourceActive3", bool(true));
    }

    MPX::services = new MPX::Service::Manager ;
    MPX::services->add(boost::shared_ptr<MPX::YoukiThemeEngine>( new MPX::YoukiThemeEngine ));
    MPX::services->add(boost::shared_ptr<MPX::Preferences>( MPX::Preferences::create() ));
    MPX::services->add(boost::shared_ptr<MPX::Covers>(new MPX::Covers));

    const std::string path( Glib::build_filename(DATA_DIR, Glib::build_filename("glade","rimokon.glade")));
    xml = Gnome::Glade::Xml::create(path) ;

    std::string hostname = mcs->key_get<std::string>("mpd","hostname") ; 
    int port = mcs->key_get<int>("mpd","port") ;

    sql = new MPXPD::SQL( hostname ) ;
    MPX::services->add(boost::shared_ptr<MPXPD::SQL>( sql ));

    mpd = new MPXPD::MPD( *sql, xml, hostname, port ) ;
    MPX::services->add(boost::shared_ptr<MPXPD::MPD>( mpd ));

    shl = new MPXPD::Shell( *sql, *mpd, xml ) ;
    MPX::services->add(boost::shared_ptr<MPXPD::Shell>( shl ));

    MPX::services->add(boost::shared_ptr<MPX::PluginManager>( new MPX::PluginManager )) ;
    MPX::services->add(boost::shared_ptr<MPX::PluginManagerGUI>( MPX::PluginManagerGUI::create())) ;

    kit.run(*shl) ;

    delete sql ;
    delete shl ;
    delete mpd ; 

    delete MPX::mcs_bind ;
    delete MPX::mcs;
}
