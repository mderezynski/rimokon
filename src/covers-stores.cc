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

#include <gtkmm.h>
#include <glibmm.h>
#include <giomm.h>

#include <string>
#include <cstring>

#include <boost/format.hpp>

#include "mpx/mpx-covers.hh"
#include "mpx/mpx-covers-stores.hh"

#include "mpx/algorithm/ld.hh"
#include "mpx/algorithm/base64.h"

#include "mpx/mpx-uri.hh"

#include "mpx/util-string.hh"
#include "mpx/util-string.hh"

#include "mpx/xml/xml.hh"

namespace
{
    static boost::format amazon_f[] =
    {
        boost::format("http://images.amazon.com/images/P/%s.01.LZZZZZZZ.jpg"),
        boost::format("http://images-eu.amazon.com/images/P/%s.01.LZZZZZZZ.jpg"),
        boost::format("http://ec1.images-amazon.com/images/P/%s.01.LZZZZZZZ.jpg"),
        boost::format("http://images.amazon.com/images/P/%s.01.MZZZZZZZ.jpg"),
        boost::format("http://images-eu.amazon.com/images/P/%s.01.MZZZZZZZ.jpg"),
        boost::format("http://ec1.images-amazon.com/images/P/%s.01.MZZZZZZZ.jpg"),
    };

    static boost::format mbxml_f ("http://www.uk.musicbrainz.org/ws/1/release/%s?type=xml&inc=url-rels");

    static boost::format amapi2_f ("http://free.apisigning.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId=1E90RVC80K4MVNTCHG02&Operation=ItemSearch&Artist=%s&Title=%s&SearchIndex=Music&ResponseGroup=Images") ;
}

using namespace Glib;

namespace MPX
{
    // --------------------------------------------------
    //
    // Remote store (still an abstract class, but provides a callback)
    //
    // --------------------------------------------------
    void
    RemoteStore::load_artwork(CoverFetchContext* ctx)
    {
        if( can_load_artwork( ctx ) )
        {
            fetch_image( get_url( ctx ), ctx ) ;
        }
    }

    void
    RemoteStore::fetch_image(const std::string& url, CoverFetchContext* ctx)
    {
        if( can_load_artwork( ctx ) )
        {
            g_message( url.c_str() ) ;

            request = Soup::RequestSync::create ( url ) ; 
            guint code = request->run() ;

            if( code == 200 )
            {
                save_image(
                      request->get_data_raw()
                    , request->get_data_size()
                    , ctx
                ) ;
            }
            else
            {
                request_failed( ctx );
            }
        }
    }

    bool
    RemoteStore::can_load_artwork(CoverFetchContext* ctx)
    {
        return true;
    }

    void
    RemoteStore::save_image(
          char const*        data
        , guint              size
        , CoverFetchContext* ctx
    )
    {
        RefPtr<Gdk::PixbufLoader> loader = Gdk::PixbufLoader::create ();
        loader->write (reinterpret_cast<const guint8*>( data ), size);
        loader->close ();

        RefPtr<Gdk::Pixbuf> cover = loader->get_pixbuf();

        g_message(G_STRLOC) ;

        if( cover->get_width() == 1 && cover->get_height() == 1 )
        {
            g_message(G_STRLOC) ;

            request_failed( ctx );
            return;
        }
        else
        { 
            g_message(G_STRLOC) ;

            try{
                    covers.cache_artwork( ctx->qualifier.mbid, cover );
                    m_fetch_state = FETCH_STATE_COVER_SAVED;
            } catch( Glib::FileError & cxe )
            {
                    g_message("File Error while trying to save cover image: %s", cxe.what().c_str());
                    request_failed( ctx ) ;
            }
        }
    }
    
    void
    RemoteStore::request_failed( CoverFetchContext *ctx )
    {
        m_fetch_state = FETCH_STATE_NOT_FETCHED ;
    }

    // --------------------------------------------------
    //
    // Amapi cover art urls
    //
    // --------------------------------------------------

    std::string
    AmapiCovers::get_url( CoverFetchContext* ctx )
    {
#if 0
        GTimeVal tval ;
        g_get_current_time( &tval ) ;
        gchar * tval_str = g_time_val_to_iso8601( &tval ) ;
        std::string timestamp = tval_str ;
        g_free(tval_str) ;

//        char* key       = "371oQyJFdB8KTa/Jt+f9dfxToYJxZRDvIKRhVpYI" ;
        char *key = "1234567890" ;
        std::size_t keylen    = strlen( key ) ; 

        std::string sign_ = amapi2_f_sign; /*(boost::format(amapi2_f_sign)
                                % (URI::escape_string(ctx->qualifier.artist))
                                % (URI::escape_string(ctx->qualifier.album))
                                % "1E90RVC80K4MVNTCHG02"
                                % (URI::escape_string(timestamp))
                            ).str() ;*/
        char* sign = g_strdup( sign_.c_str() ) ;


        std::size_t sign_len = strlen( sign ) ;

        MHASH td ;
        unsigned char * mac ;

        td = mhash_hmac_init(MHASH_SHA1, key, keylen, mhash_get_hash_pblock(MHASH_SHA1)) ;
        mhash( td, sign, sign_len ) ;
        mac = static_cast<unsigned char*>(mhash_hmac_end( td )) ;

        g_message("%s", reinterpret_cast<char*>(mac)) ;
        
        unsigned long len ;
        unsigned char* b64 = rfc822_binary( mac, strlen(reinterpret_cast<const char*>(mac)), &len ) ; 
#endif

        return (amapi2_f 
                    % (URI::escape_string(ctx->qualifier.artist)) 
                    % (URI::escape_string(ctx->qualifier.album))
               ).str();
    }
 
    void
    AmapiCovers::load_artwork(CoverFetchContext* ctx)
    {
        std::string url = get_url( ctx ) ;
        g_message("Amapi URL: %s", url.c_str()) ;

        request = Soup::RequestSync::create( url ) ; 
        guint code = request->run();

        if( code == 200 )
        {
/*
			try
            {
                std::string album;

                album = xpath_get_text(
                    request->get_data_raw(),
                    request->get_data_size(),
					"/amazon:ItemSearchResponse/amazon:Items//amazon:Item//amazon:ItemAttributes//amazon:Title",
                    "amazon=http://webservices.amazon.com/AWSECommerceService/2005-10-05"
                ); 

                if( ld_distance<std::string>(album, ctx->qualifier.album) > 3 )
                {
                    return;
                }
			}
            catch (std::runtime_error & cxe)
			{}
*/

            std::string image_url;
            bool got_image = false;

            if(!got_image)
            {
                try
                {
                    image_url = xpath_get_text(
                        request->get_data_raw(),
                        request->get_data_size(),
                        "//amazon:Items//amazon:Item//amazon:LargeImage//amazon:URL",
                        "amazon=http://webservices.amazon.com/AWSECommerceService/2005-10-05"
                    ); 

                    got_image = true;

                } 
                catch (std::runtime_error & cxe)
                {
                }
            }

            if(!got_image)
            {
                try
                {
                    image_url = xpath_get_text(
                        request->get_data_raw(),
                        request->get_data_size(),
	    				"//amazon:Items//amazon:Item//amazon:MediumImage//amazon:URL",
                        "amazon=http://webservices.amazon.com/AWSECommerceService/2005-10-05"
                    ); 

                    got_image = true;
    			}
                catch (std::runtime_error & cxe)
			    {
    			}
            }

            if( !got_image )
            {
                try
                {
                    image_url = xpath_get_text(
                        request->get_data_raw(),
                        request->get_data_size(),
			    		"//amazon:Items//amazon:Item//amazon:SmallImage//amazon:URL",
                        "amazon=http://webservices.amazon.com/AWSECommerceService/2005-10-05"
                    ); 

                    got_image = true;
    			}
                catch (std::runtime_error & cxe)
	    		{
    			}
            }

            if( got_image && image_url.length() )
            {
                g_message("Got Image at URL: %s", image_url.c_str()) ;
                RemoteStore::fetch_image( image_url, ctx ) ;
            }
        }
    }

    // --------------------------------------------------
    //
    // MusicBrainz cover art urls
    //
    // --------------------------------------------------

    std::string
    MusicBrainzCovers::get_url(CoverFetchContext* ctx)
    {
        return (mbxml_f % ctx->qualifier.mbid).str();
    }

    void
    MusicBrainzCovers::load_artwork(CoverFetchContext* ctx)
    {
        std::string url = get_url( ctx ) ;
        g_message("MB URL: %s", url.c_str() ) ;

        request = Soup::RequestSync::create( url ) ; 
        guint code = request->run();

        if( code == 200 )
        {
			std::string image_url;

			try
            {
			    image_url = xpath_get_text(
                    request->get_data_raw(),
                    request->get_data_size(),
					"/mb:metadata/mb:release/mb:relation-list//mb:relation[@type='CoverArtLink']/@target",
                    "mb=http://musicbrainz.org/ns/mmd-1.0# ext=http://musicbrainz.org/ns/ext-1.0#"
                ); 

                g_message("Image URL: '%s'", image_url.c_str() ) ;
			}
            catch (std::runtime_error & cxe)
			{
                g_message("Runtime error: %s", cxe.what() ) ;
				return;
			}

            RemoteStore::fetch_image( image_url, ctx ) ;
        }
    }

#if 0
    // --------------------------------------------------
    //
    // Amazon store
    //
    // --------------------------------------------------
    bool
    AmazonCovers::can_load_artwork(CoverFetchContext* ctx)
    {
        return !ctx->qualifier.asin.empty();
    }

    std::string
    AmazonCovers::get_url(CoverFetchContext* ctx)
    {
        return (amazon_f[n] % ctx->qualifier.asin).str();
    }

    void
    AmazonCovers::request_failed( CoverFetchContext *ctx )
    {
        ++(n);

        if(n < G_N_ELEMENTS(amazon_f))
        {
            load_artwork( ctx );
            return;
        }

        n = 0;
        m_fetch_state = FETCH_STATE_NOT_FETCHED;
    }

    // --------------------------------------------------
    //
    // LocalFs (filesystem) covers
    //
    // --------------------------------------------------
    void
    LocalCovers::load_artwork(CoverFetchContext* ctx)
    {
        Glib::RefPtr<Gio::File> directory = Gio::File::create_for_uri( ctx->qualifier.uri )->get_parent( );
        Glib::RefPtr<Gio::FileEnumerator> files = directory->enumerate_children( );
    
        Glib::RefPtr<Gio::FileInfo> file;
        std::string cover_art;

        while ( (file = files->next_file()) != 0 )
        {
            if( file->get_content_type().find("image") != std::string::npos &&
                ( file->get_name().find("folder") ||
                  file->get_name().find("cover")  ||
                  file->get_name().find("front")     )
              )
            {
                cover_art = directory->get_child(file->get_name())->get_path();
                break;
            }
        }

        if( !cover_art.empty() )
        {
            try{
                RefPtr<Gdk::Pixbuf> cover = Gdk::Pixbuf::create_from_file( cover_art );
                covers.cache_artwork( ctx->qualifier.mbid, cover );
                m_fetch_state = FETCH_STATE_COVER_SAVED;
            }
            catch (Gdk::PixbufError)
            {
            }
            catch (Glib::FileError)
            {
            }
        }
    }
#endif

#if 0
    // --------------------------------------------------
    //
    // Inline covers 
    //
    // --------------------------------------------------
    void
    InlineCovers::load_artwork(CoverFetchContext* ctx)
    {
        Glib::RefPtr<Gdk::Pixbuf> cover;

        if( !ctx->qualifier.uri.empty() )
        try{
            MPEG::File opfile (ctx->qualifier.uri.c_str());
            if(opfile.isOpen() && opfile.isValid())
            {
                using TagLib::ID3v2::FrameList;
                ID3v2::Tag * tag = opfile.ID3v2Tag (false);
                if( tag )
                {
                    FrameList const& list = tag->frameList();
                    for(FrameList::ConstIterator i = list.begin(); i != list.end(); ++i)
                    {
                        TagLib::ID3v2::Frame const* frame = *i;
                        if( frame )
                        {
                            TagLib::ID3v2::AttachedPictureFrame const* picture =
                                dynamic_cast<TagLib::ID3v2::AttachedPictureFrame const*>(frame);

                            if( picture /*&& picture->type() == TagLib::ID3v2::AttachedPictureFrame::FrontCover*/ )
                            {
                                RefPtr<Gdk::PixbufLoader> loader = Gdk::PixbufLoader::create ();
                                ByteVector picdata = picture->picture();
                                loader->write (reinterpret_cast<const guint8*>(picdata.data()), picdata.size());
                                loader->close ();
                                cover = loader->get_pixbuf();
                                break;
                            }
                        }
                    }
                }
            }
        } catch( ... )
        {
        }

        if( cover )
        {
            covers.cache_artwork( ctx->qualifier.mbid, cover );
            m_fetch_state = FETCH_STATE_COVER_SAVED;
        }
    }
#endif
}
