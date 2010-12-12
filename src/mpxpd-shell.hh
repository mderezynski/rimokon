#ifndef MPXPD_SHELL_HH
#define MPXPD_SHELL_HH

#include <iostream>
#include <exception>
#include <stdexcept>
#include <string>
#include <set>
#include <map>
#include <vector>
#include <deque>
#include <sqlite3.h>
#include <glib.h>
#include <glib-object.h>
#include <boost/variant.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <glibmm.h>
#include <gtkmm.h>
#include <libglademm.h>
#include "mpxpd-album-artists.hh"
#include "mpxpd-albums.hh"
#include "mpxpd-titles.hh"
#include "mpx/com/view-tracks-mpxpd.hh"
#include "mpx/com/view-tracks-mpxpd-playlist.hh"
#include "mpx/mpx-types.hh"
#include "mpx/mpx-covers.hh"
#include "mpx/mpx-services.hh"

#include "mpxpd-mpd.hh"

#include "mpx/mpxpd-sql.hh"
#include "mpx/widgets/widgetloader.hh"

#include "kobo-position.hh"
#include "kobo-volume.hh"

#include "preferences.hh"

#include "mpx/i-mpxpd-shell.hh"

namespace MPXPD
{
    class Shell
        : public IShell
        , public ::MPX::Service::Base
        , public Gnome::Glade::WidgetLoader<Gtk::Window>
    {
        private:

            struct TrackId
            {
                int64_t     LocalId ;
                int         MPDId ;
            } ;

            std::deque<TrackId>   song_queue ;

            Gtk::ScrolledWindow * sw_artists ;
            Gtk::ScrolledWindow * sw_albums ;
            Gtk::ScrolledWindow * sw_titles ;
            Gtk::ScrolledWindow * sw_tracklist ;
            Gtk::ScrolledWindow * sw_playlist ;
            Gtk::Button         * bt_clear_playlist ;

            ArtistsView         * artists_view ; 
            AlbumsView          * albums_view ;
            TitlesView          * titles_view ;

            MPX::View::Tracks::Class * tracklist_view ;
            MPX::View::Tracks::DataModelFilter_sp_t tracklist_model ;

            MPX::View::Playlist::Class * playlist_view ;
            MPX::View::Playlist::DataModel_sp_t playlist_model ;

            Glib::RefPtr<Gdk::PixbufAnimation> throbber_pb ;

            Gtk::Entry          * search_entry ;
            Gtk::Label          * info_label ;

            Gtk::Image          * throbber ;
            Gtk::Notebook       * throbber_nb ;

            ::MPX::KoboPosition * m_position ;
            ::MPX::KoboVolume   * m_volume ;

            Gtk::Button         * b_prev
                              , * b_next
                              , * b_stop
                              , * b_play ;

            Gtk::Image          * i_play ;

            Gtk::Image          * i_cover ;
            Gtk::Image          * i_cover_large ;
            

            Glib::RefPtr<Gtk::ActionGroup> m_actions ;
            Glib::RefPtr<Gtk::UIManager>   m_ui_man ;

            MPXPD::Metadata       m_current_metadata ;

            guint                 m_C_SIG_ID_track_new ;
            guint                 m_C_SIG_ID_track_out ;
            guint                 m_C_SIG_ID_track_cancelled ;

            sigc::connection      m_conn_artists ;
            sigc::connection      m_sig1, m_sig2, m_sig3, m_sig4,
                                  m_sig5, m_sig6, m_sig7, m_sig8 ;

            const mpd_Song*       m_current_song ;

            // Internal stuff
            void
            reload_stores() ;

            // Action Callbacks
            void
            on_download_covers() ;
    
            void
            on_open_preferences() ;

            void
            on_open_plugins() ;



            void
            on_server_song_metrics(
                  int
                , int
            ) ;

            void
            on_server_next_song(
            ) ;


            void
            on_server_playlist_changed(
            ) ;

            void
            on_server_state(
                  int /*state*/
            ) ;

            void
            on_server_set_volume(
                  int /*volume*/
            ) ;

            void
            on_server_disconnected(
                  int /*error*/
            ) ;

            void
            on_server_connected(
            ) ;

            void
            on_library_updated(
            ) ;

            // Our own callbacks

            void
            on_search_entry_changed(
            ) ;

            void
            on_search_entry_icon_clicked(
                  Gtk::EntryIconPosition    /*pos*/
                , const GdkEventButton*     /*event*/
            ) ;

            void
            on_artist_changed(
                  int64_t /*artist_id*/
            ) ;

            void
            on_album_changed(
                  int64_t /*album_id*/
            ) ;

            void
            on_search_song_chosen(
                  MPX::Track_sp
                , bool
                , unsigned
            ) ;

            void
            on_browse_song_chosen(
                  const std::string&
            ) ;

            void
            on_browse_album_chosen(
                  int64_t   
            ) ;

            void
            on_song_play(
                  MPX::Track_sp
                , bool
                , unsigned
            ) ;

            void
            on_playlist_clear(
            ) ;

            void
            on_delete_song(
                  int /*id*/
            ) ;

            void
            on_row_moved(
                  int /*pos1*/
                , int /*pos2*/
            ) ;

            void
            on_tracklist_find_propagate(
                  const std::string&
            ) ;

    public:

            void
            on_b_prev() ;

            void
            on_b_next() ;

            void
            on_b_play() ;

            void
            on_b_stop() ;

            virtual Gtk::Widget*
            get_widget()
            {
                return this ; 
            }

            const MPXPD::Metadata&
            get_metadata()
            {
                return m_current_metadata ;
            }

            virtual int
            get_elapsed()
            {
                return m_elapsed ;
            }

    private:

            void
            _update_gui() ;

            void
            _update_cover() ;

            void
            _update_track_info();

            void
            _update_playlist_view() ;

            void
            _update_metadata() ;


            void
            _update_controls() ;

    private:

            void
            on_seek(
                  int /*pos*/
            ) ;

            void
            on_set_volume(
                  int /*volume*/
            ) ;

        private:

            MPXPD::SQL&                         sql ;
            MPXPD::MPD&                         mpd ;
            MPX::Covers                       * covers ;

            Glib::RefPtr<Gnome::Glade::Xml>     xml ;

            int                                 m_elapsed ;
            int                                 m_old_state ;
            bool                                m_new_emitted ;

        public:

            virtual ~Shell()
            {
                Gtk::Window::get_position( Mcs::Key::adaptor<int>(mcs->key("mpx", "window-x")), Mcs::Key::adaptor<int>(mcs->key("mpx", "window-y")));
                Gtk::Window::get_size( Mcs::Key::adaptor<int>(mcs->key("mpx", "window-w")), Mcs::Key::adaptor<int>(mcs->key("mpx", "window-h")));
            }    

            Shell(
                  MPXPD::SQL& sql_
                , MPXPD::MPD& mpd_
                , Glib::RefPtr<Gnome::Glade::Xml> xml_
            ) ;
    } ;
}

#endif // MPXPD_SHELL_HH
