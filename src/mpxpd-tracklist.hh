#ifndef MPXPD_TRACKLIST_VIEW_HH
#define MPXPD_TRACKLIST_VIEW_HH

#include <gtkmm.h>
#include <glibmm.h>
#include <libglademm.h>
#include <deque>
#include <vector>
#include <boost/algorithm/string.hpp>

namespace MPXPD
{
    class TracklistView : public Gtk::TreeView
    {
        public:

            struct Columns : public Gtk::TreeModelColumnRecord
            {
                Gtk::TreeModelColumn<std::string>   AlbumArtist ;
                Gtk::TreeModelColumn<std::string>   Album ;
                Gtk::TreeModelColumn<int>           Nr ;
                Gtk::TreeModelColumn<std::string>   Name ;
                Gtk::TreeModelColumn<std::string>   TrackArtist ;
                Gtk::TreeModelColumn<int64_t>       Id ;
                Gtk::TreeModelColumn<std::string>   URI ;

                Columns()
                {
                    add( AlbumArtist ) ;
                    add( Album ) ;
                    add( Nr ) ;
                    add( Name ) ;
                    add( TrackArtist ) ;
                    add( Id ) ;
                    add( URI ) ;
                }
            } ;

        private:

            typedef sigc::signal<void,std::string>  signal_song_t ; 
            signal_song_t                           signal_song ;

        public:

            signal_song_t&
            signal_song_chosen()
            {
                return signal_song ;
            }

        private:

            Columns                                 columns ;
            Glib::RefPtr<Gtk::ListStore>            store ;
            Glib::RefPtr<Gtk::TreeModelFilter>      filter_store ;

            bool
            match_keys(
                  Glib::ustring const& _h
                , Glib::ustring const& _n
            )
            {
                using boost::algorithm::split;
                using boost::algorithm::is_any_of;
                using boost::algorithm::find_first;

                std::vector<std::string> m ;

                std::string n (_n.lowercase());
                std::string h (_h.lowercase());

                split( m, n, is_any_of(" ") );

                for( std::vector<std::string>::const_iterator i = m.begin (); i != m.end (); ++i)
                {
                    if (i->length() < 1)
                        continue;

                    if (!find_first (h, (*i)))
                        return false;
                }

                return true;
            }

            bool
            visible_func(
                const Gtk::TreeModel::const_iterator&  iter
            )
            {
                std::string haystack = (*iter)[columns.Name] ;

                if( haystack.empty() )
                    return true ; 

                return match_keys( haystack, previous_filter ) ;
            } 

            typedef Glib::RefPtr<Gtk::TreeModelFilter> FM ;

            std::deque<FM> Q ; 

            std::string previous_filter ;

        public:

            void
            set_filter( const std::string& str )
            {
                previous_filter = str ;
                filter_store->refilter() ;
            }

            TracklistView()
            {
                store = Gtk::ListStore::create( columns ) ; 
                filter_store = Gtk::TreeModelFilter::create( store ) ;
                filter_store->set_visible_func( sigc::mem_fun( *this, &TracklistView::visible_func )) ;

                Gtk::CellRendererText * cell1 = new Gtk::CellRendererText ;
                cell1->property_alignment() = Pango::ALIGN_RIGHT ;
                cell1->property_xalign() = 1.0 ; 
                cell1->property_single_paragraph_mode() = true ;
                Gtk::TreeViewColumn * col1 = new Gtk::TreeViewColumn("Nr",*cell1) ; 
                col1->add_attribute(*cell1, "text", columns.Nr ) ;

                append_column( *col1 ) ; 
                append_column( "Title", columns.Name ) ;
                append_column( "Artist", columns.TrackArtist ) ;
                append_column( "Album", columns.Album) ;

                get_column(0)->set_resizable() ;
                get_column(1)->set_resizable() ;
                get_column(2)->set_resizable() ;
                get_column(3)->set_resizable() ;
            }

        void
        set_store()
        {
            set_model( filter_store ) ;
            columns_autosize() ;
        }

        void
        append_title(
              const std::string&    album_artist
            , const std::string&    track_artist
            , const std::string&    album
            , const std::string&    name
            , const int64_t         id
            , const std::string&    uri
            , int64_t               nr
        )
        {
            Gtk::TreeIter iter = store->append() ;

            (*iter)[columns.Id] = id ;
            (*iter)[columns.Name] = name ;
            (*iter)[columns.URI] = uri ;
            (*iter)[columns.Nr] = nr ;
            (*iter)[columns.TrackArtist] = track_artist ;
            (*iter)[columns.AlbumArtist] = album_artist ;
            (*iter)[columns.Album] = album ;
        }

        void
        clear()
        {
            store->clear() ;
        }

    } ;
}

#endif //
