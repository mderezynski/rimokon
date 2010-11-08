#ifndef MPXPD_TITLES_VIEW_HH
#define MPXPD_TITLES_VIEW_HH

#include <gtkmm.h>
#include <glibmm.h>
#include <libglademm.h>

namespace MPXPD
{
    class TitlesView : public Gtk::TreeView
    {
        public:

            struct Columns : public Gtk::TreeModelColumnRecord
            {
                Gtk::TreeModelColumn<int>           Nr ;
                Gtk::TreeModelColumn<std::string>   Name ;
                Gtk::TreeModelColumn<std::string>   Duration ;
                Gtk::TreeModelColumn<int64_t>       Id ;
                Gtk::TreeModelColumn<std::string>   URI ;

                Columns()
                {
                    add( Nr ) ;
                    add( Name ) ;
                    add( Duration ) ;
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

            Columns                         columns ;
            Glib::RefPtr<Gtk::ListStore>    store ;

        public:

            TitlesView()
            {
                set_headers_visible( true ) ;

                store = Gtk::ListStore::create( columns ) ; 


                Gtk::CellRendererText * cell1 = new Gtk::CellRendererText ;
                cell1->property_alignment() = Pango::ALIGN_RIGHT ;
                cell1->property_xalign() = 1.0 ; 
                cell1->property_single_paragraph_mode() = true ;
                Gtk::TreeViewColumn * col1 = new Gtk::TreeViewColumn("Track",*cell1) ; 
                col1->add_attribute(*cell1, "text", columns.Nr ) ;

                append_column( *col1 ) ; 
                append_column( "Title", columns.Name ) ;

                Gtk::CellRendererText * cell = dynamic_cast<Gtk::CellRendererText*>(get_column_cell_renderer(1)) ;
                cell->property_ellipsize() = Pango::ELLIPSIZE_END ;

                set_model( store ) ;

                get_selection()->signal_changed().connect(
                      sigc::mem_fun(*this,&TitlesView::on_title_changed)
                ) ;
                get_selection()->set_mode( Gtk::SELECTION_BROWSE ) ;
    
                set_grid_lines( Gtk::TREE_VIEW_GRID_LINES_BOTH ) ;

                set_rules_hint() ;
            }


        sigc::signal<void,int64_t>       SigNewTitleSelected ;
        sigc::signal<void,int64_t>& 
        signal_new_title_selected()
        {
            return SigNewTitleSelected ; 
        }

        void
        on_title_changed()
        {
            if( get_selection()->count_selected_rows() )
            {
                Gtk::TreeIter iter = get_selection()->get_selected() ;

                if( iter )
                {
                    int64_t id = (*iter)[columns.Id] ;
                    SigNewTitleSelected.emit( id ) ;
                }
            }
        }

        void
        append_title(
              const std::string&    name
            , const int64_t         id
            , const std::string&    uri
            , int64_t               nr
            , int64_t               duration
        )
        {
            Gtk::TreeIter iter = store->append() ;

            (*iter)[columns.Id] = id ;
            (*iter)[columns.Name] = name ;
            (*iter)[columns.URI] = uri ;
            (*iter)[columns.Nr] = nr ;
            (*iter)[columns.Duration] = ((boost::format("%02d:%02d") % (duration / 60) % (duration % 60)).str()) ;
        }

        void
        clear()
        {
            store->clear() ;
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
            std::string uri = (*iter)[columns.URI] ;
            signal_song.emit( uri ) ;
            get_selection()->unselect_all() ;
        }
    } ;
}

#endif //
