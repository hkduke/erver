#ifndef TYPE_CONVERT_H
#define TYPE_CONVERT_H

#include <string>
#include <sstream>

typedef typename std::basic_stringstream<char> SStream;
    
template< class T > inline
	int ValueToString( const T& value , std::string& data )
{
	SStream sstream;
	sstream.precision(16);
	sstream<<value;
	sstream>>data;

	if( sstream.fail() || sstream.bad() ){
		return -1;
	}

	return 0;
}

inline int ValueToString( const std::string& value , std::string& data ){
	data = value;
	return 0;
}

inline int ValueToString( const char* value , std::string& data ){
	data = value;
	return 0;
}

template< class T > inline
int StringToValue( 
		const char*   data ,
		const uint32_t& size ,
		T&            value
	)
{
	SStream sstream;
	sstream.write( data , size );

	sstream >> value;

	if( sstream.bad() || sstream.fail() )
	{
		return -1;
	}

	return 0;
}

inline int StringToValue( 
	const char* data ,
	const uint32_t& size ,
	char& value
	)
{
	int32_t content = 0;
	SStream sstream;
	sstream.write( data , size );
	sstream >> content;

	if( sstream.bad() || sstream.fail() )
	{
		return -1;
	}

	if( content >= -128 && content <= 128 )
	{
		value = (char)content;
		return 0;
	}

	return -1;
}

inline int StringToValue( 
	const char* data ,
	const uint32_t& size ,
	unsigned char& value
	)
{
	int32_t content = 0;
	SStream sstream;
	sstream.write( data , size );
	sstream >> content;

	if( sstream.bad() || sstream.fail() )
	{
		return -1;
	}

	if( content <= 256 )
	{
		value = (unsigned char)content;
		return 0;
	}

	return -1;
}


inline int StringToValue(
	const char* data ,
	const uint32_t& size ,
	std::string& value
	)
{
	value.assign( data , size );
	return 0;
}

template< class T >
int StringToValue( 
	const std::string& data ,
	T&            value
	)
{
	SStream sstream;
	sstream << data ;
	sstream >> value;

	if( sstream.bad() || sstream.fail() )
	{
		return -1;
	}

	return 0;
}

inline int StringToValue( 
	const std::string& data ,
	char& value
	)
{
	int32_t content = 0;
	SStream sstream;
	sstream << data ;
	sstream >> content;

	if( sstream.bad() || sstream.fail() )
	{
		return -1;
	}

	if( content >= -128 && content <= 128 )
	{
		value = (char)content;
		return 0;
	}

	return -1;
}

inline int StringToValue( 
	const std::string& data ,
	unsigned char& value
	)
{
	int32_t content = 0;
	SStream sstream;
	sstream << data ;
	sstream >> content;

	if( sstream.bad() || sstream.fail() )
	{
		return -1;
	}

	if( content <= 256 )
	{
		value = (unsigned char)content;
		return 0;
	}

	return -1;
}


inline int StringToValue(
		const std::string& data ,
		      std::string& value
	)
{
	value = data;
    return 0;
}

#endif
