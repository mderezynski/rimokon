#ifndef MPXPD_ALBUMS_VIEW_HH
#define MPXPD_ALBUMS_VIEW_HH

#include <gtkmm.h>
#include <glibmm.h>
#include <libglademm.h>
#include <boost/format.hpp>

namespace MPXPD
{
    class AlbumsView : public Gtk::TreeView
    {
        public:

            struct Columns : public Gtk::TreeModelColumnRecord
            {
                Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> >    Image ;
                Gtk::TreeModelColumn<std::string>                   Name ;
                Gtk::TreeModelColumn<std::string>                   BareName ;
                Gtk::TreeModelColumn<int64_t>                       Id ;
                Gtk::TreeModelColumn<std::string>                   MPXId ;

                Columns()
                {
                    add( Image ) ;
                    add( Name ) ;
                    add( BareName ) ; 
                    add( Id ) ;
                    add( MPXId ) ;
                }
            } ;

        private:

            Columns                         columns ;
            Glib::RefPtr<Gtk::ListStore>    store ;

        private:

            typedef sigc::signal<void,int64_t>  signal_album_t ; 
            signal_album_t                      signal_album ;

        public:

            signal_album_t&
            signal_album_chosen()
            {
                return signal_album ;
            }

        public:

            void
	        begin_add()
	        {
		        unset_model() ;
                hide() ;
            }

            void
            end_add()
            {
                set_model( store ) ;
                set_search_column( 2 ) ;
                set_enable_search() ;
                show() ;
            }

            AlbumsView()
            {
                set_headers_visible( true ) ;

                store = Gtk::ListStore::create( columns ) ; 
    
                append_column( "", columns.Image ) ;
                Gtk::TreeViewColumn * col = new Gtk::TreeViewColumn("Album") ;
                Gtk::CellRendererText * cell = new Gtk::CellRendererText ;
                cell->property_ellipsize() = Pango::ELLIPSIZE_END ;
                col->pack_start( *cell ) ;
                col->add_attribute( *cell, "markup", columns.Name ) ;
                append_column(*col) ;

                get_column_cell_renderer(0)->set_padding( 4, 4 ) ;

                set_model( store ) ;
                set_enable_search() ;
                set_search_column( 2 ) ;

                get_selection()->signal_changed().connect(
                      sigc::mem_fun(*this,&AlbumsView::on_album_changed)
                ) ;

                set_rules_hint() ;
            }


        sigc::signal<void,int64_t>       SigNewAlbumSelected ;
        sigc::signal<void,int64_t>& 
        signal_new_album_selected()
        {
            return SigNewAlbumSelected ; 
        }

        void
        on_album_changed()
        {
            if( get_selection()->count_selected_rows() )
            {
                Gtk::TreeIter iter = get_selection()->get_selected() ;
    
                if( iter )
                {
                    int64_t id = (*iter)[columns.Id] ;
                    SigNewAlbumSelected.emit( id ) ;
                }
            }
        }

        void
        append_album(
              const std::string&        name
            , const int64_t             id
            , const std::string&        mpxid
            , Glib::RefPtr<Gdk::Pixbuf> image
            , const std::string&        date
        )
        {
            Gtk::TreeIter iter = store->append() ;

            (*iter)[columns.Image] = image ;
            (*iter)[columns.Name] = (boost::format("<b>%s</b>\n<small>%s</small>") % Glib::Markup::escape_text(name) % date).str() ;
            (*iter)[columns.BareName] = name ; 
            (*iter)[columns.Id] = id ;
            (*iter)[columns.MPXId] = mpxid ;
        }

        void
        clear()
        {
            store->clear() ;
        }

        std::size_t
        get_size()
        {
            return store->children().size() ;
        }

        void
        select_first()
        {
            get_selection()->select( Gtk::TreePath("0")) ;
        }

        void
        autosize()
        {
            columns_autosize() ;
        }

        virtual void
        on_row_activated(
              const Gtk::TreeModel::Path&   path
            , Gtk::TreeViewColumn*          col
        )
        {
            Gtk::TreeIter iter = store->get_iter(path) ;
            int64_t id = (*iter)[columns.Id] ;
            signal_album.emit( id ) ;
        }
    } ;
}

#endif
