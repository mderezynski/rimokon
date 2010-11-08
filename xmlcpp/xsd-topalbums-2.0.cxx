// Copyright (C) 2005-2007 Code Synthesis Tools CC
//
// This program was generated by CodeSynthesis XSD, an XML Schema to
// C++ data binding compiler.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
//
// In addition, as a special exception, Code Synthesis Tools CC gives
// permission to link this program with the Xerces-C++ library (or with
// modified versions of Xerces-C++ that use the same license as Xerces-C++),
// and distribute linked combinations including the two. You must obey
// the GNU General Public License version 2 in all respects for all of
// the code used other than Xerces-C++. If you modify this copy of the
// program, you may extend this exception to your version of the program,
// but you are not obligated to do so. If you do not wish to do so, delete
// this exception statement from your version.
//
// Furthermore, Code Synthesis Tools CC makes a special exception for
// the Free/Libre and Open Source Software (FLOSS) which is described
// in the accompanying FLOSSE file.
//

// Begin prologue.
//
//
// End prologue.

#include <xsd/cxx/pre.hxx>

#include "xsd-topalbums-2.0.hxx"

namespace lfm_tagtopalbums
{
  // lfm
  // 

  const lfm::topalbums_type& lfm::
  topalbums () const
  {
    return this->topalbums_.get ();
  }

  lfm::topalbums_type& lfm::
  topalbums ()
  {
    return this->topalbums_.get ();
  }

  void lfm::
  topalbums (const topalbums_type& topalbums)
  {
    this->topalbums_.set (topalbums);
  }

  void lfm::
  topalbums (::std::auto_ptr< topalbums_type > topalbums)
  {
    this->topalbums_.set (topalbums);
  }

  const lfm::status_type& lfm::
  status () const
  {
    return this->status_.get ();
  }

  lfm::status_type& lfm::
  status ()
  {
    return this->status_.get ();
  }

  void lfm::
  status (const status_type& status)
  {
    this->status_.set (status);
  }

  void lfm::
  status (::std::auto_ptr< status_type > status)
  {
    this->status_.set (status);
  }


  // topalbums
  // 

  const topalbums::album_sequence& topalbums::
  album () const
  {
    return this->album_;
  }

  topalbums::album_sequence& topalbums::
  album ()
  {
    return this->album_;
  }

  void topalbums::
  album (const album_sequence& album)
  {
    this->album_ = album;
  }

  const topalbums::tag_type& topalbums::
  tag () const
  {
    return this->tag_.get ();
  }

  topalbums::tag_type& topalbums::
  tag ()
  {
    return this->tag_.get ();
  }

  void topalbums::
  tag (const tag_type& tag)
  {
    this->tag_.set (tag);
  }

  void topalbums::
  tag (::std::auto_ptr< tag_type > tag)
  {
    this->tag_.set (tag);
  }


  // album
  // 

  const album::name_type& album::
  name () const
  {
    return this->name_.get ();
  }

  album::name_type& album::
  name ()
  {
    return this->name_.get ();
  }

  void album::
  name (const name_type& name)
  {
    this->name_.set (name);
  }

  void album::
  name (::std::auto_ptr< name_type > name)
  {
    this->name_.set (name);
  }

  const album::tagcount_type& album::
  tagcount () const
  {
    return this->tagcount_.get ();
  }

  album::tagcount_type& album::
  tagcount ()
  {
    return this->tagcount_.get ();
  }

  void album::
  tagcount (const tagcount_type& tagcount)
  {
    this->tagcount_.set (tagcount);
  }

  const album::mbid_type& album::
  mbid () const
  {
    return this->mbid_.get ();
  }

  album::mbid_type& album::
  mbid ()
  {
    return this->mbid_.get ();
  }

  void album::
  mbid (const mbid_type& mbid)
  {
    this->mbid_.set (mbid);
  }

  void album::
  mbid (::std::auto_ptr< mbid_type > mbid)
  {
    this->mbid_.set (mbid);
  }

  const album::url_type& album::
  url () const
  {
    return this->url_.get ();
  }

  album::url_type& album::
  url ()
  {
    return this->url_.get ();
  }

  void album::
  url (const url_type& url)
  {
    this->url_.set (url);
  }

  void album::
  url (::std::auto_ptr< url_type > url)
  {
    this->url_.set (url);
  }

  const album::artist_type& album::
  artist () const
  {
    return this->artist_.get ();
  }

  album::artist_type& album::
  artist ()
  {
    return this->artist_.get ();
  }

  void album::
  artist (const artist_type& artist)
  {
    this->artist_.set (artist);
  }

  void album::
  artist (::std::auto_ptr< artist_type > artist)
  {
    this->artist_.set (artist);
  }

  const album::image_sequence& album::
  image () const
  {
    return this->image_;
  }

  album::image_sequence& album::
  image ()
  {
    return this->image_;
  }

  void album::
  image (const image_sequence& image)
  {
    this->image_ = image;
  }

  const album::rank_type& album::
  rank () const
  {
    return this->rank_.get ();
  }

  album::rank_type& album::
  rank ()
  {
    return this->rank_.get ();
  }

  void album::
  rank (const rank_type& rank)
  {
    this->rank_.set (rank);
  }


  // artist
  // 

  const artist::name_type& artist::
  name () const
  {
    return this->name_.get ();
  }

  artist::name_type& artist::
  name ()
  {
    return this->name_.get ();
  }

  void artist::
  name (const name_type& name)
  {
    this->name_.set (name);
  }

  void artist::
  name (::std::auto_ptr< name_type > name)
  {
    this->name_.set (name);
  }

  const artist::mbid_type& artist::
  mbid () const
  {
    return this->mbid_.get ();
  }

  artist::mbid_type& artist::
  mbid ()
  {
    return this->mbid_.get ();
  }

  void artist::
  mbid (const mbid_type& mbid)
  {
    this->mbid_.set (mbid);
  }

  void artist::
  mbid (::std::auto_ptr< mbid_type > mbid)
  {
    this->mbid_.set (mbid);
  }

  const artist::url_type& artist::
  url () const
  {
    return this->url_.get ();
  }

  artist::url_type& artist::
  url ()
  {
    return this->url_.get ();
  }

  void artist::
  url (const url_type& url)
  {
    this->url_.set (url);
  }

  void artist::
  url (::std::auto_ptr< url_type > url)
  {
    this->url_.set (url);
  }


  // image
  // 

  const image::size_type& image::
  size () const
  {
    return this->size_.get ();
  }

  image::size_type& image::
  size ()
  {
    return this->size_.get ();
  }

  void image::
  size (const size_type& size)
  {
    this->size_.set (size);
  }

  void image::
  size (::std::auto_ptr< size_type > size)
  {
    this->size_.set (size);
  }
}

#include <xsd/cxx/xml/dom/parsing-source.hxx>

namespace lfm_tagtopalbums
{
  // lfm
  //

  lfm::
  lfm (const topalbums_type& topalbums,
       const status_type& status)
  : ::xml_schema::type (),
    topalbums_ (topalbums, ::xml_schema::flags (), this),
    status_ (status, ::xml_schema::flags (), this)
  {
  }

  lfm::
  lfm (const lfm& x,
       ::xml_schema::flags f,
       ::xml_schema::type* c)
  : ::xml_schema::type (x, f, c),
    topalbums_ (x.topalbums_, f, this),
    status_ (x.status_, f, this)
  {
  }

  lfm::
  lfm (const ::xercesc::DOMElement& e,
       ::xml_schema::flags f,
       ::xml_schema::type* c)
  : ::xml_schema::type (e, f | ::xml_schema::flags::base, c),
    topalbums_ (f, this),
    status_ (f, this)
  {
    if ((f & ::xml_schema::flags::base) == 0)
    {
      ::xsd::cxx::xml::dom::parser< char > p (e);
      this->parse (p, f);
    }
  }

  void lfm::
  parse (::xsd::cxx::xml::dom::parser< char >& p,
         ::xml_schema::flags f)
  {
    for (; p.more_elements (); p.next_element ())
    {
      const ::xercesc::DOMElement& i (p.cur_element ());
      const ::xsd::cxx::xml::qualified_name< char > n (
        ::xsd::cxx::xml::dom::name< char > (i));

      // topalbums
      //
      if (n.name () == "topalbums" && n.namespace_ () == "")
      {
        ::std::auto_ptr< topalbums_type > r (
          topalbums_traits::create (i, f, this));

        if (!topalbums_.present ())
        {
          this->topalbums (r);
          continue;
        }
      }

      break;
    }

    if (!topalbums_.present ())
    {
      throw ::xsd::cxx::tree::expected_element< char > (
        "topalbums",
        "");
    }

    while (p.more_attributes ())
    {
      const ::xercesc::DOMAttr& i (p.next_attribute ());
      const ::xsd::cxx::xml::qualified_name< char > n (
        ::xsd::cxx::xml::dom::name< char > (i));

      if (n.name () == "status" && n.namespace_ ().empty ())
      {
        ::std::auto_ptr< status_type > r (
          status_traits::create (i, f, this));

        this->status (r);
        continue;
      }
    }

    if (!status_.present ())
    {
      throw ::xsd::cxx::tree::expected_attribute< char > (
        "status",
        "");
    }
  }

  lfm* lfm::
  _clone (::xml_schema::flags f,
          ::xml_schema::type* c) const
  {
    return new lfm (*this, f, c);
  }

  // topalbums
  //

  topalbums::
  topalbums (const tag_type& tag)
  : ::xml_schema::type (),
    album_ (::xml_schema::flags (), this),
    tag_ (tag, ::xml_schema::flags (), this)
  {
  }

  topalbums::
  topalbums (const topalbums& x,
             ::xml_schema::flags f,
             ::xml_schema::type* c)
  : ::xml_schema::type (x, f, c),
    album_ (x.album_, f, this),
    tag_ (x.tag_, f, this)
  {
  }

  topalbums::
  topalbums (const ::xercesc::DOMElement& e,
             ::xml_schema::flags f,
             ::xml_schema::type* c)
  : ::xml_schema::type (e, f | ::xml_schema::flags::base, c),
    album_ (f, this),
    tag_ (f, this)
  {
    if ((f & ::xml_schema::flags::base) == 0)
    {
      ::xsd::cxx::xml::dom::parser< char > p (e);
      this->parse (p, f);
    }
  }

  void topalbums::
  parse (::xsd::cxx::xml::dom::parser< char >& p,
         ::xml_schema::flags f)
  {
    for (; p.more_elements (); p.next_element ())
    {
      const ::xercesc::DOMElement& i (p.cur_element ());
      const ::xsd::cxx::xml::qualified_name< char > n (
        ::xsd::cxx::xml::dom::name< char > (i));

      // album
      //
      if (n.name () == "album" && n.namespace_ () == "")
      {
        ::std::auto_ptr< album_type > r (
          album_traits::create (i, f, this));

        this->album ().push_back (r);
        continue;
      }

      break;
    }

    while (p.more_attributes ())
    {
      const ::xercesc::DOMAttr& i (p.next_attribute ());
      const ::xsd::cxx::xml::qualified_name< char > n (
        ::xsd::cxx::xml::dom::name< char > (i));

      if (n.name () == "tag" && n.namespace_ ().empty ())
      {
        ::std::auto_ptr< tag_type > r (
          tag_traits::create (i, f, this));

        this->tag (r);
        continue;
      }
    }

    if (!tag_.present ())
    {
      throw ::xsd::cxx::tree::expected_attribute< char > (
        "tag",
        "");
    }
  }

  topalbums* topalbums::
  _clone (::xml_schema::flags f,
          ::xml_schema::type* c) const
  {
    return new topalbums (*this, f, c);
  }

  // album
  //

  album::
  album (const name_type& name,
         const tagcount_type& tagcount,
         const mbid_type& mbid,
         const url_type& url,
         const artist_type& artist,
         const rank_type& rank)
  : ::xml_schema::type (),
    name_ (name, ::xml_schema::flags (), this),
    tagcount_ (tagcount, ::xml_schema::flags (), this),
    mbid_ (mbid, ::xml_schema::flags (), this),
    url_ (url, ::xml_schema::flags (), this),
    artist_ (artist, ::xml_schema::flags (), this),
    image_ (::xml_schema::flags (), this),
    rank_ (rank, ::xml_schema::flags (), this)
  {
  }

  album::
  album (const album& x,
         ::xml_schema::flags f,
         ::xml_schema::type* c)
  : ::xml_schema::type (x, f, c),
    name_ (x.name_, f, this),
    tagcount_ (x.tagcount_, f, this),
    mbid_ (x.mbid_, f, this),
    url_ (x.url_, f, this),
    artist_ (x.artist_, f, this),
    image_ (x.image_, f, this),
    rank_ (x.rank_, f, this)
  {
  }

  album::
  album (const ::xercesc::DOMElement& e,
         ::xml_schema::flags f,
         ::xml_schema::type* c)
  : ::xml_schema::type (e, f | ::xml_schema::flags::base, c),
    name_ (f, this),
    tagcount_ (f, this),
    mbid_ (f, this),
    url_ (f, this),
    artist_ (f, this),
    image_ (f, this),
    rank_ (f, this)
  {
    if ((f & ::xml_schema::flags::base) == 0)
    {
      ::xsd::cxx::xml::dom::parser< char > p (e);
      this->parse (p, f);
    }
  }

  void album::
  parse (::xsd::cxx::xml::dom::parser< char >& p,
         ::xml_schema::flags f)
  {
    for (; p.more_elements (); p.next_element ())
    {
      const ::xercesc::DOMElement& i (p.cur_element ());
      const ::xsd::cxx::xml::qualified_name< char > n (
        ::xsd::cxx::xml::dom::name< char > (i));

      // name
      //
      if (n.name () == "name" && n.namespace_ () == "")
      {
        ::std::auto_ptr< name_type > r (
          name_traits::create (i, f, this));

        if (!name_.present ())
        {
          this->name (r);
          continue;
        }
      }

      // tagcount
      //
      if (n.name () == "tagcount" && n.namespace_ () == "")
      {
        if (!tagcount_.present ())
        {
          this->tagcount (tagcount_traits::create (i, f, this));
          continue;
        }
      }

      // mbid
      //
      if (n.name () == "mbid" && n.namespace_ () == "")
      {
        ::std::auto_ptr< mbid_type > r (
          mbid_traits::create (i, f, this));

        if (!mbid_.present ())
        {
          this->mbid (r);
          continue;
        }
      }

      // url
      //
      if (n.name () == "url" && n.namespace_ () == "")
      {
        ::std::auto_ptr< url_type > r (
          url_traits::create (i, f, this));

        if (!url_.present ())
        {
          this->url (r);
          continue;
        }
      }

      // artist
      //
      if (n.name () == "artist" && n.namespace_ () == "")
      {
        ::std::auto_ptr< artist_type > r (
          artist_traits::create (i, f, this));

        if (!artist_.present ())
        {
          this->artist (r);
          continue;
        }
      }

      // image
      //
      if (n.name () == "image" && n.namespace_ () == "")
      {
        ::std::auto_ptr< image_type > r (
          image_traits::create (i, f, this));

        this->image ().push_back (r);
        continue;
      }

      break;
    }

    if (!name_.present ())
    {
      throw ::xsd::cxx::tree::expected_element< char > (
        "name",
        "");
    }

    if (!tagcount_.present ())
    {
      throw ::xsd::cxx::tree::expected_element< char > (
        "tagcount",
        "");
    }

    if (!mbid_.present ())
    {
      throw ::xsd::cxx::tree::expected_element< char > (
        "mbid",
        "");
    }

    if (!url_.present ())
    {
      throw ::xsd::cxx::tree::expected_element< char > (
        "url",
        "");
    }

    if (!artist_.present ())
    {
      throw ::xsd::cxx::tree::expected_element< char > (
        "artist",
        "");
    }

    while (p.more_attributes ())
    {
      const ::xercesc::DOMAttr& i (p.next_attribute ());
      const ::xsd::cxx::xml::qualified_name< char > n (
        ::xsd::cxx::xml::dom::name< char > (i));

      if (n.name () == "rank" && n.namespace_ ().empty ())
      {
        this->rank (rank_traits::create (i, f, this));
        continue;
      }
    }

    if (!rank_.present ())
    {
      throw ::xsd::cxx::tree::expected_attribute< char > (
        "rank",
        "");
    }
  }

  album* album::
  _clone (::xml_schema::flags f,
          ::xml_schema::type* c) const
  {
    return new album (*this, f, c);
  }

  // artist
  //

  artist::
  artist (const name_type& name,
          const mbid_type& mbid,
          const url_type& url)
  : ::xml_schema::type (),
    name_ (name, ::xml_schema::flags (), this),
    mbid_ (mbid, ::xml_schema::flags (), this),
    url_ (url, ::xml_schema::flags (), this)
  {
  }

  artist::
  artist (const artist& x,
          ::xml_schema::flags f,
          ::xml_schema::type* c)
  : ::xml_schema::type (x, f, c),
    name_ (x.name_, f, this),
    mbid_ (x.mbid_, f, this),
    url_ (x.url_, f, this)
  {
  }

  artist::
  artist (const ::xercesc::DOMElement& e,
          ::xml_schema::flags f,
          ::xml_schema::type* c)
  : ::xml_schema::type (e, f | ::xml_schema::flags::base, c),
    name_ (f, this),
    mbid_ (f, this),
    url_ (f, this)
  {
    if ((f & ::xml_schema::flags::base) == 0)
    {
      ::xsd::cxx::xml::dom::parser< char > p (e);
      this->parse (p, f);
    }
  }

  void artist::
  parse (::xsd::cxx::xml::dom::parser< char >& p,
         ::xml_schema::flags f)
  {
    for (; p.more_elements (); p.next_element ())
    {
      const ::xercesc::DOMElement& i (p.cur_element ());
      const ::xsd::cxx::xml::qualified_name< char > n (
        ::xsd::cxx::xml::dom::name< char > (i));

      // name
      //
      if (n.name () == "name" && n.namespace_ () == "")
      {
        ::std::auto_ptr< name_type > r (
          name_traits::create (i, f, this));

        if (!name_.present ())
        {
          this->name (r);
          continue;
        }
      }

      // mbid
      //
      if (n.name () == "mbid" && n.namespace_ () == "")
      {
        ::std::auto_ptr< mbid_type > r (
          mbid_traits::create (i, f, this));

        if (!mbid_.present ())
        {
          this->mbid (r);
          continue;
        }
      }

      // url
      //
      if (n.name () == "url" && n.namespace_ () == "")
      {
        ::std::auto_ptr< url_type > r (
          url_traits::create (i, f, this));

        if (!url_.present ())
        {
          this->url (r);
          continue;
        }
      }

      break;
    }

    if (!name_.present ())
    {
      throw ::xsd::cxx::tree::expected_element< char > (
        "name",
        "");
    }

    if (!mbid_.present ())
    {
      throw ::xsd::cxx::tree::expected_element< char > (
        "mbid",
        "");
    }

    if (!url_.present ())
    {
      throw ::xsd::cxx::tree::expected_element< char > (
        "url",
        "");
    }
  }

  artist* artist::
  _clone (::xml_schema::flags f,
          ::xml_schema::type* c) const
  {
    return new artist (*this, f, c);
  }

  // image
  //

  image::
  image (const ::xml_schema::uri& anyURI,
         const size_type& size)
  : ::xml_schema::uri (anyURI),
    size_ (size, ::xml_schema::flags (), this)
  {
  }

  image::
  image (const image& x,
         ::xml_schema::flags f,
         ::xml_schema::type* c)
  : ::xml_schema::uri (x, f, c),
    size_ (x.size_, f, this)
  {
  }

  image::
  image (const ::xercesc::DOMElement& e,
         ::xml_schema::flags f,
         ::xml_schema::type* c)
  : ::xml_schema::uri (e, f | ::xml_schema::flags::base, c),
    size_ (f, this)
  {
    if ((f & ::xml_schema::flags::base) == 0)
    {
      ::xsd::cxx::xml::dom::parser< char > p (e);
      this->parse (p, f);
    }
  }

  void image::
  parse (::xsd::cxx::xml::dom::parser< char >& p,
         ::xml_schema::flags f)
  {
    while (p.more_attributes ())
    {
      const ::xercesc::DOMAttr& i (p.next_attribute ());
      const ::xsd::cxx::xml::qualified_name< char > n (
        ::xsd::cxx::xml::dom::name< char > (i));

      if (n.name () == "size" && n.namespace_ ().empty ())
      {
        ::std::auto_ptr< size_type > r (
          size_traits::create (i, f, this));

        this->size (r);
        continue;
      }
    }

    if (!size_.present ())
    {
      throw ::xsd::cxx::tree::expected_attribute< char > (
        "size",
        "");
    }
  }

  image* image::
  _clone (::xml_schema::flags f,
          ::xml_schema::type* c) const
  {
    return new image (*this, f, c);
  }
}

#include <istream>
#include <xercesc/framework/Wrapper4InputSource.hpp>
#include <xsd/cxx/xml/sax/std-input-source.hxx>
#include <xsd/cxx/tree/error-handler.hxx>

namespace lfm_tagtopalbums
{
  ::std::auto_ptr< ::lfm_tagtopalbums::lfm >
  lfm_ (const ::std::string& u,
        ::xml_schema::flags f,
        const ::xml_schema::properties& p)
  {
    ::xsd::cxx::xml::auto_initializer i (
      (f & ::xml_schema::flags::dont_initialize) == 0,
      (f & ::xml_schema::flags::keep_dom) == 0);

    ::xsd::cxx::tree::error_handler< char > h;

    ::xsd::cxx::xml::dom::auto_ptr< ::xercesc::DOMDocument > d (
      ::xsd::cxx::xml::dom::parse< char > (u, h, p, f));

    h.throw_if_failed< ::xsd::cxx::tree::parsing< char > > ();

    ::std::auto_ptr< ::lfm_tagtopalbums::lfm > r (
      ::lfm_tagtopalbums::lfm_ (
        d.get (), f | ::xml_schema::flags::own_dom, p));

    if (f & ::xml_schema::flags::keep_dom)
      d.release ();

    return r;
  }

  ::std::auto_ptr< ::lfm_tagtopalbums::lfm >
  lfm_ (const ::std::string& u,
        ::xml_schema::error_handler& h,
        ::xml_schema::flags f,
        const ::xml_schema::properties& p)
  {
    ::xsd::cxx::xml::auto_initializer i (
      (f & ::xml_schema::flags::dont_initialize) == 0,
      (f & ::xml_schema::flags::keep_dom) == 0);

    ::xsd::cxx::xml::dom::auto_ptr< ::xercesc::DOMDocument > d (
      ::xsd::cxx::xml::dom::parse< char > (u, h, p, f));

    if (!d)
      throw ::xsd::cxx::tree::parsing< char > ();

    ::std::auto_ptr< ::lfm_tagtopalbums::lfm > r (
      ::lfm_tagtopalbums::lfm_ (
        d.get (), f | ::xml_schema::flags::own_dom, p));

    if (f & ::xml_schema::flags::keep_dom)
      d.release ();

    return r;
  }

  ::std::auto_ptr< ::lfm_tagtopalbums::lfm >
  lfm_ (const ::std::string& u,
        ::xercesc::DOMErrorHandler& h,
        ::xml_schema::flags f,
        const ::xml_schema::properties& p)
  {
    ::xsd::cxx::xml::dom::auto_ptr< ::xercesc::DOMDocument > d (
      ::xsd::cxx::xml::dom::parse< char > (u, h, p, f));

    if (!d)
      throw ::xsd::cxx::tree::parsing< char > ();

    ::std::auto_ptr< ::lfm_tagtopalbums::lfm > r (
      ::lfm_tagtopalbums::lfm_ (
        d.get (), f | ::xml_schema::flags::own_dom, p));

    if (f & ::xml_schema::flags::keep_dom)
      d.release ();

    return r;
  }

  ::std::auto_ptr< ::lfm_tagtopalbums::lfm >
  lfm_ (::std::istream& is,
        ::xml_schema::flags f,
        const ::xml_schema::properties& p)
  {
    ::xsd::cxx::xml::auto_initializer i (
      (f & ::xml_schema::flags::dont_initialize) == 0,
      (f & ::xml_schema::flags::keep_dom) == 0);

    ::xsd::cxx::xml::sax::std_input_source isrc (is);
    ::xercesc::Wrapper4InputSource wrap (&isrc, false);
    return ::lfm_tagtopalbums::lfm_ (wrap, f, p);
  }

  ::std::auto_ptr< ::lfm_tagtopalbums::lfm >
  lfm_ (::std::istream& is,
        ::xml_schema::error_handler& h,
        ::xml_schema::flags f,
        const ::xml_schema::properties& p)
  {
    ::xsd::cxx::xml::auto_initializer i (
      (f & ::xml_schema::flags::dont_initialize) == 0,
      (f & ::xml_schema::flags::keep_dom) == 0);

    ::xsd::cxx::xml::sax::std_input_source isrc (is);
    ::xercesc::Wrapper4InputSource wrap (&isrc, false);
    return ::lfm_tagtopalbums::lfm_ (wrap, h, f, p);
  }

  ::std::auto_ptr< ::lfm_tagtopalbums::lfm >
  lfm_ (::std::istream& is,
        ::xercesc::DOMErrorHandler& h,
        ::xml_schema::flags f,
        const ::xml_schema::properties& p)
  {
    ::xsd::cxx::xml::sax::std_input_source isrc (is);
    ::xercesc::Wrapper4InputSource wrap (&isrc, false);
    return ::lfm_tagtopalbums::lfm_ (wrap, h, f, p);
  }

  ::std::auto_ptr< ::lfm_tagtopalbums::lfm >
  lfm_ (::std::istream& is,
        const ::std::string& sid,
        ::xml_schema::flags f,
        const ::xml_schema::properties& p)
  {
    ::xsd::cxx::xml::auto_initializer i (
      (f & ::xml_schema::flags::dont_initialize) == 0,
      (f & ::xml_schema::flags::keep_dom) == 0);

    ::xsd::cxx::xml::sax::std_input_source isrc (is, sid);
    ::xercesc::Wrapper4InputSource wrap (&isrc, false);
    return ::lfm_tagtopalbums::lfm_ (wrap, f, p);
  }

  ::std::auto_ptr< ::lfm_tagtopalbums::lfm >
  lfm_ (::std::istream& is,
        const ::std::string& sid,
        ::xml_schema::error_handler& h,
        ::xml_schema::flags f,
        const ::xml_schema::properties& p)
  {
    ::xsd::cxx::xml::auto_initializer i (
      (f & ::xml_schema::flags::dont_initialize) == 0,
      (f & ::xml_schema::flags::keep_dom) == 0);

    ::xsd::cxx::xml::sax::std_input_source isrc (is, sid);
    ::xercesc::Wrapper4InputSource wrap (&isrc, false);
    return ::lfm_tagtopalbums::lfm_ (wrap, h, f, p);
  }

  ::std::auto_ptr< ::lfm_tagtopalbums::lfm >
  lfm_ (::std::istream& is,
        const ::std::string& sid,
        ::xercesc::DOMErrorHandler& h,
        ::xml_schema::flags f,
        const ::xml_schema::properties& p)
  {
    ::xsd::cxx::xml::sax::std_input_source isrc (is, sid);
    ::xercesc::Wrapper4InputSource wrap (&isrc, false);
    return ::lfm_tagtopalbums::lfm_ (wrap, h, f, p);
  }

  ::std::auto_ptr< ::lfm_tagtopalbums::lfm >
  lfm_ (const ::xercesc::DOMInputSource& i,
        ::xml_schema::flags f,
        const ::xml_schema::properties& p)
  {
    ::xsd::cxx::tree::error_handler< char > h;

    ::xsd::cxx::xml::dom::auto_ptr< ::xercesc::DOMDocument > d (
      ::xsd::cxx::xml::dom::parse< char > (i, h, p, f));

    h.throw_if_failed< ::xsd::cxx::tree::parsing< char > > ();

    ::std::auto_ptr< ::lfm_tagtopalbums::lfm > r (
      ::lfm_tagtopalbums::lfm_ (
        d.get (), f | ::xml_schema::flags::own_dom, p));

    if (f & ::xml_schema::flags::keep_dom)
      d.release ();

    return r;
  }

  ::std::auto_ptr< ::lfm_tagtopalbums::lfm >
  lfm_ (const ::xercesc::DOMInputSource& i,
        ::xml_schema::error_handler& h,
        ::xml_schema::flags f,
        const ::xml_schema::properties& p)
  {
    ::xsd::cxx::xml::dom::auto_ptr< ::xercesc::DOMDocument > d (
      ::xsd::cxx::xml::dom::parse< char > (i, h, p, f));

    if (!d)
      throw ::xsd::cxx::tree::parsing< char > ();

    ::std::auto_ptr< ::lfm_tagtopalbums::lfm > r (
      ::lfm_tagtopalbums::lfm_ (
        d.get (), f | ::xml_schema::flags::own_dom, p));

    if (f & ::xml_schema::flags::keep_dom)
      d.release ();

    return r;
  }

  ::std::auto_ptr< ::lfm_tagtopalbums::lfm >
  lfm_ (const ::xercesc::DOMInputSource& i,
        ::xercesc::DOMErrorHandler& h,
        ::xml_schema::flags f,
        const ::xml_schema::properties& p)
  {
    ::xsd::cxx::xml::dom::auto_ptr< ::xercesc::DOMDocument > d (
      ::xsd::cxx::xml::dom::parse< char > (i, h, p, f));

    if (!d)
      throw ::xsd::cxx::tree::parsing< char > ();

    ::std::auto_ptr< ::lfm_tagtopalbums::lfm > r (
      ::lfm_tagtopalbums::lfm_ (
        d.get (), f | ::xml_schema::flags::own_dom, p));

    if (f & ::xml_schema::flags::keep_dom)
      d.release ();

    return r;
  }

  ::std::auto_ptr< ::lfm_tagtopalbums::lfm >
  lfm_ (const ::xercesc::DOMDocument& d,
        ::xml_schema::flags f,
        const ::xml_schema::properties& p)
  {
    if (f & ::xml_schema::flags::keep_dom)
    {
      ::xsd::cxx::xml::dom::auto_ptr< ::xercesc::DOMDocument > c (
        static_cast< ::xercesc::DOMDocument* > (d.cloneNode (true)));

      ::std::auto_ptr< ::lfm_tagtopalbums::lfm > r (
        ::lfm_tagtopalbums::lfm_ (
          c.get (), f | ::xml_schema::flags::own_dom, p));

      c.release ();
      return r;
    }

    const ::xercesc::DOMElement& e (*d.getDocumentElement ());
    const ::xsd::cxx::xml::qualified_name< char > n (
      ::xsd::cxx::xml::dom::name< char > (e));

    if (n.name () == "lfm" &&
        n.namespace_ () == "")
    {
      ::std::auto_ptr< ::lfm_tagtopalbums::lfm > r (
        ::xsd::cxx::tree::traits< ::lfm_tagtopalbums::lfm, char >::create (
          e, f, 0));
      return r;
    }

    throw ::xsd::cxx::tree::unexpected_element < char > (
      n.name (),
      n.namespace_ (),
      "lfm",
      "");
  }

  ::std::auto_ptr< ::lfm_tagtopalbums::lfm >
  lfm_ (::xercesc::DOMDocument* d,
        ::xml_schema::flags f,
        const ::xml_schema::properties&)
  {
    ::xsd::cxx::xml::dom::auto_ptr< ::xercesc::DOMDocument > c (
      ((f & ::xml_schema::flags::keep_dom) &&
       !(f & ::xml_schema::flags::own_dom))
      ? static_cast< ::xercesc::DOMDocument* > (d->cloneNode (true))
      : 0);

    const ::xercesc::DOMElement& e (
      c.get ()
      ? *c->getDocumentElement ()
      : *d->getDocumentElement ());
    const ::xsd::cxx::xml::qualified_name< char > n (
      ::xsd::cxx::xml::dom::name< char > (e));

    if (n.name () == "lfm" &&
        n.namespace_ () == "")
    {
      ::std::auto_ptr< ::lfm_tagtopalbums::lfm > r (
        ::xsd::cxx::tree::traits< ::lfm_tagtopalbums::lfm, char >::create (
          e, f, 0));
      c.release ();
      return r;
    }

    throw ::xsd::cxx::tree::unexpected_element < char > (
      n.name (),
      n.namespace_ (),
      "lfm",
      "");
  }
}

#include <xsd/cxx/post.hxx>

// Begin epilogue.
//
//
// End epilogue.

