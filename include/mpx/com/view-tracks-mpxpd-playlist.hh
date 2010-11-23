#ifndef YOUKI_VIEW_TRACKS_PLAYLIST_HH
#define YOUKI_VIEW_TRACKS_PLAYLIST_HH

#include <gtkmm.h>
#include <cairomm/cairomm.h>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/optional.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <cmath>
#include <deque>
#include <boost/unordered_set.hpp>
#include <sigx/sigx.h>

#include "mpx/util-string.hh"
#include "mpx/util-graphics.hh"

#include "mpx/mpx-types.hh"
#include "mpx/mpx-main.hh"
#include "mpx/mpx-covers.hh"

#include "mpx/algorithm/aque.hh"
#include "mpx/algorithm/ntree.hh"
#include "mpx/algorithm/interval.hh"
#include "mpx/algorithm/limiter.hh"

#include "mpx/aux/glibaddons.hh"
#include "mpx/widgets/cairo-extensions.hh"
#include "mpx/i-youki-theme-engine.hh"

#include "mpx/mpxpd-sql.hh"

#include "glib-marshalers.h"
// ugh

typedef Glib::Property<Gtk::Adjustment*> PropAdj;

namespace MPX
{
namespace View
{
namespace Playlist
{

        using namespace boost;

        namespace
        {
            const double rounding = 4. ; 
        }

        typedef boost::tuple<std::string, std::string, std::string, int64_t, Track_sp, int64_t, int64_t, std::string, int64_t, int64_t>  Row_t ;

        typedef std::vector<Row_t>                  Model_t ;
        typedef boost::shared_ptr<Model_t>          Model_sp_t ;
        typedef sigc::signal<void>                  Signal1 ;

        struct Model_t_iterator_equal
            : std::binary_function<Model_t::iterator, Model_t::iterator, bool>
        {
            bool operator()(
                  const Model_t::iterator& a
                , const Model_t::iterator& b
            ) const
            {
                return a == b ;
            }

            bool operator()(
                  const Model_t::const_iterator& a
                , const Model_t::const_iterator& b
            ) const
            {
                return a == b ;
            }

            bool operator()(
                  const Model_t::const_iterator& a
                , const Model_t::iterator& b
            ) const
            {
                return a == b ;
            }

            bool operator()(
                  const Model_t::iterator& a
                , const Model_t::const_iterator& b
            ) const
            {
                return a == b ;
            }
        } ;

        struct Model_t_iterator_hash
        {
            std::size_t operator()( const Model_t::iterator& i ) const
            {
                return GPOINTER_TO_INT(&(*i)) ;
            }

            std::size_t operator()( Model_t::iterator& i ) const
            {
                return GPOINTER_TO_INT(&(*i)) ;
            }

            std::size_t operator()( const Model_t::const_iterator& i ) const
            {
                return GPOINTER_TO_INT(&(*i)) ;
            }

            std::size_t operator()( Model_t::const_iterator& i ) const
            {
                return GPOINTER_TO_INT(&(*i)) ;
            }
        } ;

        struct OrderFunc
            : public std::binary_function<Row_t, Row_t, bool>
        {
            bool operator() (
                  const Row_t&  a
                , const Row_t&  b
            )
            {
                const std::string&  order_artist_a = get<7>( a ) ; 
                const std::string&  order_artist_b = get<7>( b ) ; 

                const std::string&  order_album_a  = get<2>( a ) ; 
                const std::string&  order_album_b  = get<2>( b ) ; 

                int64_t order_track [2] = {
                      get<5>( a )
                    , get<5>( b )
                } ;

                if( order_artist_a < order_artist_b)
                    return true ;

                if( order_album_a < order_album_b )
                    return true ;

                if( order_track[0] < order_track[1] )
                    return true ;

                return false ;
            }
        } ;

        struct DataModel
        : public sigc::trackable
        {
                Model_sp_t      m_realmodel ;
                Signal1         m_changed ;

                DataModel()
                {
                    m_realmodel = Model_sp_t(new Model_t); 
                }

                DataModel(Model_sp_t model)
                {
                    m_realmodel = model; 
                }

                virtual void
                clear()
                {
                    m_realmodel->clear () ;
                    m_changed.emit() ;
                } 

                virtual Signal1&
                signal_changed()
                {
                    return m_changed ;
                }

                virtual bool
                is_set()
                {
                    return bool(m_realmodel) ;
                }

                virtual std::size_t
                size()
                {
                    return m_realmodel->size() ;
                }

                virtual const Row_t&
                row(std::size_t row)
                {
                    return (*m_realmodel)[row] ;
                }

                void
                swap( std::size_t p1, std::size_t p2 )
                {
                    std::swap( (*m_realmodel)[p1], (*m_realmodel)[p2] ) ;
                    m_changed.emit() ;
                }

                virtual void
                erase(
                      Model_t::iterator iter1
                    , Model_t::iterator iter2
                )
                {
                    m_realmodel->erase( iter1, iter2 ) ;
                    m_changed.emit() ;
                }

                virtual void
                append_track(
                      ::MPXPD::sql_row_t&   r
                    , const MPX::Track_sp&  track
                    , int                   pl_id 
                )
                {
                    using boost::get ;

                    int64_t id = 0 ;

                    if( r.count("tracks_id") )
                    { 
                        id = get<int64_t>(r["tracks_id"]) ;
                    }
                    else
                    {
                        g_critical("%s: No id for track, extremely suspicious", G_STRLOC) ;
                        throw std::logic_error("No ID given for track") ;
                    }

                    Row_t row (
                              r.count("tracks_name") ? get<std::string>(r["tracks_name"]) : ""
                            , r.count("track_artists_name") ? get<std::string>(r["track_artists_name"]) : "" 
                            , r.count("albums_name") ? get<std::string>(r["albums_name"]) : ""
                            , id 
                            , track 
                            , r.count("tracks_nr") ? get<int64_t>(r["tracks_nr"]) : int64_t(0)
                            , r.count("artists_id") ? get<int64_t>(r["artists_id"]) : int64_t(0) 
                            , r.count("artists_name") ? get<std::string>(r["artists_name"]) : ""
                            , pl_id 
                            , r.count("tracks_duration") ? get<int64_t>(r["tracks_duration"]) : 0 
                    ) ;

                    m_realmodel->push_back(row) ;
                    m_changed.emit() ;
                }

                virtual void
                set_track(
                      ::MPXPD::sql_row_t&   r
                    , const MPX::Track_sp&  track
                    , int                   pl_id 
                    , Model_t::iterator     iter 
                )
                {
                    using boost::get ;

                    int64_t id = 0 ;

                    if( r.count("tracks_id") )
                    { 
                        id = get<int64_t>(r["tracks_id"]) ;
                    }
                    else
                    {
                        g_critical("%s: No id for track, extremely suspicious", G_STRLOC) ;
                        throw std::logic_error("No ID given for track") ;
                    }

                    Row_t row (
                              r.count("tracks_name") ? get<std::string>(r["tracks_name"]) : ""
                            , r.count("track_artists_name") ? get<std::string>(r["track_artists_name"]) : "" 
                            , r.count("albums_name") ? get<std::string>(r["albums_name"]) : ""
                            , id 
                            , track 
                            , r.count("tracks_nr") ? get<int64_t>(r["tracks_nr"]) : int64_t(0)
                            , r.count("artists_id") ? get<int64_t>(r["artists_id"]) : int64_t(0) 
                            , r.count("artists_name") ? get<std::string>(r["artists_name"]) : ""
                            , pl_id 
                            , r.count("tracks_duration") ? get<int64_t>(r["tracks_duration"]) : 0 
                    ) ;

                    *iter = row ;
                    m_changed.emit() ;
                }

                void
                erase_track(int64_t id)
                {
                    for( Model_t::iterator i = m_realmodel->begin(); i != m_realmodel->end(); ++i )
                    {
                        int64_t model_id = get<3>( *i ) ;
                        if( model_id == id )
                        {
                            m_realmodel->erase( i ) ;
                            return ;
                        }
                    }
                }
        };

        typedef boost::shared_ptr<DataModel> DataModel_sp_t;

        class Column
        {
                int                 m_width ;
                int                 m_column ;
                std::string         m_title ;
                Pango::Alignment    m_alignment ;
                
            public:

                Column (std::string const& title)
                : m_width( 0 )
                , m_column( 0 )
                , m_title( title )
                , m_alignment( Pango::ALIGN_LEFT )
                {
                }

                ~Column ()
                {
                }

                void
                set_width (int width)
                {
                    if( m_width != width )
                    {
                        m_width = width; 
                    }
                }

                int
                get_width ()
                {
                    return m_width;
                }

                void
                set_column (int column)
                {
                    m_column = column;
                }

                int
                get_column ()
                {
                    return m_column;
                }

                void
                set_title (std::string const& title)
                {
                    m_title = title;
                }

                std::string const&
                get_title ()
                {
                    return m_title;
                }

                void
                set_alignment(
                    Pango::Alignment align
                )
                {
                    m_alignment = align ;
                }


                Pango::Alignment
                get_alignment(
                )
                {
                    return m_alignment ;
                }


                void
                render_header(
                      Cairo::RefPtr<Cairo::Context>&    cairo
                    , Gtk::Widget&                      widget
                    , int                               xpos
                    , int                               ypos
                    , int                               rowheight
                    , int                               column
                    , const ThemeColor&                 color
                )
                {
                    using boost::get;

                    cairo->rectangle(
                          xpos + 5
                        , ypos + 6
                        , m_width
                        , rowheight
                    ) ;

                    cairo->clip() ;

                    cairo->move_to(
                          xpos + 5
                        , ypos + 6
                    ) ;

                    cairo->set_operator(Cairo::OPERATOR_OVER);

                    cairo->set_source_rgba(
                          color.r
                        , color.g
                        , color.b
                        , color.a 
                    ) ; 

                    Glib::RefPtr<Pango::Layout> layout = widget.create_pango_layout(""); 

                    layout->set_markup(
                          (boost::format("<b>%s</b>") % Glib::Markup::escape_text(m_title).c_str()).str()
                    ) ;

                    layout->set_ellipsize(
                          Pango::ELLIPSIZE_END
                    ) ;

                    layout->set_width(
                          (m_width-10)*PANGO_SCALE
                    ) ;

                    layout->set_alignment(
                          m_alignment
                    ) ;

                    pango_cairo_show_layout(
                          cairo->cobj()
                        , layout->gobj()
                    ) ;

                    cairo->reset_clip() ;
                }

                void
                render(
                      Cairo::RefPtr<Cairo::Context>&    cairo
                    , Gtk::Widget&                      widget
                    , const Row_t&                      datarow
                    , int                               row
                    , int                               xpos
                    , int                               ypos
                    , int                               rowheight
                    , const ThemeColor&                 color
                    , double                            alpha
                )
                {
                    using boost::get ;

                    cairo->set_operator(Cairo::OPERATOR_OVER) ;

                    cairo->set_source_rgba(
                          color.r
                        , color.g
                        , color.b
                        , color.a * alpha 
                    ) ; 

                    cairo->rectangle(
                          xpos
                        , ypos
                        , m_width
                        , rowheight
                    ) ;

                    cairo->clip();

                    {
                      cairo->move_to(
                            xpos + 6
                          , ypos + 2
                      ) ;

                      std::string str ;

                      switch( m_column )
                      {
                          case 0:
                              str = get<0>(datarow);
                              break;
                          case 1:
                              str = get<1>(datarow);
                              break;
                          case 2:
                              str = get<2>(datarow);
                              break;
                          case 3:
                              str = get<3>(datarow);
                              break;
                          case 5:
                              str = boost::lexical_cast<std::string>(get<5>(datarow)) ;
                              break;
                          case 9:
                              {
                                int64_t time_ = get<9>(datarow) ;
                                str = ((boost::format("%02d:%02d") % (time_/60) % (time_ % 60)).str()) ;
                              }
                              break;
                      }

                      Glib::RefPtr<Pango::Layout> layout; 

                      layout = widget.create_pango_layout( str );

                      layout->set_ellipsize(
                            Pango::ELLIPSIZE_END
                      ) ;

                      layout->set_width(
                            (m_width - 12) * PANGO_SCALE
                      ) ;

                      layout->set_alignment(
                            m_alignment
                      ) ;

                      pango_cairo_show_layout(
                            cairo->cobj()
                          , layout->gobj()
                      ) ;

                    }

                    cairo->reset_clip();
                  }
        };

        typedef boost::shared_ptr<Column>               Column_sp_t ;
        typedef std::vector<Column_sp_t>                Columns ;

        typedef sigc::signal<void, MPX::Track_sp, bool, unsigned>   SignalTrackActivated ;
        typedef sigc::signal<void>                                  SignalVAdjChanged ;
        typedef sigc::signal<void>                                  SignalFindAccepted ;
        typedef sigc::signal<void, const std::string&>              SignalFindPropagate ;
        typedef sigc::signal<void, int, int>                        SignalRowMoved ;

        class Class
        : public Gtk::DrawingArea
        {
            public:

                DataModel_sp_t                      m_model ;

            private:

                int                                 m_row_height ;
                int                                 m_row_start ;
                int                                 m_visible_height ;

                Columns                             m_columns ;

                PropAdj                             m_prop_vadj ;
                PropAdj                             m_prop_hadj ;

                boost::optional<std::size_t>        m_selection ;

                int64_t                             m_clicked_row ;
                bool                                m_clicked ;
                bool                                m_highlight ;

                boost::optional<int64_t>            m_id_currently_playing ;
                boost::optional<int64_t>            m_row_currently_playing ;
                boost::optional<int64_t>            m_hover_track ;

                Glib::RefPtr<Gdk::Pixbuf>           m_pb_play_l ;
                Glib::RefPtr<Gdk::Pixbuf>           m_pb_hover_l ;

                std::set<int>                       m_collapsed ;
                std::set<int>                       m_fixed ;
                int64_t                             m_fixed_total_width ;
        
                Gtk::Entry                        * m_SearchEntry ;
                Gtk::Window                       * m_SearchWindow ;
                Gtk::HBox                         * m_SearchHBox ;
                Gtk::Button                       * m_SearchButton ;

                sigc::connection                    m_search_changed_conn ; 
                bool                                m_search_active ;

                Glib::RefPtr<Gtk::ActionGroup>      m_refActionGroup ;
                Gtk::Menu*                          m_pMenuPopup ;

                typedef sigc::signal<void>          SignalClear ;
                typedef sigc::signal<void,int>      SignalDelPos ;

                SignalClear     _signal_0 ; 
                SignalDelPos    _signal_1 ; 

                SignalTrackActivated                m_SIGNAL_track_activated ;
                SignalVAdjChanged                   m_SIGNAL_vadj_changed ;
                SignalFindAccepted                  m_SIGNAL_find_accepted ;
                SignalFindPropagate                 m_SIGNAL_find_propagate ;
                SignalRowMoved                      m_SIGNAL_row_moved ;

                Interval<std::size_t>               m_Model_I ;

                Glib::RefPtr<Gtk::Action>           m_Action_Remove_Selected ;
                Glib::RefPtr<Gtk::Action>           m_Action_Clear_Playlist ;

                void
                initialize_metrics ()
                {
                    PangoContext *context = gtk_widget_get_pango_context (GTK_WIDGET (gobj()));

                    PangoFontMetrics *metrics = pango_context_get_metrics (context,
                                                                            GTK_WIDGET (gobj())->style->font_desc, 
                                                                            pango_context_get_language (context));

                    m_row_height = (pango_font_metrics_get_ascent (metrics)/PANGO_SCALE) + 
                                   (pango_font_metrics_get_descent (metrics)/PANGO_SCALE) + 5 ;

                    const int visible_area_pad = 2 ;

                    m_row_start = m_row_height + visible_area_pad ;
                }

                void
                on_vadj_value_changed ()
                {
                    if( m_model->size() )
                    {
                        queue_draw ();
                        m_SIGNAL_vadj_changed.emit() ;
                    }
                }

            protected:

                virtual bool
                on_focus_in_event (GdkEventFocus* G_GNUC_UNUSED)
                {
                    if( !m_selection && m_model->size() )
                    {
                        select_row( get_upper_row() ) ; 
                    }

                    queue_draw() ;

                    return false ;
                }

                bool
                key_press_event (GdkEventKey * event)
                {
                    if( event->is_modifier )
                        return false ;

                    if( !m_model->size() )
                        return false ;

                    if( m_search_active )
                    {
                        switch( event->keyval )
                        {
                            case GDK_Up:
                            case GDK_KP_Up:
                                find_prev_match() ;
                                return true ;

                            case GDK_Down:
                            case GDK_KP_Down:
                                find_next_match() ;
                                return true ;

                            case GDK_Escape:
                                cancel_search() ;
                                return true ;

                            case GDK_Tab:
                                cancel_search() ;
                                return false ;

                            case GDK_Return:
                            case GDK_KP_Enter:
                            case GDK_ISO_Enter:
                            case GDK_3270_Enter:
                                cancel_search() ;
                                goto continue_matching ;
        
                            default: ;
                        }

                        GdkEvent *new_event = gdk_event_copy( (GdkEvent*)(event) ) ;
                        g_object_unref( ((GdkEventKey*)new_event)->window ) ;
                        ((GdkEventKey *) new_event)->window = GDK_WINDOW(g_object_ref(G_OBJECT(GTK_WIDGET(m_SearchWindow->gobj())->window))) ;
                        gtk_widget_event(GTK_WIDGET(m_SearchEntry->gobj()), new_event) ;
                        gdk_event_free(new_event) ;

                        return true ;
                    }

                    continue_matching:

                    int step = 0 ; 
                    int origin = m_selection ? m_selection.get() : 0 ;

                    switch( event->keyval )
                    {
/*
                        case GDK_Delete:
                        {
                            if( m_selection )
                            {
                                std::size_t p = origin ;
                                m_selection.reset() ;
                                m_model->erase( p ) ;
                            }
                            return true ;
                        }
*/

                        case GDK_Return:
                        case GDK_KP_Enter:
                        case GDK_ISO_Enter:
                        case GDK_3270_Enter:
                        {
                            if( m_search_active )
                            {
                                cancel_search() ;
                            }

                            if( m_selection )
                            {
                                using boost::get;

                                MPX::Track_sp track = get<4>(m_model->row(m_selection.get())) ;
                                m_SIGNAL_track_activated.emit( track, !(event->state & GDK_CONTROL_MASK), m_selection.get()) ;
                            }

                            return true;
                        }

                        case GDK_Up:
                        case GDK_KP_Up:
                        case GDK_Page_Up:
                        {
                            if( event->keyval == GDK_Page_Up )
                            {
                                step = get_page_size() ; 
                            }
                            else
                            {
                                step = 1 ;
                            }

                            if( !m_selection || !get_row_is_visible( origin ))
                            {
                                select_row( get_upper_row() ) ;
                            }
                            else
                            {
                                std::size_t row = ((origin-step) < 0 ) ? 0 : (origin-step) ;

                                if( row < get_upper_row() ) 
                                {
                                    m_prop_vadj.get_value()->set_value( std::max<int>( int(get_upper_row()) - step, row )) ; 
                                }
    
                                select_row( row ) ;
                            }

                            m_hover_track.reset() ;

                            return true;
                        }

                        case GDK_Home:
                        {
                            select_row( 0 ) ;
                            scroll_to_row( 0 ) ;

                            return true ;
                        }

                        case GDK_End:
                        {
                            select_row( m_model->size() - 1 ) ;
                            scroll_to_row( m_model->size() - get_page_size() ) ;

                            return true ;
                        }

                        case GDK_Down:
                        case GDK_KP_Down:
                        case GDK_Page_Down:
                        {
                            if( event->keyval == GDK_Page_Down )
                            {
                                step = get_page_size() ; 
                            }
                            else
                            {
                                step = 1 ;
                            }

                            if( !m_selection || !get_row_is_visible( origin ))
                            {
                                select_row( get_upper_row() ) ;
                            }
                            else
                            {
                                std::size_t row = std::min<std::size_t>( origin+step, m_model->size()-1 ) ;

                                if( row >= get_lower_row() ) 
                                {
                                    m_prop_vadj.get_value()->set_value( std::min<std::size_t>(get_upper_row()+step, row )) ;
                                }

                                select_row( row ) ;
                            }

                            m_hover_track.reset() ;

                            return true;
                        }

                        default:
                            break;

/*
                            if( !m_search_active )
                            {
                                int x, y, x_root, y_root ;

                                dynamic_cast<Gtk::Window*>(get_toplevel())->get_position( x_root, y_root ) ;

                                x = x_root + get_allocation().get_x() ;
                                y = y_root + get_allocation().get_y() + get_allocation().get_height() ;

                                m_SearchWindow->set_size_request( m_columns[1]->get_width(), - 1 ) ;
                                m_SearchWindow->move( x, y ) ;
                                m_SearchWindow->show() ;

                                send_focus_change( *m_SearchEntry, true ) ;

                                GdkEvent *new_event = gdk_event_copy( (GdkEvent*)(event) ) ;
                                g_object_unref( ((GdkEventKey*)new_event)->window ) ;
                                gtk_widget_realize( GTK_WIDGET(m_SearchWindow->gobj()) ) ;
                                ((GdkEventKey *) new_event)->window = GDK_WINDOW(g_object_ref(G_OBJECT(GTK_WIDGET(m_SearchWindow->gobj())->window))) ;
                                gtk_widget_event(GTK_WIDGET(m_SearchEntry->gobj()), new_event) ;
                                gdk_event_free(new_event) ;

                                m_search_active = true ;

                                return false ;
                            }
*/
                    }

                    return false ;
                }

                void
                send_focus_change(
                      Gtk::Widget&  w
                    , bool          in
                    )
                {
                    GtkWidget * widget = w.gobj() ;

                    GdkEvent *fevent = gdk_event_new (GDK_FOCUS_CHANGE);

                    g_object_ref (widget);

                   if( in )
                      GTK_WIDGET_SET_FLAGS( widget, GTK_HAS_FOCUS ) ;
                    else
                      GTK_WIDGET_UNSET_FLAGS( widget, GTK_HAS_FOCUS ) ;

                    fevent->focus_change.type   = GDK_FOCUS_CHANGE;
                    fevent->focus_change.window = GDK_WINDOW(g_object_ref( widget->window )) ;
                    fevent->focus_change.in     = in;

                    gtk_widget_event( widget, fevent ) ;

                    g_object_notify(
                          G_OBJECT (widget)
                        , "has-focus"
                    ) ;

                    g_object_unref( widget ) ;
                    gdk_event_free( fevent ) ;
                }

                bool
                on_button_press_event (GdkEventButton * event)
                {
                    using boost::get;

                    {
                        cancel_search() ;
                        grab_focus() ;

                        int x = event->x ;

                        if( event->y < (m_row_height+4))
                        {
                            int p = 32 ;

                            for( std::size_t n = 0; n < m_columns.size() ; ++n )
                            {
                                int w = m_columns[n]->get_width() ;

                                if( (x >= p) && (x <= p + w) && !m_fixed.count(n) )
                                {
                                    column_set_collapsed( n, !m_collapsed.count( n ) ) ;
                                    break ;
                                }

                                p += w ;
                            }
                            return true;
                        }

                        Limiter<int64_t> row ( 
                              Limiter<int64_t>::ABS_ABS
                            , 0
                            , m_model->size() - 1
                            , get_upper_row() + (event->y-m_row_start) / m_row_height
                        ) ;

                        if( x >= 32 && m_model->size() ) 
                        {
                            select_row( row ) ;

                            m_clicked_row   = row ;
                            m_clicked       = true ;
                            queue_draw() ;
                        }
                        else
                        if( x >= 16 )
                        {
                            /* reserved for future use for clicking on the hover 'play' icon */
                        }
                    }

                    if( event->type == GDK_2BUTTON_PRESS && event->button == 1 )
                    {
                        if( event->y < m_row_height )
                            return false ;

                        Limiter<int64_t> row ( 
                              Limiter<int64_t>::ABS_ABS
                            , 0
                            , m_model->size()
                            , get_upper_row() + (event->y-m_row_start) / m_row_height
                        ) ;

                        Interval<std::size_t> i (
                              Interval<std::size_t>::IN_EX
                            , 0
                            , m_model->size()
                        ) ;

                        if( i.in( row )) 
                        {
                            MPX::Track_sp track = get<4>(m_model->row(row)) ;
                            m_SIGNAL_track_activated.emit( track, true, row ) ;
                        }
                    
                        return true ;
                    }
                
                    if( event->button == 3 )
                    {
                        m_clicked = false ;
                        m_pMenuPopup->popup(event->button, event->time) ;                            
                        return true ;
                    }

                    return true;
                }

                bool
                on_button_release_event (GdkEventButton * event)
                {
                    m_clicked = false ;
                    return true ;
                }

                bool
                on_leave_notify_event(
                    GdkEventCrossing* G_GNUC_UNUSED
                )
                {
                    m_hover_track.reset() ;
                    queue_draw () ;

                    return true ;
                }

                bool
                on_motion_notify_event(
                    GdkEventMotion* event
                )
                {
                    int x_orig, y_orig;
                    GdkModifierType state;

                    if (event->is_hint)
                    {
                        gdk_window_get_pointer( event->window, &x_orig, &y_orig, &state ) ;
                    }
                    else
                    {
                        x_orig = int( event->x ) ;
                        y_orig = int( event->y ) ;
                        state  = GdkModifierType( event->state ) ;
                    }

                    int64_t row = get_upper_row() + ( y_orig - m_row_start ) / m_row_height ;

                    if( !m_clicked )
                    {
                            if( m_Model_I.in( row ) && y_orig >= m_row_start ) 
                            {
                                m_hover_track = row ;
                                queue_draw_area( 0, m_row_start, 32, get_allocation().get_height() - m_row_start ) ;
                            }
                            else
                            {
                                m_hover_track.reset() ;
                                queue_draw_area( 0, m_row_start, 32, get_allocation().get_height() - m_row_start ) ;
                            }
                    }
                    else
                    {
                            if( row != m_clicked_row )
                            {
                                if( m_Model_I.in( row )) 
                                {
                                    m_model->swap( row, m_clicked_row ) ;
                                    m_SIGNAL_row_moved.emit( row, m_clicked_row ) ;
                                    m_clicked_row = row ;
                                    select_row( row ) ;
                                }
                            }
                    }

                    return true ;
                }

                void
                configure_vadj(
                      std::size_t   upper
                    , std::size_t   page_size
                    , std::size_t   step_increment
                )
                {
                    if( m_prop_vadj.get_value() )
                    {
                        m_prop_vadj.get_value()->set_upper( upper ) ; 
                        m_prop_vadj.get_value()->set_page_size( page_size ) ; 
                        m_prop_vadj.get_value()->set_step_increment( step_increment ) ; 
                    }
                }

                bool
                on_configure_event(
                    GdkEventConfigure* event
                )        
                {
                    const double column_width_collapsed = 40. ;

                    m_visible_height = event->height - m_row_start ;

                    configure_vadj(
                          m_model->size()
                        , get_page_size()
                        , 1
                    ) ;

                    int width = event->width - 32 ;

                    double column_width_calculated = (double(width) - double(m_fixed_total_width) - double(column_width_collapsed*double(m_collapsed.size()))) / (m_columns.size() - m_collapsed.size() - m_fixed.size()) ;

                    for( std::size_t n = 0; n < m_columns.size(); ++n )
                    {
                        if( !m_fixed.count( n ) )
                        {
                            m_columns[n]->set_width(m_collapsed.count( n ) ? column_width_collapsed : column_width_calculated ) ; 
                        }
                    }

                    return false;
                }

                inline bool
                compare_id_to_optional(
                      const Row_t&                      row
                    , const boost::optional<int64_t>&    id
                )
                {
                    if( id && id.get() == boost::get<3>( row ))
                        return true ;

                    return false ;
                }

                template <typename T>
                inline bool
                compare_val_to_optional(
                      const T&                          val
                    , const boost::optional<T>&         cmp
                )
                {
                    if( cmp && cmp.get() == val ) 
                        return true ;

                    return false ;
                }


                bool
                on_expose_event (GdkEventExpose *event)
                {
                    const Gtk::Allocation& a = get_allocation();

                    boost::shared_ptr<IYoukiThemeEngine> theme = services->get<IYoukiThemeEngine>("mpx-service-theme") ;

                    const ThemeColor& c_text        = theme->get_color( THEME_COLOR_TEXT ) ;
                    const ThemeColor& c_text_sel    = theme->get_color( THEME_COLOR_TEXT_SELECTED ) ;
                    const ThemeColor& c_rules_hint  = theme->get_color( THEME_COLOR_BASE_ALTERNATE ) ;

                    Cairo::RefPtr<Cairo::Context> cairo = get_window()->create_cairo_context(); 

                    cairo->rectangle(
                          0
                        , 0
                        , a.get_width()
                        , a.get_height()
                    ) ;
    
                    cairo->clip() ;

                    cairo->set_operator( Cairo::OPERATOR_OVER ) ;

                    std::size_t row = get_upper_row() ;
                    std::size_t idx = 0 ;

                    int col     = 0 ;
                    int cnt     = get_page_size() + 1 ; 

                    int xpos    = 0 ;
                    int ypos    = m_row_start ;

                    const int inner_pad = 1 ;

                    if( event->area.y <= m_row_start )
                    {
                            xpos = 32 ;

                            for( Columns::iterator i = m_columns.begin(); i != m_columns.end(); ++i, ++col )
                            {
                                (*i)->render_header(
                                    cairo
                                  , *this
                                  , xpos
                                  , 0
                                  , m_row_start
                                  , col
                                  , c_text
                                ) ;

                                xpos += (*i)->get_width() ; 
                            }
                    }

/*
                    if( m_model->size() == 0 )
                        return true ;
*/

                    cairo->set_operator(Cairo::OPERATOR_OVER);

                    //// ROWS

                    if( event->area.width > 32 )
                    {
                            while( m_model->is_set() && cnt && m_Model_I.in( row ) ) 
                            {
                                if( !(idx % 2) ) 
                                {
                                    GdkRectangle r ;

                                    r.x       = 32 + inner_pad ;
                                    r.y       = ypos + inner_pad ;
                                    r.width   = a.get_width() - 2 * inner_pad - 32 ;
                                    r.height  = m_row_height - 2 * inner_pad ;

                                    RoundedRectangle(
                                          cairo
                                        , r.x
                                        , r.y
                                        , r.width
                                        , r.height
                                        , rounding
                                    ) ;

                                    cairo->set_source_rgba(
                                          c_rules_hint.r 
                                        , c_rules_hint.g
                                        , c_rules_hint.b 
                                        , c_rules_hint.a
                                    ) ;

                                    cairo->fill() ;
                                }

                                ypos += m_row_height ;
                                row  ++ ;
                                cnt  -- ;
                                idx  ++ ;
                            }

                            boost::optional<std::size_t> row_select ;

                            if( m_selection )
                            {
                                Interval<std::size_t> i (
                                      Interval<std::size_t>::IN_IN
                                    , get_upper_row()
                                    , get_upper_row() + get_page_size()
                                ) ;

                                std::size_t row = m_selection.get() ; 
                                row_select = row ;

                                if( i.in( row ) )
                                {
                                        GdkRectangle r ;

                                        r.x         = 32 + inner_pad ; 
                                        r.y         = inner_pad + (row - get_upper_row()) * m_row_height + m_row_start ;
                                        r.width     = a.get_width() - 2*inner_pad - 32 ; 
                                        r.height    = m_row_height  - 2*inner_pad ;

                                        theme->draw_selection_rectangle(
                                              cairo
                                            , r
                                            , has_focus()
                                        ) ;
                                    }
                            }

                            row     = get_upper_row() ;

                            col     = 0 ;
                            cnt     = get_page_size() + 1 ; 

                            ypos    = m_row_start ;

                            bool currently_playing = bool( m_id_currently_playing ) ;

                            while( m_model->is_set() && cnt && m_Model_I.in( row ) ) 
                            {
                                xpos = 32 ;

                                const Row_t& r_data = m_model->row( row ) ;

                                if( currently_playing ) 
                                {
                                    double alpha = 0.50 ;

                                    if( boost::get<8>(r_data) == m_id_currently_playing.get() ) 
                                    {
                                        alpha = 1.0 ;
                                    }

                                    for(Columns::const_iterator i = m_columns.begin(); i != m_columns.end(); ++i)
                                    {
                                        (*i)->render(
                                              cairo
                                            , *this
                                            , r_data 
                                            , row
                                            , xpos
                                            , ypos
                                            , m_row_height
                                            , (row_select && (row == row_select.get())) ? c_text_sel : c_text
                                            , alpha 
                                        ) ;

                                        xpos += (*i)->get_width() ; 
                                    }
                                }
                                else
                                {
                                    for(Columns::const_iterator i = m_columns.begin(); i != m_columns.end(); ++i)
                                    {
                                        (*i)->render(
                                              cairo
                                            , *this
                                            , r_data 
                                            , row
                                            , xpos
                                            , ypos
                                            , m_row_height
                                            , (row_select && (row == row_select.get())) ? c_text_sel : c_text
                                            , 1.0
                                        ) ;

                                        xpos += (*i)->get_width() ; 
                                    }
                                }

                                ypos += m_row_height ;
                                row  ++ ;
                                cnt  -- ;
                            }

                            if( true /*m_render_dashes*/)
                            {
                                std::valarray<double> dashes ( 3 ) ;
                                dashes[0] = 0. ;
                                dashes[1] = 3. ;
                                dashes[2] = 0. ;

                                xpos = 32 ;

                                cairo->save() ;

                                const ThemeColor& c_treelines = theme->get_color( THEME_COLOR_TREELINES ) ;

                                cairo->set_line_width(
                                      .3
                                ) ;
                                cairo->set_dash(
                                      dashes
                                    , 0
                                ) ;

                                Columns::iterator i2 = m_columns.end() ;
                                --i2 ;

                                for( Columns::const_iterator i = m_columns.begin(); i != i2; ++i )
                                {
                                    xpos += (*i)->get_width() ; 

                                    cairo->set_source_rgba(
                                          c_treelines.r
                                        , c_treelines.g
                                        , c_treelines.b
                                        , c_treelines.a
                                    ) ;

                                    cairo->move_to(
                                          xpos
                                        , 0 
                                    ) ; 

                                    cairo->line_to(
                                          xpos
                                        , a.get_height()
                                    ) ;

                                    cairo->stroke() ;
                                }

                                cairo->restore(); 
                            }
                    }
    
                    //// ICONS

                    const int icon_lateral = 16 ;

                    ypos    = m_row_start ;
                    cnt     = get_page_size() + 1 ; 
                    row     = get_upper_row() ;

                    while( m_model->is_set() && cnt && m_Model_I.in( row ) )
                    {
                        enum Skip
                        {
                              SKIP_SKIP_NONE
                            , SKIP_SKIP_PLAY        = 1 << 0
                            , SKIP_SKIP_TERMINAL    = 1 << 1
                        } ;

                        enum Icon
                        {
                              ICON_PLAY
                            , ICON_TERMINAL
                        } ;

                        const int icon_y = ypos + (m_row_height - icon_lateral) / 2 ;

                        const int icon_x[2] = { 16, 0 } ;

                        if( m_id_currently_playing && boost::get<8>(m_model->row(row)) == m_id_currently_playing.get() ) 
                        {
                            Gdk::Cairo::set_source_pixbuf(
                                  cairo
                                , m_pb_play_l
                                , icon_x[ICON_PLAY]
                                , icon_y 
                            ) ;
                            cairo->rectangle(
                                  icon_x[ICON_PLAY]
                                , icon_y 
                                , icon_lateral
                                , icon_lateral
                            ) ;
                            cairo->fill () ;

                            // skip &= SKIP_SKIP_PLAY ;
                        }

                        else if( compare_val_to_optional<int64_t>( row, m_hover_track )) 
                        {
                            if( !m_id_currently_playing || m_id_currently_playing.get() != m_hover_track.get() )
                            {
                             //   if( !( skip & SKIP_SKIP_PLAY))
                                {
                                    Gdk::Cairo::set_source_pixbuf(
                                          cairo
                                        , m_pb_hover_l
                                        , icon_x[ICON_PLAY]
                                        , icon_y 
                                    ) ;
                                    cairo->rectangle(
                                          icon_x[ICON_PLAY] 
                                        , icon_y 
                                        , icon_lateral
                                        , icon_lateral
                                    ) ;
                                    cairo->fill () ;
                                }
                             }
                        }

                        ypos += m_row_height ;
                        row  ++ ;
                        cnt  -- ;
                    }

                    return true;
                }

                void
                on_model_changed(
                )
                {
                    m_Model_I = Interval<std::size_t> (
                             Interval<std::size_t>::IN_EX
                            , 0
                            , m_model->size()
                    ) ;

                    configure_vadj(
                          m_model->size()
                        , get_page_size()
                        , 1
                    ) ;

                    m_selection.reset() ;

                    update_actions() ;
                    queue_draw() ;
                }

                static gboolean
                list_view_set_adjustments(
                    GtkWidget*obj,
                    GtkAdjustment*hadj,
                    GtkAdjustment*vadj, 
                    gpointer data
                )
                {
                    if( vadj )
                    {
                            g_object_set(G_OBJECT(obj), "vadjustment", vadj, NULL); 
                            g_object_set(G_OBJECT(obj), "hadjustment", hadj, NULL);

                            Class & view = *(reinterpret_cast<Class*>(data));

                            view.m_prop_vadj.get_value()->signal_value_changed().connect(
                                sigc::mem_fun(
                                    view,
                                    &Class::on_vadj_value_changed
                            ));
                    }

                    return TRUE;
                }

                bool
                query_tooltip(
                      int                                   tooltip_x
                    , int                                   tooltip_y
                    , bool                                  keypress
                    , const Glib::RefPtr<Gtk::Tooltip>&     tooltip
                )
                {
                    std::size_t row = (double( tooltip_y ) - m_row_start) / double(m_row_height) ;

                    MPX::Track_sp t = boost::get<4>( m_model->row(row) ) ;
                    const MPX::Track& track = *(t.get()) ;

                    boost::shared_ptr<Covers> covers = services->get<Covers>("mpx-service-covers") ;
                    Glib::RefPtr<Gdk::Pixbuf> cover ;

                    const std::string& mbid = boost::get<std::string>( track[ATTRIBUTE_MB_ALBUM_ID].get() ) ;

                    Gtk::Image * image = Gtk::manage( new Gtk::Image ) ;

                    if( covers->fetch(
                          mbid
                        , cover
                    ))
                    {   
                        image->set( cover ) ;
                        tooltip->set_custom( *image ) ;
                        return true ;
                    }

                    return false ;
                }

            public:
   
                inline std::size_t
                get_page_size(
                )
                {
                    if( m_visible_height && m_row_height )
                        return m_visible_height / m_row_height ; 
                    else
                        return 0 ;
                }

                inline std::size_t
                get_upper_row(
                )
                {
                    if( m_prop_vadj.get_value() )
                        return m_prop_vadj.get_value()->get_value() ;
                    else
                        return 0 ;
                }

                inline std::size_t
                get_lower_row(
                )
                {
                    return m_prop_vadj.get_value()->get_value() + get_page_size() ;
                }

                inline bool
                get_row_is_visible(
                      std::size_t   row
                )
                {
                    std::size_t up = get_upper_row() ;

                    Interval<std::size_t> i (
                          Interval<std::size_t>::IN_IN
                        , up 
                        , up + get_page_size()
                    ) ;
            
                    return i.in( row ) ;
                }

                void
                set_highlight(bool highlight)
                {
                    m_highlight = highlight;
                    queue_draw ();
                }

                void
                set_model(MPX::View::Playlist::DataModel_sp_t& model)
                {
                    m_model = model;

                    m_model->signal_changed().connect(
                        sigc::mem_fun(
                            *this,
                            &Class::on_model_changed
                    ));

                    on_model_changed() ;
                }

                void
                append_column(
                      Column_sp_t   column
                )
                {
                    m_columns.push_back(column) ;
                }

                SignalTrackActivated&
                signal_track_activated()
                {
                    return m_SIGNAL_track_activated ;
                }

                SignalVAdjChanged&
                signal_vadj_changed()
                {
                    return m_SIGNAL_vadj_changed ;
                }

                SignalFindAccepted&
                signal_find_accepted()
                {
                    return m_SIGNAL_find_accepted ;
                }

                SignalFindPropagate&
                signal_find_propagate()
                {
                    return m_SIGNAL_find_propagate ;
                }

                SignalRowMoved &
                signal_row_moved()
                {
                    return m_SIGNAL_row_moved ;
                }

                void
                clear_active_track()
                {
                    m_id_currently_playing.reset() ;
                    m_row_currently_playing.reset() ;
                    queue_draw() ;
                }

            protected:

                void
                scan_active_row()
                {
                    for(Model_t::iterator i = m_model->m_realmodel->begin() ; i != m_model->m_realmodel->end() ; ++i )
                    {
                        if( boost::get<8>(*i) == m_id_currently_playing.get() ) 
                        {
                            m_row_currently_playing = std::distance( m_model->m_realmodel->begin(), i ) ;
                            return ;
                        }
                    }
                }

            public:

                void
                set_active_track(int64_t id)
                {
                    m_id_currently_playing = id ;
                    scan_active_row() ;
                    queue_draw () ;
                }

                boost::optional<int64_t>
                get_active_track()
                {
                    return m_id_currently_playing ; 
                }

                boost::optional<int64_t>
                get_active_row()
                {
                    return m_row_currently_playing ;
                }

                boost::optional<std::size_t>
                get_selected_track()
                {
                    return m_selection ; 
                }

                void
                column_set_collapsed(
                      int       column
                    , bool      collapsed
                )
                {
                    if( collapsed )
                    {
                        m_collapsed.insert( column ) ;
                        queue_resize () ;
                    }
                    else
                    {
                        m_collapsed.erase( column ) ;
                        queue_resize () ;
                    }
                }

                void
                column_set_fixed(
                      int       column
                    , bool      fixed
                    , int       width = 0
                )
                {
                    if( fixed )
                    {
                        m_fixed.insert( column ) ;
                        m_fixed_total_width += width ;
                        m_columns[column]->set_width( width ) ;
                        queue_resize () ;
                    }
                    else
                    {
                        m_fixed.erase( column ) ;
                        m_fixed_total_width -= m_columns[column]->get_width() ; 
                        queue_resize () ;
                    }
                }

                void
                scroll_to_row(
                      std::size_t row
                )
                {
                    if( m_visible_height && m_row_height && m_prop_vadj.get_value() && m_model )
                    {
                        Limiter<std::size_t> d ( 
                              Limiter<std::size_t>::ABS_ABS
                            , 0
                            , m_model->size() - get_page_size()
                            , row 
                        ) ;

                        if( m_model->size() < get_page_size()) 
                            m_prop_vadj.get_value()->set_value( 0 ) ; 
                        else
                        if( row > (m_model->size() - get_page_size()) )
                            m_prop_vadj.get_value()->set_value( m_model->size() - get_page_size() ) ; 
                        else
                            m_prop_vadj.get_value()->set_value( d ) ; 

                        m_hover_track.reset() ;
                    }
                }

                void
                select_row(
                      std::size_t row
                )
                {
                    Interval<std::size_t> i (
                          Interval<std::size_t>::IN_EX
                        , 0
                        , m_model->size()
                    ) ;

                    if( i.in( row ))
                    {
                        m_selection = row ;
                    }
                    else
                    {
                        m_selection.reset() ;
                    }

                    update_actions() ;
                    queue_draw() ;
                }

                void
                clear_selection(
                )
                {
                    m_selection.reset() ;
                    queue_draw() ;
                }

            protected:

                void
                find_next_match()
                {
                    using boost::get ;

                    Glib::ustring text = m_SearchEntry->get_text().casefold() ;

                    if( text.empty() )
                    {
                        return ;
                    }

                    Model_t::iterator i = m_model->m_realmodel->begin(); 

                    if( m_selection )
                    {
                        std::advance( i, m_selection.get() ) ;
                        ++i ;
                    }

                    for( ; i != m_model->m_realmodel->end(); ++i )
                    {
                        Glib::ustring match = Glib::ustring(get<0>(*i)).casefold() ;

                        if( match.length() && match.substr( 0, text.length()) == text.substr( 0, text.length()) )
                        {
                            std::size_t d = std::distance( m_model->m_realmodel->begin(), i ) ; 
                            scroll_to_row( d ) ;
                            select_row( d ) ;
                            return ;
                        }
                    }
                }

                void
                find_prev_match()
                {
                    using boost::get ;

                    Glib::ustring text = m_SearchEntry->get_text().casefold() ;

                    if( text.empty() )
                    {
                        return ;
                    }

                    Model_t::iterator i = m_model->m_realmodel->begin(); 

                    if( m_selection )
                    {
                        std::advance( i, m_selection.get()) ;
                        --i ; 
                    }

                    for( ; i >= m_model->m_realmodel->begin(); --i )
                    {
                        Glib::ustring match = Glib::ustring(get<0>(*i)).casefold() ;

                        if( match.length() && match.substr( 0, text.length()) == text.substr( 0, text.length()) )
                        {
                            std::size_t d = std::distance( m_model->m_realmodel->begin(), i ) ; 
                            scroll_to_row( d ) ;
                            select_row( d ) ;
                            return ;
                        }
                    }
                }

                void
                on_search_entry_changed()
                {
                    using boost::get ;

                    Glib::ustring text = m_SearchEntry->get_text().casefold() ;

                    if( text.empty() )
                    {
                        return ;
                    }

                    Model_t::iterator i = m_model->m_realmodel->begin(); 
               
                    for( ; i != m_model->m_realmodel->end(); ++i )
                    {
                        Glib::ustring match = Glib::ustring(get<0>(*i)).casefold() ;

                        if( match.length() && match.substr( 0, text.length()) == text.substr( 0, text.length()) )
                        {
                            std::size_t d = std::distance( m_model->m_realmodel->begin(), i ) ; 
                            scroll_to_row( d ) ; 
                            select_row( d ) ;
                            return ;
                        }
                    }

                    clear_selection() ;
                    scroll_to_row( 0 ) ;
                }

                void
                on_search_entry_activated()
                {
                    cancel_search() ;

                    m_SIGNAL_find_accepted.emit() ;
                }

                bool
                on_search_window_focus_out(
                      GdkEventFocus* G_GNUC_UNUSED
                )
                {
                    cancel_search() ;
                    return false ;
                }

                void
                on_search_button_clicked(
                )
                {
                    std::string text = m_SearchEntry->get_text() ;
                    cancel_search() ;

                    m_SIGNAL_find_propagate.emit( text ) ;
                }

                void
                on_clear()
                {
                    clear_active_track() ;
                    clear_selection() ;
                    m_model->clear() ;
                    _signal_0.emit() ;
                    scroll_to_row(0) ;
                }
                void
                on_delete_selected()
                {
                    if( m_selection )
                    {
                        int64_t sel_pl_id = boost::get<8>(m_model->row(m_selection.get())) ;
                        boost::optional<int64_t> active_id = get_active_track() ;

                        if( active_id && active_id.get() == sel_pl_id ) 
                        {
                            clear_active_track() ;
                        }

                        clear_selection() ;
                        queue_draw() ;

                        _signal_1.emit( sel_pl_id ) ;
                    }
                }

            public:

                void
                cancel_search()
                {
                    if( !m_search_active )
                        return ;

                    send_focus_change( *m_SearchEntry, false ) ;

                    m_SearchWindow->hide() ;
                    m_search_changed_conn.block () ;
                    m_SearchEntry->set_text("") ;
                    m_search_changed_conn.unblock () ;
                    m_search_active = false ;
                }

            protected:

                virtual void
                on_realize()
                {
                    Gtk::DrawingArea::on_realize() ;
                    initialize_metrics();
                    queue_resize();
                }

            public:

                SignalClear&
                signal_clear()
                {
                    return _signal_0 ;
                }

                SignalDelPos&
                signal_song_removed()
                {
                    return _signal_1 ;
                }

                Class ( Glib::RefPtr<Gtk::UIManager> ui_man )

                        : ObjectBase( "YoukiClassPlaylist" )
                        , m_prop_vadj( *this, "vadjustment", (Gtk::Adjustment*)( 0 ))
                        , m_prop_hadj( *this, "hadjustment", (Gtk::Adjustment*)( 0 ))
                        , m_clicked_row( 0 ) 
                        , m_clicked( false )
                        , m_highlight( false )
                        , m_fixed_total_width( 0 )
                        , m_search_active( false )

                {
                    boost::shared_ptr<IYoukiThemeEngine> theme = services->get<IYoukiThemeEngine>("mpx-service-theme") ;
                    const ThemeColor& c = theme->get_color( THEME_COLOR_BASE ) ;
                    Gdk::Color cgdk ;
                    cgdk.set_rgb_p( c.r, c.g, c.b ) ; 
                    modify_bg( Gtk::STATE_NORMAL, cgdk ) ;
                    modify_base( Gtk::STATE_NORMAL, cgdk ) ;

                    m_pb_play_l  = Gdk::Pixbuf::create_from_file( Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "row-play.png" )) ;
                    m_pb_hover_l = Gdk::Pixbuf::create_from_file( Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "row-hover.png" )) ;

                    set_flags(Gtk::CAN_FOCUS);
                    add_events(Gdk::EventMask(GDK_KEY_PRESS_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK ));

                    ((GtkWidgetClass*)(G_OBJECT_GET_CLASS(G_OBJECT(gobj()))))->set_scroll_adjustments_signal = 
                            g_signal_new ("set_scroll_adjustments",
                                      G_OBJECT_CLASS_TYPE (G_OBJECT_CLASS (G_OBJECT_GET_CLASS(G_OBJECT(gobj())))),
                                      GSignalFlags (G_SIGNAL_RUN_FIRST),
                                      0,
                                      NULL, NULL,
                                      g_cclosure_user_marshal_VOID__OBJECT_OBJECT, G_TYPE_NONE, 2, GTK_TYPE_ADJUSTMENT, GTK_TYPE_ADJUSTMENT);

                    g_signal_connect(G_OBJECT(gobj()), "set_scroll_adjustments", G_CALLBACK(list_view_set_adjustments), this);

                    /*
                    signal_query_tooltip().connect(
                        sigc::mem_fun(
                              *this
                            , &Class::query_tooltip
                    )) ;

                    set_has_tooltip( true ) ;
                    */

                    m_SearchEntry = Gtk::manage( new Gtk::Entry ) ;
                    //gtk_widget_realize( GTK_WIDGET(m_SearchEntry->gobj()) ) ;
                    m_SearchEntry->show() ;

                    m_SearchHBox = Gtk::manage( new Gtk::HBox ) ;
                    m_SearchButton = Gtk::manage( new Gtk::Button ) ;
                    m_SearchButton->signal_clicked().connect(
                        sigc::mem_fun(
                              *this
                            , &Class::on_search_button_clicked
                    )) ;

                    Gtk::Image * img = Gtk::manage( new Gtk::Image ) ;
                    img->set( Gtk::Stock::FIND, Gtk::ICON_SIZE_MENU ) ;
                    img->show() ;
    
                    m_SearchButton->set_image( *img ) ;

                    m_search_changed_conn = m_SearchEntry->signal_changed().connect(
                            sigc::mem_fun(
                                  *this
                                , &Class::on_search_entry_changed
                    )) ;
    
                    m_SearchEntry->signal_activate().connect(
                            sigc::mem_fun(
                                  *this
                                , &Class::on_search_entry_activated
                    )) ;

                    m_SearchWindow = new Gtk::Window( Gtk::WINDOW_POPUP ) ;
                    m_SearchWindow->set_decorated( false ) ;

                    m_SearchWindow->signal_focus_out_event().connect(
                            sigc::mem_fun(
                                  *this
                                , &Class::on_search_window_focus_out
                    )) ;

                    signal_focus_out_event().connect(
                            sigc::mem_fun(
                                  *this
                                , &Class::on_search_window_focus_out
                    )) ;

                    signal_key_press_event().connect(
                            sigc::mem_fun(
                                  *this
                                , &Class::key_press_event
                    ), true ) ;

                    m_SearchHBox->pack_start( *m_SearchEntry, true, true, 0 ) ;
                    m_SearchHBox->pack_start( *m_SearchButton, true, true, 0 ) ;

                    m_SearchWindow->add( *m_SearchHBox ) ;
                    m_SearchHBox->show_all() ;

                    property_can_focus() = true ;

                    m_refActionGroup = Gtk::ActionGroup::create() ;

                    m_Action_Clear_Playlist = Gtk::Action::create(
                            "ContextClear"
                          , "Clear Playlist") ;

                    m_refActionGroup->add( m_Action_Clear_Playlist,
                        Gtk::AccelKey("<ctrl>d","<PopupMenu>/ContextClear"),
                            sigc::mem_fun(*this, &Class::on_clear )) ;

                    m_Action_Remove_Selected = Gtk::Action::create( 
                            "ContextDelSelected"
                          , "Remove Selected") ;
    
                    m_refActionGroup->add( m_Action_Remove_Selected,
                        Gtk::AccelKey("Delete", "<PopupMenu>/ContextDelSelected"),
                            sigc::mem_fun(*this, &Class::on_delete_selected )) ;

                    ui_man->insert_action_group( m_refActionGroup ) ;

                    std::string ui_info =
                    "<ui>"
                    "   <popup name='PopupMenu'>"
                    "       <menuitem action='ContextDelSelected'/>"
                    "       <menuitem action='ContextClear'/>"
                    "   </popup>"
                    "</ui>" ;

                    ui_man->add_ui_from_string( ui_info ) ;
                    m_pMenuPopup = dynamic_cast<Gtk::Menu*>(ui_man->get_widget("/PopupMenu")) ;
                }

                virtual ~Class ()
                {
                }

            private:

                void
                update_actions()
                {
                    m_Action_Remove_Selected->set_sensitive(bool(m_selection)) ; 
                    m_Action_Clear_Playlist->set_sensitive(bool(!m_model->m_realmodel->empty())) ;
                }
        };
}
}
}

#endif // _YOUKI_TRACK_LIST_HH
