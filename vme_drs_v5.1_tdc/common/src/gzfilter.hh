// -*- C++ -*-

#ifndef _GZSTREAM_H
#define _GZSTREAM_H 1

#include<streambuf>
#include<istream>
#include<ostream>
#include<iostream>
#include<cstdlib>
#include<cstdio>
#include<bits/char_traits.h>
#include<zlib.h>

// This is Hirayama san no pakuri (demo tyotto tigau)
// http://www.jah.ne.jp/~naoyuki/Writings/ExtIos.html

namespace h_Utility{
  
  //--------------------------------------------------------------------------
  // Class definition of basic_gzfilterbuf
  //--------------------------------------------------------------------------

  template<typename _CharT, typename _Traits = std::char_traits<_CharT> >
   class basic_gzfilterbuf : public std::basic_streambuf<_CharT, _Traits>{
     
     static const unsigned int size_streambuf = 8192;

   public:
     typedef _CharT                                char_type;
     typedef _Traits                               traits_type;
     typedef typename _Traits::int_type            int_type;
     typedef typename _Traits::pos_type            pos_type;
     typedef typename _Traits::off_type            off_type;

     typedef std::basic_streambuf<_CharT, _Traits> streambuf_type;
     typedef std::basic_istream<_CharT, _Traits>   istream_type;
     typedef std::basic_ostream<_CharT, _Traits>   ostream_type;

   protected:
     istream_type*          in_stream;    // Reference to external
     ostream_type*          out_stream;   // Reference to external

     char_type*             ptr_buffer;
     int                    size_buffer;

     char_type              sbuffer[size_streambuf];
     char_type              dbuffer[size_streambuf];

     z_stream               zlib_stream;  // Struct of zlib
     int                    iflush;       // Command of zlib (deconpress)
     int                    oflush;       // Command of zlib (compress)

   public:
     // Constructor conrresponds to filter (No default constructor)
              basic_gzfilterbuf(istream_type& roIS);
              basic_gzfilterbuf(ostream_type& roOS);
     virtual ~basic_gzfilterbuf();
     
   protected:
     virtual streambuf_type* setbuf(char_type* roPtr, int roSize);
     virtual int_type        overflow(int_type control_flag = _Traits::eof());
     virtual int_type        underflow();
     virtual int             sync();
     
   protected:
     int  compress();
     void decompress();
     
   private:
     basic_gzfilterbuf();
     basic_gzfilterbuf(const basic_gzfilterbuf& object);
     basic_gzfilterbuf& operator =(const basic_gzfilterbuf& object);
   };

  // Implementation of methods -----------------------------------------------

  // Constructor for basic_istream
  template<typename _CharT, typename _Traits>
  inline
  basic_gzfilterbuf<_CharT, _Traits>::basic_gzfilterbuf(istream_type &roIS)
    : streambuf_type(),
      in_stream(&roIS),
      out_stream(NULL)
  {
    // Initialize internal buffer *** streambuf is dbuffer ***
    ptr_buffer  = dbuffer;
    size_buffer = size_streambuf;
    // Set read pointer as force underflow
    this->setg(ptr_buffer,
	       ptr_buffer +size_buffer,
	       ptr_buffer +size_buffer);

    // Initialize zlib
    zlib_stream.zalloc   = Z_NULL;
    zlib_stream.zfree    = Z_NULL;
    zlib_stream.opaque   = Z_NULL;
    zlib_stream.next_in  = Z_NULL;
    zlib_stream.avail_in = 0;
    if(inflateInit2(&zlib_stream, 16+MAX_WBITS) != Z_OK){ // For gzip
    //    if(inflateInit2(&zlib_stream, -15) != Z_OK){ // For stream
    //    if(inflateInit(&zlib_stream) != Z_OK){
      std::cerr << "zlib error " << zlib_stream.msg << std::endl;
      exit(-1);
    }
    iflush               = Z_NO_FLUSH;
    oflush               = Z_NO_FLUSH;
  }
  
  // Constructor for basic_ostream
  template<typename _CharT, typename _Traits>
  inline
  basic_gzfilterbuf<_CharT, _Traits>::basic_gzfilterbuf(ostream_type& roOS)
    : streambuf_type(),
      in_stream(NULL),
      out_stream(&roOS)
  {
    // Initialize internal buffer *** streambuf is sbuffer ***
    ptr_buffer  = sbuffer;
    size_buffer = size_streambuf;
    this->setp(ptr_buffer,
	       ptr_buffer +size_buffer);

    // Initialize zlib
    zlib_stream.zalloc = Z_NULL;
    zlib_stream.zfree  = Z_NULL;
    zlib_stream.opaque = Z_NULL;
    //    if(deflateInit(&zlib_stream, Z_DEFAULT_COMPRESSION) != Z_OK){
    if(deflateInit2(&zlib_stream,
		    Z_DEFAULT_COMPRESSION,
		    Z_DEFLATED,
		    16+MAX_WBITS, // For gzip compatible
		    //			  -15, // For stream data transmit
		    8,
		    Z_DEFAULT_STRATEGY
		    ) != Z_OK){
      std::cerr << "zlib error " << zlib_stream.msg << std::endl;
    }
    iflush             = Z_NO_FLUSH;
    oflush             = Z_NO_FLUSH;
  }

  // Destructor
  template<typename _CharT, typename _Traits>
  inline
  basic_gzfilterbuf<_CharT, _Traits>::~basic_gzfilterbuf(){
    iflush = Z_FINISH;
    oflush = Z_FINISH;
    this->sync();

    if(in_stream != NULL){
      if(inflateEnd(&zlib_stream) != Z_OK){
	std::cerr << "zlib error " << zlib_stream.msg << std::endl;
      }
    }
    if(out_stream != NULL){
      if(deflateEnd(&zlib_stream) != Z_OK){
	std::cerr << "zlib error " << zlib_stream.msg << std::endl;
      }
    }
  }

  // setbuf (overwrite internal buffer with external input)
  template<typename _CharT, typename _Traits>
  inline
  typename basic_gzfilterbuf<_CharT, _Traits>::streambuf_type*
  basic_gzfilterbuf<_CharT, _Traits>::setbuf(char_type* roPtr, int roSize){
    if(in_stream != NULL){
      this->setg(roPtr, roPtr, roPtr +roSize);
    }
    
    if(out_stream != NULL){
      this->setp(roPtr, roPtr +roSize);
    }
    
    ptr_buffer  = roPtr;
    size_buffer = roSize;
    
    return this;
  }
  
  // underflow (method for in_stream (decompress))
  template<typename _CharT, typename _Traits>
  inline
  typename basic_gzfilterbuf<_CharT, _Traits>::int_type
  basic_gzfilterbuf<_CharT, _Traits>::underflow(){
    if(this->egptr() <= this->gptr()){
      if(iflush == Z_FINISH){
	return _Traits::eof();
      }
      decompress();
      if(this->egptr() <= this->gptr()){
	return _Traits::eof();
      }
    }
    
    return _Traits::to_int_type(*(this->gptr()));
  }

  // overflow (method for out_stream (compress))
  template<typename _CharT, typename _Traits>
  inline
  typename basic_gzfilterbuf<_CharT, _Traits>::int_type
  basic_gzfilterbuf<_CharT, _Traits>::overflow(int_type control_flag){
    compress();
    if(control_flag != _Traits::eof()){
      *(this->pptr()) = _Traits::to_char_type(control_flag);
      this->pbump(1);
      return _Traits::not_eof(control_flag);
    }else{
      return _Traits::eof();
    }
  }

  // sync
  template<typename _CharT, typename _Traits>
  inline
  int
  basic_gzfilterbuf<_CharT, _Traits>::sync(){
    if(in_stream != NULL){;}
    
    if(out_stream != NULL){
      while(Z_OK == compress());
    }
    
    return 0;
  }

  // decompress
  template<typename _CharT, typename _Traits>
  inline
  void
  basic_gzfilterbuf<_CharT, _Traits>::decompress(){
    zlib_stream.next_out  = (unsigned char*)ptr_buffer;
    zlib_stream.avail_out = size_buffer;

    for(;;){
      if(zlib_stream.avail_in == 0){
	// input buffer if empty
	in_stream->read(sbuffer, size_buffer);
	zlib_stream.next_in  = (unsigned char*)sbuffer;
	zlib_stream.avail_in = in_stream->gcount();

	if(zlib_stream.avail_in == 0){
	  iflush = Z_FINISH;
	}
      }

      if(zlib_stream.avail_out == 0){
	// output buffer is empty
	this->setg(ptr_buffer,
		   ptr_buffer,
		   ptr_buffer +size_buffer);
	return;
      }

      int status = inflate(&zlib_stream, iflush);
      if(status == Z_STREAM_END){
	// Finish
	this->setg(ptr_buffer,
		   ptr_buffer,
		   ptr_buffer +size_buffer -zlib_stream.avail_out);
	return;
      }

      if(status != Z_OK){
	std::cerr << "zlib error " << zlib_stream.msg << std::endl;
      }
    }
  }

  //compress
  template<typename _CharT, typename _Traits>
  inline
  int
  basic_gzfilterbuf<_CharT, _Traits>::compress(){
    zlib_stream.next_in   = (unsigned char*)ptr_buffer;
    zlib_stream.avail_in  = this->pptr() -ptr_buffer;
    zlib_stream.next_out  = (unsigned char*)dbuffer;
    zlib_stream.avail_out = size_buffer;

    for(;;){
      int status = deflate(&zlib_stream, oflush);
      if(status == Z_STREAM_END){
	// Finish
	out_stream->write(dbuffer,
			  size_buffer -zlib_stream.avail_out);
	return status;
      }

      if(status != Z_OK){
	if(status == Z_BUF_ERROR){
	  //	  std::cerr << "zlib error " << zlib_stream.msg << std::endl;
	}else{
	  std::cerr << "zlib error " << zlib_stream.msg << std::endl;
	}
      }

      if(zlib_stream.avail_in == 0){
	// input buffer is empty
	out_stream->write(dbuffer,
			  size_buffer -zlib_stream.avail_out);
	this->setp(ptr_buffer,
		   ptr_buffer +size_buffer);
	return status;
      }

      if(zlib_stream.avail_out == 0){
	// output buffer is empty
	out_stream->write(dbuffer, size_buffer);
	zlib_stream.next_out  = (unsigned char*)dbuffer;
	zlib_stream.avail_out = size_buffer;
      }

    }
  }

  //--------------------------------------------------------------------------
  // Class definition of basic_igzfilter
  //--------------------------------------------------------------------------
  template<typename _CharT, typename _Traits = std::char_traits<_CharT> >
    class basic_igzfilter : public std::basic_istream<_CharT, _Traits>{
    public:
      typedef basic_gzfilterbuf<char> gzbuf_type;
      typedef std::basic_istream<_CharT, _Traits>   istream_type;
      typedef std::basic_ostream<_CharT, _Traits>   ostream_type;
    
    public:
       basic_igzfilter(std::istream& roIS)
	 : istream_type(new gzbuf_type(roIS))
      {
	
      }

      ~basic_igzfilter()
      {
	delete this->rdbuf();
      }

    private:
      basic_igzfilter();
      basic_igzfilter(const basic_igzfilter& object);
      basic_igzfilter& operator =(const basic_igzfilter& object);
    };
  
  //--------------------------------------------------------------------------
  // Class definition of basic_ogzfilter
  //--------------------------------------------------------------------------
  template<typename _CharT, typename _Traits = std::char_traits<_CharT> >
    class basic_ogzfilter : public std::basic_ostream<_CharT, _Traits>{
    public:
      typedef basic_gzfilterbuf<char> gzbuf_type;
      typedef std::basic_istream<_CharT, _Traits>   istream_type;
      typedef std::basic_ostream<_CharT, _Traits>   ostream_type;

    public:
       basic_ogzfilter(std::ostream& roOS)
	 : ostream_type(new gzbuf_type(roOS))
      {
	
      }
      
      ~basic_ogzfilter()
      {
	this->flush();
	delete this->rdbuf();
      }

    private:
      basic_ogzfilter();
      basic_ogzfilter(const basic_ogzfilter& object);
      basic_ogzfilter& operator =(const basic_ogzfilter& object);
    };

  typedef basic_igzfilter<char> igzfilter;
  typedef basic_ogzfilter<char> ogzfilter;
    
}

#endif
