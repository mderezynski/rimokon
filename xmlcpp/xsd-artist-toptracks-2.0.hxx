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

#ifndef XSD_ARTIST_TOPTRACKS_2_0_HXX
#define XSD_ARTIST_TOPTRACKS_2_0_HXX

// Begin prologue.
//
//
// End prologue.

#include <xsd/cxx/version.hxx>

#if (XSD_INT_VERSION != 3000000L)
#error XSD runtime version mismatch
#endif

#include <xsd/cxx/pre.hxx>

#ifndef XSD_USE_CHAR
#define XSD_USE_CHAR
#endif

#ifndef XSD_CXX_TREE_USE_CHAR
#define XSD_CXX_TREE_USE_CHAR
#endif

#include <xsd/cxx/tree/exceptions.hxx>
#include <xsd/cxx/tree/elements.hxx>
#include <xsd/cxx/tree/types.hxx>

#include <xsd/cxx/xml/error-handler.hxx>

#include <xsd/cxx/tree/parsing.hxx>

namespace xml_schema
{
  // anyType and anySimpleType.
  //
  typedef ::xsd::cxx::tree::type type;
  typedef ::xsd::cxx::tree::simple_type<type> simple_type;

  // 8-bit
  //
  typedef signed char byte;
  typedef unsigned char unsigned_byte;

  // 16-bit
  //
  typedef short short_;
  typedef unsigned short unsigned_short;

  // 32-bit
  //
  typedef int int_;
  typedef unsigned int unsigned_int;

  // 64-bit
  //
  typedef long long long_;
  typedef unsigned long long unsigned_long;

  // Supposed to be arbitrary-length integral types.
  //
  typedef long long integer;
  typedef integer non_positive_integer;
  typedef integer non_negative_integer;
  typedef integer positive_integer;
  typedef integer negative_integer;

  // Boolean.
  //
  typedef bool boolean;

  // Floating-point types.
  //
  typedef float float_;
  typedef double double_;
  typedef double decimal;

  // String types.
  //
  typedef ::xsd::cxx::tree::string< char, simple_type > string;
  typedef ::xsd::cxx::tree::normalized_string< char, string > normalized_string;
  typedef ::xsd::cxx::tree::token< char, normalized_string > token;
  typedef ::xsd::cxx::tree::name< char, token > name;
  typedef ::xsd::cxx::tree::nmtoken< char, token > nmtoken;
  typedef ::xsd::cxx::tree::nmtokens< char, simple_type, nmtoken> nmtokens;
  typedef ::xsd::cxx::tree::ncname< char, name > ncname;
  typedef ::xsd::cxx::tree::language< char, token > language;

  // ID/IDREF.
  //
  typedef ::xsd::cxx::tree::id< char, ncname > id;
  typedef ::xsd::cxx::tree::idref< type, char, ncname > idref;
  typedef ::xsd::cxx::tree::idrefs< char, simple_type, idref > idrefs;

  // URI.
  //
  typedef ::xsd::cxx::tree::uri< char, simple_type > uri;

  // Qualified name.
  //
  typedef ::xsd::cxx::tree::qname< char, simple_type, uri, ncname > qname;

  // Binary.
  //
  typedef ::xsd::cxx::tree::buffer< char > buffer;
  typedef ::xsd::cxx::tree::base64_binary< char, simple_type > base64_binary;
  typedef ::xsd::cxx::tree::hex_binary< char, simple_type > hex_binary;

  // Date/time.
  //
  typedef ::xsd::cxx::tree::date< char, simple_type > date;
  typedef ::xsd::cxx::tree::date_time< char, simple_type > date_time;
  typedef ::xsd::cxx::tree::duration< char, simple_type > duration;
  typedef ::xsd::cxx::tree::day< char, simple_type > day;
  typedef ::xsd::cxx::tree::month< char, simple_type > month;
  typedef ::xsd::cxx::tree::month_day< char, simple_type > month_day;
  typedef ::xsd::cxx::tree::year< char, simple_type > year;
  typedef ::xsd::cxx::tree::year_month< char, simple_type > year_month;
  typedef ::xsd::cxx::tree::time< char, simple_type > time;

  // Entity.
  //
  typedef ::xsd::cxx::tree::entity< char, ncname > entity;
  typedef ::xsd::cxx::tree::entities< char, simple_type, entity > entities;

  // Flags and properties.
  //
  typedef ::xsd::cxx::tree::flags flags;
  typedef ::xsd::cxx::tree::properties< char > properties;

  // DOM user data key for back pointers to tree nodes.
  //
#ifndef XSD_CXX_TREE_TREE_NODE_KEY_IN___XML_SCHEMA
#define XSD_CXX_TREE_TREE_NODE_KEY_IN___XML_SCHEMA

  const XMLCh* const tree_node_key = ::xsd::cxx::tree::user_data_keys::node;

#endif

  // Exceptions.
  //
  typedef ::xsd::cxx::tree::exception< char > exception;
  typedef ::xsd::cxx::tree::parsing< char > parsing;
  typedef ::xsd::cxx::tree::expected_element< char > expected_element;
  typedef ::xsd::cxx::tree::unexpected_element< char > unexpected_element;
  typedef ::xsd::cxx::tree::expected_attribute< char > expected_attribute;
  typedef ::xsd::cxx::tree::unexpected_enumerator< char > unexpected_enumerator;
  typedef ::xsd::cxx::tree::expected_text_content< char > expected_text_content;
  typedef ::xsd::cxx::tree::no_type_info< char > no_type_info;
  typedef ::xsd::cxx::tree::not_derived< char > not_derived;
  typedef ::xsd::cxx::tree::duplicate_id< char > duplicate_id;
  typedef ::xsd::cxx::tree::serialization< char > serialization;
  typedef ::xsd::cxx::tree::no_namespace_mapping< char > no_namespace_mapping;
  typedef ::xsd::cxx::tree::no_prefix_mapping< char > no_prefix_mapping;
  typedef ::xsd::cxx::tree::xsi_already_in_use< char > xsi_already_in_use;
  typedef ::xsd::cxx::tree::bounds< char > bounds;

  // Parsing/serialization diagnostics.
  //
  typedef ::xsd::cxx::tree::severity severity;
  typedef ::xsd::cxx::tree::error< char > error;
  typedef ::xsd::cxx::tree::diagnostics< char > diagnostics;

  // Error handler interface.
  //
  typedef ::xsd::cxx::xml::error_handler< char > error_handler;
}

// Forward declarations.
//
namespace lfm_artisttoptracks
{
  class lfm;
  class toptracks;
  class track;
  class streamable;
  class artist;
  class image;
}


#include <memory>    // std::auto_ptr
#include <algorithm> // std::binary_search

#include <xsd/cxx/tree/exceptions.hxx>
#include <xsd/cxx/tree/elements.hxx>
#include <xsd/cxx/tree/containers.hxx>
#include <xsd/cxx/tree/list.hxx>

#include <xsd/cxx/xml/dom/parsing-header.hxx>

namespace lfm_artisttoptracks
{
  class lfm: public ::xml_schema::type
  {
    public:
    // toptracks
    // 
    typedef ::lfm_artisttoptracks::toptracks toptracks_type;
    typedef ::xsd::cxx::tree::traits< toptracks_type, char > toptracks_traits;

    const toptracks_type&
    toptracks () const;

    toptracks_type&
    toptracks ();

    void
    toptracks (const toptracks_type& x);

    void
    toptracks (::std::auto_ptr< toptracks_type > p);

    // status
    // 
    typedef ::xml_schema::ncname status_type;
    typedef ::xsd::cxx::tree::traits< status_type, char > status_traits;

    const status_type&
    status () const;

    status_type&
    status ();

    void
    status (const status_type& x);

    void
    status (::std::auto_ptr< status_type > p);

    // Constructors.
    //
    lfm (const toptracks_type&,
         const status_type&);

    lfm (const ::xercesc::DOMElement& e,
         ::xml_schema::flags f = 0,
         ::xml_schema::type* c = 0);

    lfm (const lfm& x,
         ::xml_schema::flags f = 0,
         ::xml_schema::type* c = 0);

    virtual lfm*
    _clone (::xml_schema::flags f = 0,
            ::xml_schema::type* c = 0) const;

    // Implementation.
    //
    protected:
    void
    parse (::xsd::cxx::xml::dom::parser< char >&,
           ::xml_schema::flags);

    private:
    ::xsd::cxx::tree::one< toptracks_type > toptracks_;
    ::xsd::cxx::tree::one< status_type > status_;
  };

  class toptracks: public ::xml_schema::type
  {
    public:
    // track
    // 
    typedef ::lfm_artisttoptracks::track track_type;
    typedef ::xsd::cxx::tree::sequence< track_type > track_sequence;
    typedef track_sequence::iterator track_iterator;
    typedef track_sequence::const_iterator track_const_iterator;
    typedef ::xsd::cxx::tree::traits< track_type, char > track_traits;

    const track_sequence&
    track () const;

    track_sequence&
    track ();

    void
    track (const track_sequence& s);

    // artist
    // 
    typedef ::xml_schema::ncname artist_type;
    typedef ::xsd::cxx::tree::traits< artist_type, char > artist_traits;

    const artist_type&
    artist () const;

    artist_type&
    artist ();

    void
    artist (const artist_type& x);

    void
    artist (::std::auto_ptr< artist_type > p);

    // Constructors.
    //
    toptracks (const artist_type&);

    toptracks (const ::xercesc::DOMElement& e,
               ::xml_schema::flags f = 0,
               ::xml_schema::type* c = 0);

    toptracks (const toptracks& x,
               ::xml_schema::flags f = 0,
               ::xml_schema::type* c = 0);

    virtual toptracks*
    _clone (::xml_schema::flags f = 0,
            ::xml_schema::type* c = 0) const;

    // Implementation.
    //
    protected:
    void
    parse (::xsd::cxx::xml::dom::parser< char >&,
           ::xml_schema::flags);

    private:
    track_sequence track_;
    ::xsd::cxx::tree::one< artist_type > artist_;
  };

  class track: public ::xml_schema::type
  {
    public:
    // name
    // 
    typedef ::xml_schema::string name_type;
    typedef ::xsd::cxx::tree::traits< name_type, char > name_traits;

    const name_type&
    name () const;

    name_type&
    name ();

    void
    name (const name_type& x);

    void
    name (::std::auto_ptr< name_type > p);

    // playcount
    // 
    typedef ::xml_schema::integer playcount_type;
    typedef ::xsd::cxx::tree::traits< playcount_type, char > playcount_traits;

    const playcount_type&
    playcount () const;

    playcount_type&
    playcount ();

    void
    playcount (const playcount_type& x);

    // mbid
    // 
    typedef ::xml_schema::string mbid_type;
    typedef ::xsd::cxx::tree::traits< mbid_type, char > mbid_traits;

    const mbid_type&
    mbid () const;

    mbid_type&
    mbid ();

    void
    mbid (const mbid_type& x);

    void
    mbid (::std::auto_ptr< mbid_type > p);

    // url
    // 
    typedef ::xml_schema::uri url_type;
    typedef ::xsd::cxx::tree::traits< url_type, char > url_traits;

    const url_type&
    url () const;

    url_type&
    url ();

    void
    url (const url_type& x);

    void
    url (::std::auto_ptr< url_type > p);

    // streamable
    // 
    typedef ::lfm_artisttoptracks::streamable streamable_type;
    typedef ::xsd::cxx::tree::traits< streamable_type, char > streamable_traits;

    const streamable_type&
    streamable () const;

    streamable_type&
    streamable ();

    void
    streamable (const streamable_type& x);

    void
    streamable (::std::auto_ptr< streamable_type > p);

    // artist
    // 
    typedef ::lfm_artisttoptracks::artist artist_type;
    typedef ::xsd::cxx::tree::traits< artist_type, char > artist_traits;

    const artist_type&
    artist () const;

    artist_type&
    artist ();

    void
    artist (const artist_type& x);

    void
    artist (::std::auto_ptr< artist_type > p);

    // image
    // 
    typedef ::lfm_artisttoptracks::image image_type;
    typedef ::xsd::cxx::tree::sequence< image_type > image_sequence;
    typedef image_sequence::iterator image_iterator;
    typedef image_sequence::const_iterator image_const_iterator;
    typedef ::xsd::cxx::tree::traits< image_type, char > image_traits;

    const image_sequence&
    image () const;

    image_sequence&
    image ();

    void
    image (const image_sequence& s);

    // rank
    // 
    typedef ::xml_schema::integer rank_type;
    typedef ::xsd::cxx::tree::traits< rank_type, char > rank_traits;

    const rank_type&
    rank () const;

    rank_type&
    rank ();

    void
    rank (const rank_type& x);

    // Constructors.
    //
    track (const name_type&,
           const playcount_type&,
           const mbid_type&,
           const url_type&,
           const streamable_type&,
           const artist_type&,
           const rank_type&);

    track (const ::xercesc::DOMElement& e,
           ::xml_schema::flags f = 0,
           ::xml_schema::type* c = 0);

    track (const track& x,
           ::xml_schema::flags f = 0,
           ::xml_schema::type* c = 0);

    virtual track*
    _clone (::xml_schema::flags f = 0,
            ::xml_schema::type* c = 0) const;

    // Implementation.
    //
    protected:
    void
    parse (::xsd::cxx::xml::dom::parser< char >&,
           ::xml_schema::flags);

    private:
    ::xsd::cxx::tree::one< name_type > name_;
    ::xsd::cxx::tree::one< playcount_type > playcount_;
    ::xsd::cxx::tree::one< mbid_type > mbid_;
    ::xsd::cxx::tree::one< url_type > url_;
    ::xsd::cxx::tree::one< streamable_type > streamable_;
    ::xsd::cxx::tree::one< artist_type > artist_;
    image_sequence image_;
    ::xsd::cxx::tree::one< rank_type > rank_;
  };

  class streamable: public ::xsd::cxx::tree::fundamental_base< ::xml_schema::integer, char, ::xml_schema::simple_type >
  {
    public:
    // fulltrack
    // 
    typedef ::xml_schema::integer fulltrack_type;
    typedef ::xsd::cxx::tree::traits< fulltrack_type, char > fulltrack_traits;

    const fulltrack_type&
    fulltrack () const;

    fulltrack_type&
    fulltrack ();

    void
    fulltrack (const fulltrack_type& x);

    // Constructors.
    //
    streamable (const ::xml_schema::integer&,
                const fulltrack_type&);

    streamable (const ::xercesc::DOMElement& e,
                ::xml_schema::flags f = 0,
                ::xml_schema::type* c = 0);

    streamable (const streamable& x,
                ::xml_schema::flags f = 0,
                ::xml_schema::type* c = 0);

    virtual streamable*
    _clone (::xml_schema::flags f = 0,
            ::xml_schema::type* c = 0) const;

    // Implementation.
    //
    protected:
    void
    parse (::xsd::cxx::xml::dom::parser< char >&,
           ::xml_schema::flags);

    private:
    ::xsd::cxx::tree::one< fulltrack_type > fulltrack_;
  };

  class artist: public ::xml_schema::type
  {
    public:
    // name
    // 
    typedef ::xml_schema::string name_type;
    typedef ::xsd::cxx::tree::traits< name_type, char > name_traits;

    const name_type&
    name () const;

    name_type&
    name ();

    void
    name (const name_type& x);

    void
    name (::std::auto_ptr< name_type > p);

    // mbid
    // 
    typedef ::xml_schema::string mbid_type;
    typedef ::xsd::cxx::tree::traits< mbid_type, char > mbid_traits;

    const mbid_type&
    mbid () const;

    mbid_type&
    mbid ();

    void
    mbid (const mbid_type& x);

    void
    mbid (::std::auto_ptr< mbid_type > p);

    // url
    // 
    typedef ::xml_schema::uri url_type;
    typedef ::xsd::cxx::tree::traits< url_type, char > url_traits;

    const url_type&
    url () const;

    url_type&
    url ();

    void
    url (const url_type& x);

    void
    url (::std::auto_ptr< url_type > p);

    // Constructors.
    //
    artist (const name_type&,
            const mbid_type&,
            const url_type&);

    artist (const ::xercesc::DOMElement& e,
            ::xml_schema::flags f = 0,
            ::xml_schema::type* c = 0);

    artist (const artist& x,
            ::xml_schema::flags f = 0,
            ::xml_schema::type* c = 0);

    virtual artist*
    _clone (::xml_schema::flags f = 0,
            ::xml_schema::type* c = 0) const;

    // Implementation.
    //
    protected:
    void
    parse (::xsd::cxx::xml::dom::parser< char >&,
           ::xml_schema::flags);

    private:
    ::xsd::cxx::tree::one< name_type > name_;
    ::xsd::cxx::tree::one< mbid_type > mbid_;
    ::xsd::cxx::tree::one< url_type > url_;
  };

  class image: public ::xml_schema::uri
  {
    public:
    // size
    // 
    typedef ::xml_schema::ncname size_type;
    typedef ::xsd::cxx::tree::traits< size_type, char > size_traits;

    const size_type&
    size () const;

    size_type&
    size ();

    void
    size (const size_type& x);

    void
    size (::std::auto_ptr< size_type > p);

    // Constructors.
    //
    image (const ::xml_schema::uri&,
           const size_type&);

    image (const ::xercesc::DOMElement& e,
           ::xml_schema::flags f = 0,
           ::xml_schema::type* c = 0);

    image (const image& x,
           ::xml_schema::flags f = 0,
           ::xml_schema::type* c = 0);

    virtual image*
    _clone (::xml_schema::flags f = 0,
            ::xml_schema::type* c = 0) const;

    // Implementation.
    //
    protected:
    void
    parse (::xsd::cxx::xml::dom::parser< char >&,
           ::xml_schema::flags);

    private:
    ::xsd::cxx::tree::one< size_type > size_;
  };
}

#include <iosfwd>

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMInputSource.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>

namespace lfm_artisttoptracks
{
  // Parse a URI or a local file.
  //

  ::std::auto_ptr< ::lfm_artisttoptracks::lfm >
  lfm_ (const ::std::string& uri,
        ::xml_schema::flags f = 0,
        const ::xml_schema::properties& p = ::xml_schema::properties ());

  ::std::auto_ptr< ::lfm_artisttoptracks::lfm >
  lfm_ (const ::std::string& uri,
        ::xml_schema::error_handler& eh,
        ::xml_schema::flags f = 0,
        const ::xml_schema::properties& p = ::xml_schema::properties ());

  ::std::auto_ptr< ::lfm_artisttoptracks::lfm >
  lfm_ (const ::std::string& uri,
        ::xercesc::DOMErrorHandler& eh,
        ::xml_schema::flags f = 0,
        const ::xml_schema::properties& p = ::xml_schema::properties ());

  // Parse std::istream.
  //

  ::std::auto_ptr< ::lfm_artisttoptracks::lfm >
  lfm_ (::std::istream& is,
        ::xml_schema::flags f = 0,
        const ::xml_schema::properties& p = ::xml_schema::properties ());

  ::std::auto_ptr< ::lfm_artisttoptracks::lfm >
  lfm_ (::std::istream& is,
        ::xml_schema::error_handler& eh,
        ::xml_schema::flags f = 0,
        const ::xml_schema::properties& p = ::xml_schema::properties ());

  ::std::auto_ptr< ::lfm_artisttoptracks::lfm >
  lfm_ (::std::istream& is,
        ::xercesc::DOMErrorHandler& eh,
        ::xml_schema::flags f = 0,
        const ::xml_schema::properties& p = ::xml_schema::properties ());

  ::std::auto_ptr< ::lfm_artisttoptracks::lfm >
  lfm_ (::std::istream& is,
        const ::std::string& id,
        ::xml_schema::flags f = 0,
        const ::xml_schema::properties& p = ::xml_schema::properties ());

  ::std::auto_ptr< ::lfm_artisttoptracks::lfm >
  lfm_ (::std::istream& is,
        const ::std::string& id,
        ::xml_schema::error_handler& eh,
        ::xml_schema::flags f = 0,
        const ::xml_schema::properties& p = ::xml_schema::properties ());

  ::std::auto_ptr< ::lfm_artisttoptracks::lfm >
  lfm_ (::std::istream& is,
        const ::std::string& id,
        ::xercesc::DOMErrorHandler& eh,
        ::xml_schema::flags f = 0,
        const ::xml_schema::properties& p = ::xml_schema::properties ());

  // Parse xercesc::DOMInputSource.
  //

  ::std::auto_ptr< ::lfm_artisttoptracks::lfm >
  lfm_ (const ::xercesc::DOMInputSource& is,
        ::xml_schema::flags f = 0,
        const ::xml_schema::properties& p = ::xml_schema::properties ());

  ::std::auto_ptr< ::lfm_artisttoptracks::lfm >
  lfm_ (const ::xercesc::DOMInputSource& is,
        ::xml_schema::error_handler& eh,
        ::xml_schema::flags f = 0,
        const ::xml_schema::properties& p = ::xml_schema::properties ());

  ::std::auto_ptr< ::lfm_artisttoptracks::lfm >
  lfm_ (const ::xercesc::DOMInputSource& is,
        ::xercesc::DOMErrorHandler& eh,
        ::xml_schema::flags f = 0,
        const ::xml_schema::properties& p = ::xml_schema::properties ());

  // Parse xercesc::DOMDocument.
  //

  ::std::auto_ptr< ::lfm_artisttoptracks::lfm >
  lfm_ (const ::xercesc::DOMDocument& d,
        ::xml_schema::flags f = 0,
        const ::xml_schema::properties& p = ::xml_schema::properties ());

  ::std::auto_ptr< ::lfm_artisttoptracks::lfm >
  lfm_ (::xercesc::DOMDocument* d,
        ::xml_schema::flags f = 0,
        const ::xml_schema::properties& p = ::xml_schema::properties ());
}

#include <xsd/cxx/post.hxx>

// Begin epilogue.
//
//
// End epilogue.

#endif // XSD_ARTIST_TOPTRACKS_2_0_HXX