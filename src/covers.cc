//  MPX
//  Copyright (C) 2010 MPX development.
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
//  The MPX project hereby grants permission for non-GPL compatible GStreamer
//  plugins to be used and distributed together with GStreamer and MPX. This
//  permission is above and beyond the permissions granted by the GPL license
//  MPX is covered by.

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <gtkmm.h>
#include <glibmm.h>
#include <giomm.h>
#include <glibmm/i18n.h>
#include <glib/gstdio.h>
#include <boost/ref.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <unistd.h>
#include <sys/stat.h>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include "mpx/mpx-covers.hh"
#include "mpx/mpx-covers-stores.hh"
#include "mpx/mpx-main.hh"
#include "mpx/mpx-minisoup.hh"
#include "mpx/mpx-network.hh"
#include "mpx/mpx-uri.hh"
#include "mpx/util-file.hh"
#include "mpx/util-graphics.hh"
#include "mpx/util-string.hh"
#include "mpx/xml/xml.hh"
#include "mpx/algorithm/ld.hh"

using namespace Glib;

namespace
{
  using namespace MPX;

  void
  create_or_lock( CoverFetchContext* data, MutexMap & mmap )
  {
    if( mmap.find( data->qualifier.mbid ) == mmap.end() )
    {
        MutexPtr p (new Glib::RecMutex );
        p->lock();
        mmap.insert(std::make_pair( data->qualifier.mbid, p ));
    }
    else
    {
        mmap[data->qualifier.mbid]->lock();
    }
  }

  void
  create_or_unlock( CoverFetchContext* data, MutexMap & mmap )
  {
    if( mmap.find( data->qualifier.mbid ) == mmap.end() )
    {
        MutexPtr p (new Glib::RecMutex );
        mmap.insert(std::make_pair( data->qualifier.mbid, p ));
    }
    else
    {
        mmap[data->qualifier.mbid]->unlock();
    }
  }
}

namespace MPX
{
    Covers::Covers ()

        : Service::Base("mpx-service-covers")

        , m_rebuild(0)
        , m_rebuilt(0)

    {
        Glib::ScopedPtr<char> path (g_build_filename(g_get_user_cache_dir(), PACKAGE, "covers", NULL));
        g_mkdir(path.get(), 0700);

        m_stores_all.push_back(StorePtr(new MusicBrainzCovers(*this)));
        m_stores_all.push_back(StorePtr(new AmapiCovers(*this)));

        rebuild_stores();

        mcs->subscribe(
                "Preferences-CoverArtSources",
                "SourceActive1",
                sigc::mem_fun(
                    *this,
                    &Covers::source_pref_changed_callback
        ));

        m_default_cover = Gdk::Pixbuf::create_from_file( Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "disc.png" )) ;
    }

    void
    Covers::source_pref_changed_callback(
          const std::string&        domain
        , const std::string&        key
        , const Mcs::KeyVariant&    value
    )
    {
        g_atomic_int_set(&m_rebuilt, 0);
        g_atomic_int_set(&m_rebuild, 1);
    }

    void
    Covers::rebuild_stores()
    {
        m_stores_cur = StoresVec (m_stores_all.size(), StorePtr());

        int at0 = mcs->key_get<int>("Preferences-CoverArtSources", "Source0");
        int at1 = mcs->key_get<int>("Preferences-CoverArtSources", "Source1");

        bool use0 = mcs->key_get<bool>("Preferences-CoverArtSources", "SourceActive0");
        bool use1 = mcs->key_get<bool>("Preferences-CoverArtSources", "SourceActive1");

        if (use0)
        {
            m_stores_cur[0] = m_stores_all[at0];
        }

        if (use1)
        {
            m_stores_cur[1] = m_stores_all[at1];
        }
    }

    void
    Covers::cache_artwork(
        const std::string& mbid,
        RefPtr<Gdk::Pixbuf> cover
    )
    {
        g_message("Saving cover as: '%s'", get_thumb_path( mbid ).c_str()) ;
        cover->save( get_thumb_path( mbid ), "png" );
        m_pixbuf_cache[mbid] = cover;
    }

    std::string
    Covers::get_thumb_path(
        std::string mbid
    )
    {
        using boost::algorithm::replace_all;
        replace_all(mbid, "/","_");
        std::string basename = (boost::format ("%s.png") % mbid).str ();
        Glib::ScopedPtr<char> path (g_build_filename(g_get_user_cache_dir(), PACKAGE, "covers", basename.c_str(), NULL));
        return std::string(path.get());
    }

    bool
    Covers::exists(
          const RequestQualifier& qual
    )
    {
        std::string thumb_path = get_thumb_path( qual.mbid ) ;

        if( file_test( thumb_path, FILE_TEST_EXISTS ))
        {
            return true ; 
        }

        return false ;
    }

    bool
    Covers::cache(
          const RequestQualifier& qual
        , bool                    acquire
    )
    {
        if( g_atomic_int_get(&m_rebuild) )
        {
            rebuild_stores();
            g_atomic_int_set(&m_rebuild, 0);
            g_atomic_int_set(&m_rebuilt, 1);
        }

        std::string thumb_path = get_thumb_path( qual.mbid ) ;

        if( file_test( thumb_path, FILE_TEST_EXISTS ))
        {
            signal_got_cover.emit( qual.id ) ;
            return false ; 
        }

        if( acquire && m_stores_cur.size() )
        {
            std::size_t i = 0 ;

            CoverFetchContext * data = new CoverFetchContext( qual, m_stores_cur );

            while( i < data->stores.size() )
            {
                while( !data->stores[i] )
                {
                    i++;
                }
            
                if( i < data->stores.size() )
                { 
                    //create_or_lock( data, m_mutexes ) ;
                    StorePtr store = data->stores[i] ; 
                    store->load_artwork( data ) ;

                    if( store->get_state() == FETCH_STATE_COVER_SAVED )
                    {
                        signal_got_cover.emit( qual.id ) ;
                        //create_or_unlock( data, m_mutexes );
                        delete data ;
                        return false ;
                    }
                    else
                        g_message("[%d]: No Cover!", i) ;
                }

                ++i ;
            }

            delete data;
        }

        return false ;
    }

/*
    bool
    Covers::handle_precache(
        const MPX::Library* library
    )
    {
        Glib::ScopedPtr<char> path (g_build_filename(g_get_user_cache_dir(), PACKAGE, "covers", NULL));

        SQL::RowV v;

        library->getSQL( v, "SELECT DISTINCT mb_album_id FROM album" );

        for( SQL::RowV::iterator i = v.begin(); i != v.end(); ++i )
        {
            try{
                const std::string& mbid = boost::get<std::string>((*i)["mb_album_id"]);
                const std::string& cpth = build_filename( path.get(), mbid) + ".png";

                if( Glib::file_test( cpth, Glib::FILE_TEST_EXISTS )) 
                {
                    Glib::RefPtr<Gdk::Pixbuf> cover = Gdk::Pixbuf::create_from_file( cpth ); 
                    m_pixbuf_cache.insert(std::make_pair( mbid, cover ));
                }
            }
            catch( Gdk::PixbufError )
            {
            }
            catch( Glib::FileError )
            {  
            }
        }

        return false ;
    }
*/

    bool
    Covers::fetch(
          RefPtr<Gdk::Pixbuf>&      cover
        , int                       size
    )
    {
        if( size == -1 )
            cover = m_default_cover ;
        else
            cover = m_default_cover->scale_simple( size, size, Gdk::INTERP_BILINEAR ) ;

        return true ;
    }

    bool
    Covers::fetch(
          const std::string&        mbid
        , RefPtr<Gdk::Pixbuf>&      cover
        , int                       size
    )
    {
        return fetch_back1( mbid, boost::ref(cover), size ) ;
    }

    void
    Covers::purge()
    {
        Glib::ScopedPtr<char> path (g_build_filename(g_get_user_cache_dir(), PACKAGE, "covers", NULL));

        Glib::Dir dir (path.get());
        StrV v (dir.begin(), dir.end());
        dir.close ();

        for(StrV::const_iterator i = v.begin(); i != v.end(); ++i)
        {
            std::string fullpath = build_filename(path.get(), *i);
            g_unlink(fullpath.c_str());
        }
    }

    ////

    bool
    Covers::fetch_back1(
          const std::string&      mbid_
        , RefPtr<Gdk::Pixbuf>&    cover
        , int                     size
    )
    {
        std::string mbid ;

        if( size != -1 )
        {
            mbid = (boost::format("%s-SIZE%d") % mbid_ % size ).str() ;
        }
        else
        {
            mbid = mbid_ ;
        }

        PixbufCache::const_iterator i = m_pixbuf_cache.find( mbid );

        if (i != m_pixbuf_cache.end())
        {
            cover = (*i).second; 
            return true;
        }

        std::string thumb_path = get_thumb_path( mbid ) ;

        if( file_test( thumb_path, FILE_TEST_EXISTS ))
        {
            try{
              cover = Gdk::Pixbuf::create_from_file( thumb_path ) ; 
              m_pixbuf_cache.insert( std::make_pair( mbid, cover )) ;
              return true;
            }
            catch( Gdk::PixbufError )
            {
            }
            catch( Glib::FileError )
            {
            }
        }
        else 
        if( size != -1 )
        {
            std::string thumb_path = get_thumb_path( mbid_ ) ;

            if( file_test( thumb_path, FILE_TEST_EXISTS ))
            {
                    try{
                      cover = Gdk::Pixbuf::create_from_file( thumb_path )->scale_simple( size, size, Gdk::INTERP_BILINEAR ) ; 
                      cover->save( get_thumb_path( mbid ), "png" );
                      m_pixbuf_cache.insert( std::make_pair( mbid, cover )) ;
                      return true;
                    }
                    catch( Gdk::PixbufError )
                    {
                    }
                    catch( Glib::FileError )
                    {
                    }
            }
        }

        return false;
    }
}
