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
#include <boost/variant.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <glibmm.h>
#include <glibmm/i18n.h>
#include <gtkmm.h>
#include <libglademm.h>
#include "mpxpd-album-artists.hh"
#include "mpxpd-albums.hh"
#include "mpxpd-titles.hh"

#include "mpxpd-mpd.hh"
#include "mpxpd-shell.hh"

#include "mpx/mpxpd-sql.hh"
#include "mpx/mpx-types.hh"
#include "mpx/mpx-covers.hh"

#include "mpx/util-graphics.hh"
#include "util.hh"

#include "plugin-manager-gui.hh"

#include "glib-marshalers.h"

using namespace MPX ;

namespace MPXPD
{
           Shell::Shell(
                  MPXPD::SQL& sql_
                , MPXPD::MPD& mpd_
                , Glib::RefPtr<Gnome::Glade::Xml> xml_
            )
            : Glib::ObjectBase("MPXPD_Shell")
            , ::MPX::Service::Base("mpx-service-shell")
            , Gnome::Glade::WidgetLoader<Gtk::Window>(xml_,"main")
            , sql( sql_ )
            , mpd( mpd_ )
            , xml( xml_ )
            , m_elapsed( 0 )
            {
               m_C_SIG_ID_track_new =
                    g_signal_new(
                          "track-new"
                        , G_OBJECT_CLASS_TYPE(G_OBJECT_GET_CLASS(gobj()))
                        , GSignalFlags(G_SIGNAL_RUN_FIRST)
                        , 0
                        , NULL
                        , NULL
                        , g_cclosure_marshal_VOID__VOID
                        , G_TYPE_NONE
                        , 0
                ) ;

                m_C_SIG_ID_track_cancelled =
                    g_signal_new(
                          "track-cancelled"
                        , G_OBJECT_CLASS_TYPE(G_OBJECT_GET_CLASS(gobj()))
                        , GSignalFlags(G_SIGNAL_RUN_FIRST)
                        , 0
                        , NULL
                        , NULL
                        , g_cclosure_marshal_VOID__VOID
                        , G_TYPE_NONE
                        , 0
                ) ;

                m_C_SIG_ID_track_out =
                    g_signal_new(
                          "track-out"
                        , G_OBJECT_CLASS_TYPE(G_OBJECT_GET_CLASS(gobj()))
                        , GSignalFlags(G_SIGNAL_RUN_FIRST)
                        , 0
                        , NULL
                        , NULL
                        , g_cclosure_marshal_VOID__VOID
                        , G_TYPE_NONE
                        , 0
                ) ;

                m_ui_man = Gtk::UIManager::create() ; 

                covers = MPX::services->get<MPX::Covers>("mpx-service-covers").get() ;
                throbber_pb = Gdk::PixbufAnimation::create_from_file(Glib::build_filename(DATA_DIR,Glib::build_filename("images","animated-throbber.gif"))) ;
                throbber = dynamic_cast<Gtk::Image*>(xml->get_widget("throbber-albums")) ;
                throbber->set( throbber_pb ) ;
                throbber_nb = dynamic_cast<Gtk::Notebook*>(xml->get_widget("throbber-notebook")) ;

                artists_view = new ArtistsView ;
                sw_artists = dynamic_cast<Gtk::ScrolledWindow*>(xml->get_widget("sw-artists")) ;
                sw_artists->add(*artists_view) ;

                m_conn_artists = artists_view->signal_new_artist_selected().connect(
                    sigc::mem_fun(        *this
                                        , &Shell::on_artist_changed
                )) ;

                albums_view = new AlbumsView ;
                sw_albums = dynamic_cast<Gtk::ScrolledWindow*>(xml->get_widget("sw-albums")); 
                sw_albums->add(*albums_view) ;    

                albums_view->signal_new_album_selected().connect(
                    sigc::mem_fun(        *this
                                        , &Shell::on_album_changed
                )) ;

                albums_view->signal_album_chosen().connect(
                        sigc::mem_fun(    *this
                                        , &Shell::on_browse_album_chosen
                )) ;

                titles_view = new TitlesView ;
                sw_titles = dynamic_cast<Gtk::ScrolledWindow*>(xml->get_widget("sw-titles")); 
                sw_titles->add(*titles_view) ;    
                titles_view->signal_song_chosen().connect(

                        sigc::mem_fun(    *this
                                        , &Shell::on_browse_song_chosen
                )) ;

            {
                MPX::View::Tracks::DataModel_sp_t m ( new MPX::View::Tracks::DataModel ) ;

                tracklist_model = MPX::View::Tracks::DataModelFilter_sp_t( new MPX::View::Tracks::DataModelFilter( m )) ;
                tracklist_view = new MPX::View::Tracks::Class ;

                View::Tracks::Column_sp_t c1 (new View::Tracks::Column(_("Track"))) ;
                c1->set_column(5) ;
                c1->set_alignment( Pango::ALIGN_RIGHT ) ;

                View::Tracks::Column_sp_t c2 (new View::Tracks::Column(_("Title"))) ;
                c2->set_column(0) ;

                View::Tracks::Column_sp_t c3 (new View::Tracks::Column(_("Time"))) ;
                c3->set_column(9) ;
                c3->set_alignment( Pango::ALIGN_CENTER ) ;

                View::Tracks::Column_sp_t c4 (new View::Tracks::Column(_("Artist"))) ;
                c4->set_column(1) ;

                View::Tracks::Column_sp_t c5 (new View::Tracks::Column(_("Album"))) ;
                c5->set_column(2) ;

                tracklist_view->append_column(c1) ;
                tracklist_view->append_column(c2) ;
                tracklist_view->append_column(c3) ;
                tracklist_view->append_column(c4) ;
                tracklist_view->append_column(c5) ;

                tracklist_view->column_set_fixed(
                      0
                    , true
                    , 60
                ) ;

                tracklist_view->column_set_fixed(
                      2
                    , true
                    , 60
                ) ;

                sw_tracklist = dynamic_cast<Gtk::ScrolledWindow*>(xml->get_widget("sw-search")) ;
                sw_tracklist->add(*tracklist_view) ;

                tracklist_view->signal_track_activated().connect(
                        sigc::mem_fun( *this, &Shell::on_search_song_chosen )
                ) ;
            }

            {
                MPX::View::Playlist::DataModel_sp_t m ( new MPX::View::Playlist::DataModel ) ;

                playlist_model = m ; 
                playlist_view = new MPX::View::Playlist::Class( m_ui_man ) ;

                View::Playlist::Column_sp_t c1 (new View::Playlist::Column(_("Track"))) ;
                c1->set_column(5) ;
                c1->set_alignment( Pango::ALIGN_RIGHT ) ;

                View::Playlist::Column_sp_t c2 (new View::Playlist::Column(_("Title"))) ;
                c2->set_column(0) ;

                View::Playlist::Column_sp_t c3 (new View::Playlist::Column(_("Time"))) ;
                c3->set_column(9) ;
                c3->set_alignment( Pango::ALIGN_CENTER ) ;

                View::Playlist::Column_sp_t c4 (new View::Playlist::Column(_("Artist"))) ;
                c4->set_column(1) ;

                View::Playlist::Column_sp_t c5 (new View::Playlist::Column(_("Album"))) ;
                c5->set_column(2) ;

                playlist_view->append_column(c1) ;
                playlist_view->append_column(c2) ;
                playlist_view->append_column(c3) ;
                playlist_view->append_column(c4) ;
                playlist_view->append_column(c5) ;

                playlist_view->column_set_fixed(
                      0
                    , true
                    , 60
                ) ;

                playlist_view->column_set_fixed(
                      2
                    , true
                    , 60
                ) ;

                sw_playlist = dynamic_cast<Gtk::ScrolledWindow*>(xml->get_widget("sw-playlist")) ;
                sw_playlist->add(*playlist_view) ;

                playlist_view->signal_track_activated().connect(
                        sigc::mem_fun( *this, &Shell::on_song_play )
                ) ;

                playlist_view->signal_clear().connect(
                        sigc::mem_fun( *this, &Shell::on_playlist_clear )
                ) ;

                playlist_view->signal_left().connect(
                        sigc::mem_fun( *tracklist_view, &Gtk::Widget::grab_focus )
                ) ;

                mpd.give_model( m ) ;
            }

                info_label = dynamic_cast<Gtk::Label*>(xml->get_widget("info-label")) ;

                search_entry = dynamic_cast<Gtk::Entry*>(xml->get_widget("search-entry")) ;
                search_entry->signal_changed().connect(                
                      sigc::mem_fun(*this,&Shell::on_search_entry_changed
                )) ;
                search_entry->signal_icon_press().connect( sigc::mem_fun( *this, &Shell::on_search_entry_icon_clicked )) ;

                tracklist_view->set_model( tracklist_model ) ;
                tracklist_view->signal_find_propagate().connect(
                    sigc::mem_fun(
                              *this
                            , &Shell::on_tracklist_find_propagate
                )) ;

                playlist_view->set_model( playlist_model ) ;

                playlist_view->signal_song_removed().connect(

                        sigc::mem_fun(    *this
                                        , &Shell::on_delete_song
                )) ;

                playlist_view->signal_row_moved().connect(

                        sigc::mem_fun(    *this
                                        , &Shell::on_row_moved
                )) ;

                m_position = new KoboPosition ; 
                m_position->signal_seek_event().connect( sigc::mem_fun( *this, &Shell::on_seek )) ;

                Gtk::HBox * hbox_vol_pos = dynamic_cast<Gtk::HBox*>(xml->get_widget("hbox-vol-pos")) ;
                hbox_vol_pos->pack_start( *m_position, true, true ) ;

                m_volume = new KoboVolume ; 
                m_volume->signal_set_volume().connect( sigc::mem_fun( *this, &Shell::on_set_volume )) ;

                hbox_vol_pos->pack_start( *m_volume, false, false ) ;

                m_volume->set_volume( mpd.get_volume() ) ;

                b_prev = dynamic_cast<Gtk::Button*>(xml->get_widget("b-prev")) ;
                b_prev->signal_clicked().connect( sigc::mem_fun( *this, &Shell::on_b_prev )) ;

                b_next = dynamic_cast<Gtk::Button*>(xml->get_widget("b-next")) ;
                b_next->signal_clicked().connect( sigc::mem_fun( *this, &Shell::on_b_next )) ;

                b_stop = dynamic_cast<Gtk::Button*>(xml->get_widget("b-stop")) ;
                b_stop->signal_clicked().connect( sigc::mem_fun( *this, &Shell::on_b_stop )) ;

                b_play = dynamic_cast<Gtk::Button*>(xml->get_widget("b-play")) ;
                b_play->signal_clicked().connect( sigc::mem_fun( *this, &Shell::on_b_play )) ;

                i_play = dynamic_cast<Gtk::Image*>(xml->get_widget("i-play")) ;

                i_cover = dynamic_cast<Gtk::Image*>(xml->get_widget("image-cover")) ;
                i_cover_large = dynamic_cast<Gtk::Image*>(xml->get_widget("image-cover-large")) ;

                xml->get_widget( "button-clear", bt_clear_playlist ) ;
                bt_clear_playlist->signal_clicked().connect(
                      sigc::mem_fun( *this, &Shell::on_playlist_clear
                )) ;
    
                m_actions = Gtk::ActionGroup::create() ;
        
                m_actions->add( Gtk::Action::create("MenuFile","_File" )) ;
                m_actions->add( Gtk::Action::create("MenuEdit","_Edit" )) ;
                m_actions->add( Gtk::Action::create("Quit", Gtk::Stock::QUIT),
                        sigc::ptr_fun(&Gtk::Main::quit)) ;
                m_actions->add( Gtk::Action::create("Preferences","Preferences"),
                        sigc::mem_fun(*this, &Shell::on_open_preferences)) ;
                m_actions->add( Gtk::Action::create("Plugins","Plugins"),
                        sigc::mem_fun(*this, &Shell::on_open_plugins)) ;
                m_actions->add( Gtk::Action::create("DownloadCovers","Download Covers"),
                        sigc::mem_fun(*this, &Shell::on_download_covers)) ;
       
                m_ui_man->insert_action_group( m_actions ) ;

                std::string ui_str =
                    "<ui>"
                    "  <menubar name='MenuBar'>"
                    "    <menu action='MenuFile'>"
                    "       <menuitem action='Quit'/>"
                    "    </menu>"
                    "    <menu action='MenuEdit'>"
                    "       <menuitem action='DownloadCovers'/>"
                    "       <separator/>"
                    "       <menuitem action='Plugins'/>"
                    "       <menuitem action='Preferences'/>"
                    "    </menu>"
                    "  </menubar>"
                    "</ui>" ;
    
                m_ui_man->add_ui_from_string(ui_str) ;

                Gtk::Alignment * a = dynamic_cast<Gtk::Alignment*>(xml->get_widget("a-menubar")) ;
                Gtk::Widget * w = m_ui_man->get_widget("/MenuBar") ;
                a->add(*w) ;
                a->show_all() ;

                add_accel_group(m_ui_man->get_accel_group()) ; 

                m_sig4 = mpd.signal_disconnected().connect(

                        sigc::mem_fun(    *this
                                        , &Shell::on_server_disconnected
                )) ;

                m_sig5 = mpd.signal_connected().connect(

                        sigc::mem_fun(    *this
                                        , &Shell::on_server_connected
                )) ;

                mpd.set_connection_data(
                      mcs->key_get<std::string>("mpd","hostname")
                    , mcs->key_get<int>("mpd","port")
                ) ;

                mpd.connect() ;

                m_old_state = mpd.get_state() ;
                on_server_state( mpd.get_state() ) ;

                resize(
                    mcs->key_get<int>("mpx","window-w"),
                    mcs->key_get<int>("mpx","window-h")
                    );

                move(
                    mcs->key_get<int>("mpx","window-x"),
                    mcs->key_get<int>("mpx","window-y")
                    );

                show_all() ;

                Gtk::Window::raise ();
            } 

            void
            Shell::on_library_updated()
            {
                mpd.fetch_mpd_data() ;
                mpd.insert_into_sql() ;
                reload_stores() ;
            }

            void
            Shell::reload_stores()
            {
                m_conn_artists.disconnect() ;
        
                tracklist_model->clear() ;
                playlist_model->clear() ;
                artists_view->clear() ;
                albums_view->clear() ;
                titles_view->clear() ;

                sql_rows_t v ;
                sql.sql_get((boost::format("SELECT max(id) AS id FROM artists")).str(), v ) ; 
                gint64 max_artist = boost::get<gint64>(v[0]["id"]) ;

                v.clear();
                sql.sql_get((boost::format("SELECT max(id) AS id FROM albums")).str(), v ) ; 
                gint64 max_albums = boost::get<gint64>(v[0]["id"]) ;

                tracklist_model->set_sizes( max_artist, max_albums ) ;

                sql_rows_t rows ;
                sql.sql_get("SELECT * FROM track_view;", rows ) ;
                for(sql_rows_t::iterator i = rows.begin(); i != rows.end(); ++i )
                {
                    MPXPD::Util::insert_row_tracks( *i, tracklist_model ) ;
                }
                tracklist_model->regen_mapping() ;

                rows.clear() ;
                sql.sql_get("SELECT name,id FROM artists ORDER BY name ASC;", rows ) ;
                for(sql_rows_t::iterator i = rows.begin(); i != rows.end(); ++i )
                {
                    sql_row_t& row = *i ;

                    int64_t id ;
                    std::string name ;

                    id = boost::get<int64_t>(row["id"]) ; 

                    if( row["name"].which() == 2 )
                        name = boost::get<std::string>(row["name"]) ;

                    artists_view->append_artist( name, id ) ;
                }

                rows.clear() ;
                mpd.get_queue( rows ) ;
    
                for(sql_rows_t::iterator i = rows.begin(); i != rows.end(); ++i)
                {
                    MPXPD::Util::insert_row_playlist( *i, playlist_model, boost::get<int64_t>((*i)["local_id"])) ;
                }

                m_conn_artists = artists_view->signal_new_artist_selected().connect(
                    sigc::mem_fun(*this, &Shell::on_artist_changed)
                ) ;
            }

            void
            Shell::on_download_covers(
            )
            {
                Gtk::Window & win = *(dynamic_cast<Gtk::Window*>(xml->get_widget("win-covers"))) ;
                Gtk::ProgressBar & bar = *(dynamic_cast<Gtk::ProgressBar*>(xml->get_widget("covers-progress"))) ;

                win.present() ;

                sql_rows_t rows ;

                sql.sql_get("SELECT DISTINCT albums.id AS id, albums.mpxid AS mpxid, artists.name AS artist, albums.name AS album FROM albums JOIN artists ON albums.artist_j = artists.id;", rows) ;

                for(sql_rows_t::iterator i = rows.begin(); i != rows.end() ; ++i)
                {
                    RequestQualifier req ; 

                    req.mbid = boost::get<std::string>((*i)["mpxid"]) ;
                    req.artist = boost::get<std::string>((*i)["artist"]) ;
                    req.album = boost::get<std::string>((*i)["album"]) ;
                    req.id = boost::get<gint64>((*i)["id"]) ;

                    if( !covers->exists( req )) 
                    {
                        covers->cache( req, true ) ;
                    }

                    bar.set_fraction( double(std::distance(rows.begin(), i)) / double(rows.size())) ;
                    while (gtk_events_pending()) gtk_main_iteration() ;
                }
               
                win.hide() ;

                reload_stores() ; 
            }

            void
            Shell::on_open_preferences(
            )
            {
                MPX::services->get<MPX::Preferences>("mpx-service-preferences")->present() ;
            }

            void
            Shell::on_open_plugins(
            )
            {
                MPX::services->get<MPX::PluginManagerGUI>("mpx-service-plugins-gui")->present() ;
            }

            void
            Shell::on_search_entry_icon_clicked(
                  Gtk::EntryIconPosition    pos
                , const GdkEventButton*
            )
            {
                search_entry->set_text("") ;                
            }

            void
            Shell::on_seek(
                  int pos
            )
            {
                mpd.seek( pos ) ;
            }

            void
            Shell::on_set_volume(
                  int volume
            )
            {
                mpd.volume( volume ) ;
            }

            void
            Shell::_update_track_info()
            {
                if( m_current_metadata.is_set )
                {
                    std::string info_text = (boost::format("<b>%s</b> <small>•</small> %s <small>•</small> %s") 
                        % Glib::Markup::escape_text(m_current_metadata.title)
                        % Glib::Markup::escape_text(m_current_metadata.artist)
                        % Glib::Markup::escape_text(m_current_metadata.album)
                                            ).str() ;

                    info_label->set_markup( info_text ) ;
                }
                else
                {
                    info_label->set_text("") ;
                }
            }

            void
            Shell::on_server_state(
                  int state
            )
            {
                _update_gui() ;

                switch( state )
                {
                    case MPD_PLAYER_PAUSE:
                    {  
                        i_play->set( Gtk::Stock::MEDIA_PLAY, Gtk::ICON_SIZE_BUTTON ) ;
                        b_stop->set_sensitive() ;

                        if( m_old_state == MPD_PLAYER_PLAY )
                        {
                            m_CPP_SIG_paused.emit( true ) ;
                        }

                        break ;
                    }

                    case MPD_PLAYER_PLAY:
                    {  
                        i_play->set( Gtk::Stock::MEDIA_PAUSE, Gtk::ICON_SIZE_BUTTON ) ;
                        b_play->set_sensitive() ;
                        b_stop->set_sensitive() ;

                        if( m_old_state == MPD_PLAYER_PAUSE )
                        {
                            m_CPP_SIG_paused.emit( false ) ;
                        }

                        break ;
                    }

                    case MPD_PLAYER_STOP:
                    {
                        i_play->set( Gtk::Stock::MEDIA_PLAY, Gtk::ICON_SIZE_BUTTON ) ;

                        b_stop->set_sensitive( false ) ;
                        b_next->set_sensitive( false ) ;
                        b_prev->set_sensitive( false ) ;
                        b_play->set_sensitive( playlist_model->size() ) ;

                        playlist_view->clear_active_track() ;
                        m_position->set_position( 0, 0 ) ;

                        m_current_metadata = MPXPD::Metadata() ;
                        m_current_metadata.is_set = false ;

                        g_signal_emit(
                              G_OBJECT(gobj())
                            , m_C_SIG_ID_track_cancelled
                            , 0
                        ) ;

                        break ;
                    }

                    default:
                        break;
                }

                m_old_state = mpd.get_state() ;
            }

            void
            Shell::on_b_prev()
            {
                mpd.prev() ;
            }

            void
            Shell::on_b_next()
            {
                mpd.next() ;
            }

            void
            Shell::on_b_play()
            {
                if( mpd.get_state() == MPD_PLAYER_STOP )
                {
                    boost::optional<std::size_t> cur_id = playlist_view->get_selected_track() ;

                    if( cur_id )
                        mpd.play_pos( cur_id.get() ) ;
                    else
                    {
                        if( playlist_model->size() )
                        {
                            mpd.play_pos( 0 ) ;
                        }
                    }
                }
                else
                if( mpd.get_state() == MPD_PLAYER_PAUSE )
                {
                    mpd.play() ;
                }
                else
                {
                    mpd.pause() ;
                }

                boost::optional<int64_t> row = playlist_view->get_active_row() ;

                if( row )
                {
                    if( row.get() == 0 )
                        b_prev->set_sensitive( false ) ;
                    else
                        b_prev->set_sensitive( true ) ;

                    if( row.get() == (playlist_model->size()-1))
                        b_next->set_sensitive( false ) ;
                    else
                        b_next->set_sensitive( true ) ;
                }
                else
                {
                        b_prev->set_sensitive( false ) ;
                        b_next->set_sensitive( false ) ;
                }
            }

            void
            Shell::on_b_stop() 
            {
                mpd.stop() ;
            }

            void
            Shell::on_tracklist_find_propagate(
                  const std::string&    text
            )
            {
                search_entry->set_text( (boost::format("title %% \"%s\"") % text).str() ) ;
            }

            void
            Shell::on_delete_song( int id )
            {
                mpd.del_song_id( id ) ;
            }

            void
            Shell::on_row_moved(
                  int id1
                , int id2
            )
            {
                mpd.move_by_pos( id1, id2 ) ;
            }

            void
            Shell::on_server_playlist_changed()
            {
                _update_metadata() ;
                _update_gui() ;
            }

            void
            Shell::_update_cover()
            {
                if( m_current_metadata.is_set )
                {
                    sql_rows_t rows ;

                    char * sql_string = sqlite3_mprintf("SELECT albums_mpxid AS mpxid FROM track_view WHERE uri = '%q';", m_current_metadata.file.c_str() ) ;
                    sql.sql_get( sql_string, rows ) ;
                    sqlite3_free( sql_string ) ;

                    if( !rows.empty() )
                    { 
                        const std::string& mpxid = boost::get<std::string>(rows[0]["mpxid"]) ;

                        Glib::RefPtr<Gdk::Pixbuf> cover ;
                        covers->fetch( mpxid, cover ) ;

                        if( cover )
                        {
                            {
                                Glib::RefPtr<Gdk::Pixbuf> scaled = cover->scale_simple( 70, 70, Gdk::INTERP_BILINEAR) ;
                                Cairo::RefPtr<Cairo::ImageSurface> surf = MPX::Util::cairo_image_surface_from_pixbuf( scaled ) ;
                                surf = MPX::Util::cairo_image_surface_round( surf, 4. ) ;
                                scaled = MPX::Util::cairo_image_surface_to_pixbuf( surf ) ;
                                i_cover->set( scaled ) ;
                            }

                            {
                                Glib::RefPtr<Gdk::Pixbuf> scaled = cover->scale_simple( 256, 256, Gdk::INTERP_BILINEAR) ;
                                Cairo::RefPtr<Cairo::ImageSurface> surf = MPX::Util::cairo_image_surface_from_pixbuf( scaled ) ;
                                surf = MPX::Util::cairo_image_surface_round( surf, 4. ) ;
                                scaled = MPX::Util::cairo_image_surface_to_pixbuf( surf ) ;
                                i_cover_large->set( scaled ) ;
                            }
                        }
                        else
                        {
                            covers->fetch( cover ) ;

                            {
                                Glib::RefPtr<Gdk::Pixbuf> scaled = cover->scale_simple( 70, 70, Gdk::INTERP_BILINEAR) ;
                                i_cover->set( scaled ) ;
                            }

                            {
                                Glib::RefPtr<Gdk::Pixbuf> scaled = cover->scale_simple( 256, 256, Gdk::INTERP_BILINEAR) ;
                                i_cover_large->set( scaled ) ;
                            }

                        }
                    }
                }
                else
                {
                    i_cover->clear() ;
                    i_cover_large->clear() ;
                }
            }

            void
            Shell::_update_playlist_view()
            {
                const mpd_Song* song = mpd.get_current_song() ;

                if( song ) 
                {
                    playlist_view->set_active_track( song->id ) ;
                }
                else
                {
                    playlist_view->clear_active_track() ;
                }
            }

            void
            Shell::_update_controls()
            {
                boost::optional<int64_t> row = playlist_view->get_active_row() ;

                if( row )
                {
                    if( row.get() == 0 )
                        b_prev->set_sensitive( false ) ;
                    else
                        b_prev->set_sensitive( true ) ;

                    if( row.get() == (playlist_model->size()-1))
                        b_next->set_sensitive( false ) ;
                    else
                        b_next->set_sensitive( true ) ;
                }

                b_play->set_sensitive( playlist_model->size() ) ;
            }

            void
            Shell::_update_metadata()
            {
                m_current_metadata = MPXPD::Metadata() ;
                m_current_metadata.is_set = false ;

                const mpd_Song* song = mpd.get_current_song() ;

                if( song )
                {
                    sql_rows_t rows ;
                    char * sql_string = sqlite3_mprintf("SELECT * FROM track_view WHERE uri = '%q';", song->file ) ;
                    sql.sql_get( sql_string, rows ) ;
                    sqlite3_free( sql_string ) ;

                    if( !rows.empty() )
                    {
                        m_current_metadata.artist = boost::get<std::string>(rows[0]["track_artists_name"]) ;
                        m_current_metadata.album  = boost::get<std::string>(rows[0]["albums_name"]) ;
                        m_current_metadata.title  = boost::get<std::string>(rows[0]["tracks_name"]) ;
                        m_current_metadata.track_mpxid = boost::get<std::string>(rows[0]["tracks_mpxid"]) ;
                        m_current_metadata.time = boost::get<int64_t>(rows[0]["tracks_duration"]) ;
                        m_current_metadata.tracknr = boost::get<int64_t>(rows[0]["tracks_nr"]) ;
                        m_current_metadata.file = song->file ;
                        m_current_metadata.id = song->id ;
                        m_current_metadata.is_set = true ;
                    }
                }
            }

            void
            Shell::_update_gui()
            {
                _update_track_info() ;
                _update_playlist_view() ;
                _update_controls() ;
                _update_cover() ;
            }

            void
            Shell::on_server_next_song()
            {
                if( m_old_state == MPD_PLAYER_PLAY )
                {
                    g_signal_emit(
                        G_OBJECT(gobj())
                      , m_C_SIG_ID_track_out
                      , 0
                    ) ;
                }

                _update_metadata() ;

                g_signal_emit(
                    G_OBJECT(gobj())
                  , m_C_SIG_ID_track_new
                  , 0
                ) ;

                _update_gui() ;
            }

            void
            Shell::on_server_song_metrics( int pos, int dur )
            {
                m_elapsed = pos ;
                m_position->set_position( dur, pos ) ;
            }

            void
            Shell::on_server_set_volume( int volume ) 
            {
	            if( volume >= 0 )
		        {
                	m_volume->set_volume( volume ) ; 
		        }
            }

            void
            Shell::on_server_connected()
            {
                boost::shared_ptr< ::MPXPD::SQL> p2 = services->get< ::MPXPD::SQL>("mpx-service-sql") ;
                p2->open_db( mcs->key_get<std::string>("mpd","hostname")) ;

                m_sig1 = mpd.signal_updated().connect(
                          sigc::mem_fun( *this, &Shell::on_library_updated )) ;

 
                m_sig2 = mpd.signal_position_duration().connect(

                        sigc::mem_fun(    *this
                                        , &Shell::on_server_song_metrics
                )) ;

                m_sig3 = mpd.signal_volume().connect(

                        sigc::mem_fun(    *this
                                        , &Shell::on_server_set_volume
                )) ;

                m_sig6 = mpd.signal_next_song().connect(
                
                        sigc::mem_fun(    *this
                                        , &Shell::on_server_next_song
                )) ;

                m_sig7 = mpd.signal_state().connect(

                        sigc::mem_fun(    *this
                                        , &Shell::on_server_state
                )) ;

                m_sig8 = mpd.signal_playlist_changed().connect(

                        sigc::mem_fun(    *this
                                        , &Shell::on_server_playlist_changed
                )) ;

                reload_stores() ;

                mpd.start_monitoring() ;
            }

            void
            Shell::on_server_disconnected( int err )
            {
                m_conn_artists.disconnect() ;

                m_sig1.disconnect() ;
                m_sig2.disconnect() ;
                m_sig3.disconnect() ;
                //m_sig4.disconnect() ;
                //m_sig5.disconnect() ;
                m_sig6.disconnect() ;
                m_sig7.disconnect() ;
                m_sig8.disconnect() ;

                tracklist_model->clear() ;
                playlist_model->clear() ;
                artists_view->clear() ;
                albums_view->clear() ;
                titles_view->clear() ;
            }

            void
            Shell::on_search_entry_changed()
            {
                tracklist_model->set_filter( search_entry->get_text() ) ;
            }

            void
            Shell::on_artist_changed( int64_t artist_id )
            {
                artists_view->set_sensitive( false ) ;

                throbber_nb->set_current_page(1);
                albums_view->begin_add() ;
                albums_view->clear() ;
                titles_view->clear() ;

                sql_rows_t  rows ;

                if( artist_id != -1 )
                    sql.sql_get((boost::format("SELECT name,id,mpxid,date FROM albums WHERE artist_j = '%lld' ORDER BY date,name ASC;") % artist_id).str(), rows ) ;
                else
                    sql.sql_get("SELECT albums.name as name, albums.id as id, albums.mpxid as mpxid, albums.date as date, artists.name as artist_name FROM albums JOIN artists ON artists.id = albums.artist_j ORDER BY artist_name, date,name;", rows ) ;

                for(sql_rows_t::iterator i = rows.begin(); i != rows.end(); ++i )
                {
                    sql_row_t& row = *i ;

                    int64_t id ;
                    std::string name, mpxid, date ;

                    id = boost::get<int64_t>(row["id"]) ; 

                    if( row["name"].which() == 2 )
                        name = boost::get<std::string>(row["name"]) ;

                    if( row["mpxid"].which() == 2 )
                        mpxid = boost::get<std::string>(row["mpxid"]) ;

                    if( row["date"].which() == 2 )
                        date = boost::get<std::string>(row["date"]).substr(0,4) ;

                    Glib::RefPtr<Gdk::Pixbuf> cover ;
                    covers->fetch( mpxid, cover ) ;

                    if( cover )
                    {
                        Glib::RefPtr<Gdk::Pixbuf> scaled = cover->scale_simple( 64, 64, Gdk::INTERP_BILINEAR) ;
                        Cairo::RefPtr<Cairo::ImageSurface> surf = MPX::Util::cairo_image_surface_from_pixbuf( scaled ) ;
                        surf = MPX::Util::cairo_image_surface_round( surf, 4. ) ;
                        scaled = MPX::Util::cairo_image_surface_to_pixbuf( surf ) ;
                        albums_view->append_album( name, id, mpxid, scaled, date ) ;
                    }
                    else
                    {
                        covers->fetch( cover ) ;
                        Glib::RefPtr<Gdk::Pixbuf> scaled = cover->scale_simple( 64, 64, Gdk::INTERP_BILINEAR) ;
                        albums_view->append_album( name, id, mpxid, scaled, date ) ; 
                    }

                    while (gtk_events_pending()) gtk_main_iteration() ;
                }

                albums_view->end_add() ;
                albums_view->autosize() ;
                albums_view->scroll_to_row( Gtk::TreePath("0") ) ;

                throbber_nb->set_current_page(0);

                if( albums_view->get_size() == 1 )
                {
                    albums_view->select_first() ;
                }

                artists_view->set_sensitive( true ) ;
            }

            void
            Shell::on_album_changed( int64_t album_id )
            {
                titles_view->clear() ;

                sql_rows_t rows ;

                sql.sql_get((boost::format("SELECT name,id,uri,duration,nr FROM tracks WHERE album_j = '%lld' ORDER BY nr ASC;") % album_id).str(), rows ) ;

                for(sql_rows_t::iterator i = rows.begin(); i != rows.end(); ++i )
                {
                    sql_row_t& row = *i ;

                    int64_t id, nr, duration ;
                    std::string name, uri ;

                    id = boost::get<int64_t>(row["id"]) ; 
                    nr = boost::get<int64_t>(row["nr"]) ;
                    duration = boost::get<int64_t>(row["duration"]) ;

                    if( row["name"].which() == 2 )
                        name = boost::get<std::string>(row["name"]) ;

                    if( row["uri"].which() == 2 )
                        uri = boost::get<std::string>(row["uri"]) ;

                    titles_view->append_title( name, id, uri, nr, duration ) ;
                }


                titles_view->autosize() ;
                titles_view->scroll_to_row( Gtk::TreePath("0")) ;
                titles_view->set_search_column( 1 ) ;
                titles_view->set_enable_search() ;
            }

            void
            Shell::on_song_play(
                  MPX::Track_sp     track
                , bool              ctrl
                , unsigned          row
            )
            {
                mpd.play_pos( row ) ;
            }

            void
            Shell::on_search_song_chosen(
                  MPX::Track_sp     track
                , bool              ctrl
                , unsigned          row
            )
            {
                MPX::Track& t = *(track.get()) ;
                std::string uri = boost::get<std::string>(t[ATTRIBUTE_LOCATION].get()) ;
                mpd.add_song( uri ) ;
                playlist_view->grab_focus_select_last_row() ;
            }

            void
            Shell::on_browse_song_chosen(
                  const std::string&    uri
            )
            {
                mpd.add_song( uri ) ;
                while (gtk_events_pending()) gtk_main_iteration() ;
                playlist_view->grab_focus_select_last_row() ;

            }

            void
            Shell::on_browse_album_chosen(
                  int64_t               id 
            )
            {
                sql_rows_t rows ;

                sql.sql_get((boost::format("SELECT uri FROM track_view WHERE albums_id = '%lld';") % id).str(), rows  ) ;

                for(sql_rows_t::iterator i = rows.begin(); i != rows.end(); ++i )
                {
                    mpd.queue_add_song( boost::get<std::string>((*i)["uri"]) );
                }

                mpd.queue_commit() ;
            }

            void
            Shell::on_playlist_clear()
            {
                mpd.clear() ;
            }
}
