#ifndef MPXPD_ALBUM_ARTISTS
#define MPXPD_ALBUM_ARTISTS

#include <gtkmm.h>
#include <glibmm.h>
#include <libglademm.h>
#include <boost/format.hpp>

namespace MPXPD
{
    class ArtistsView : public Gtk::TreeView
    {
        public:

            struct Columns : public Gtk::TreeModelColumnRecord
            {
                Gtk::TreeModelColumn<std::string>   Name ;
                Gtk::TreeModelColumn<int64_t>       Id ;

                Columns()
                {
                    add( Name ) ;
                    add( Id ) ;
                }
            } ;

        private:

            Columns                         columns ;
            Glib::RefPtr<Gtk::ListStore>    store ;

        public:

            ArtistsView()
            {
                set_headers_visible( true ) ;

                store = Gtk::ListStore::create( columns ) ; 

                append_column( "Artist", columns.Name ) ;

                Gtk::CellRendererText * cell = dynamic_cast<Gtk::CellRendererText*>(get_column_cell_renderer(0)) ;
                cell->property_ellipsize() = Pango::ELLIPSIZE_END ;

                set_model( store ) ;

                get_selection()->signal_changed().connect(
                      sigc::mem_fun(*this,&ArtistsView::on_artist_changed)
                ) ;
                get_selection()->set_mode( Gtk::SELECTION_BROWSE ) ;

                set_rules_hint() ;
            }


        sigc::signal<void,int64_t>       SigNewArtistSelected ;
        sigc::signal<void,int64_t>& 
        signal_new_artist_selected()
        {
            return SigNewArtistSelected ; 
        }

        void
        on_artist_changed()
        {
            if( get_selection()->count_selected_rows() )
            {
                Gtk::TreeIter iter = get_selection()->get_selected() ;

                if( iter )
                {
                    int64_t id = (*iter)[columns.Id] ;
                    SigNewArtistSelected.emit( id ) ;
                    return ;
                }
            }

            SigNewArtistSelected.emit( -1 ) ;
        }

        void
        append_artist(
              const std::string&    name
            , const int64_t         id
        )
        {
            Gtk::TreeIter iter = store->append() ;

            (*iter)[columns.Id] = id ;
            (*iter)[columns.Name] = name ;
        }

        void
        clear()
        {
            store->clear() ;
        }
    } ;
}

#endif
