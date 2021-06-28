#pragma once

#include "libplyxx.h"
#include <sstream>

// a custom specialisation (and yes, you are allowed (and have to) put this in std)
namespace std {
  template<>
  struct hash<libply::Type> {
    using argument_type = libply::Type;
    using result_type = int;
    
    result_type operator()(argument_type a) const {
      return static_cast<result_type>(a);  
    }
  };
}

namespace libply
{
  typedef std::unordered_map<std::string, Type> TypeMap;
  const TypeMap TYPE_MAP =
  {
    { "uchar", Type::UCHAR },
    { "int", Type::INT },
    { "float", Type::FLOAT },
    { "double", Type::DOUBLE },
  };

  typedef std::unordered_map<Type, unsigned int> TypeSizeMap;
  const TypeSizeMap TYPE_SIZE_MAP =
  {
    { Type::UCHAR, 1 },
    { Type::INT, 4 },
    { Type::FLOAT, 4 },
    { Type::DOUBLE, 8 },
  };

  /// Type conversion functions.

  inline void convert_UCHAR( const textio::SubString &token, IScalarProperty &property )
  {
    property = textio::stou<unsigned char>( token );
  }

  inline void convert_INT( const textio::SubString &token, IScalarProperty &property )
  {
    property = textio::stoi<int>( token );
  }

  inline void convert_FLOAT( const textio::SubString &token, IScalarProperty &property )
  {
    property = textio::stor<float>( token );
  }

  inline void convert_DOUBLE( const textio::SubString &token, IScalarProperty &property )
  {
    property = textio::stor<double>( token );
  }

  typedef void( *ConversionFunction )( const textio::SubString &, IScalarProperty & );
  typedef std::unordered_map<Type, ConversionFunction> ConversionFunctionMap;

  const ConversionFunctionMap CONVERSION_MAP =
  {
    { Type::UCHAR , convert_UCHAR },
    { Type::INT, convert_INT },
    { Type::FLOAT, convert_FLOAT },
    { Type::DOUBLE, convert_DOUBLE }
  };

  /// Type casting functions.

  inline void cast_UCHAR( char *buffer, IScalarProperty &property )
  {
    property = *reinterpret_cast<unsigned char *>( buffer );
  }

  inline void cast_INT( char *buffer, IScalarProperty &property )
  {
    property = *reinterpret_cast<int *>( buffer );
  }

  inline void cast_FLOAT( char *buffer, IScalarProperty &property )
  {
    property = *reinterpret_cast<float *>( buffer );
  }

  inline void cast_DOUBLE( char *buffer, IScalarProperty &property )
  {
    property = *reinterpret_cast<double *>( buffer );
  }

  typedef void( *CastFunction )( char *buffer, IScalarProperty & );
  typedef std::unordered_map<Type, CastFunction> CastFunctionMap;

  const CastFunctionMap CAST_MAP =
  {
    { Type::UCHAR , cast_UCHAR },
    { Type::INT, cast_INT },
    { Type::FLOAT, cast_FLOAT },
    { Type::DOUBLE, cast_DOUBLE }
  };

  inline std::stringstream &write_convert_UCHAR( IScalarProperty &property, std::stringstream &ss )
  {
    ss << static_cast<unsigned int>( property );
    return ss;
  }

  inline std::stringstream &write_convert_INT( IScalarProperty &property, std::stringstream &ss )
  {
    ss << static_cast<int>( property );
    return ss;
  }

  inline std::stringstream &write_convert_FLOAT( IScalarProperty &property, std::stringstream &ss )
  {
    ss << static_cast<float>( property );
    return ss;
  }

  inline std::stringstream &write_convert_DOUBLE( IScalarProperty &property, std::stringstream &ss )
  {
    ss << static_cast<double>( property );
    return ss;
  }

  typedef std::stringstream &( *WriteConvertFunction )( IScalarProperty &, std::stringstream & );
  typedef std::unordered_map<Type, WriteConvertFunction> WriteConvertFunctionMap;

  const WriteConvertFunctionMap WRITE_CONVERT_MAP =
  {
    { Type::UCHAR , write_convert_UCHAR },
    { Type::INT, write_convert_INT },
    { Type::FLOAT, write_convert_FLOAT },
    { Type::DOUBLE, write_convert_DOUBLE }
  };

  inline void write_cast_UCHAR( IScalarProperty &property, char *buffer, size_t &size )
  {
    *reinterpret_cast<unsigned char *>( buffer ) = static_cast<unsigned int>( property );
    size = sizeof( unsigned char );
  }

  inline void write_cast_INT( IScalarProperty &property, char *buffer, size_t &size )
  {
    *reinterpret_cast<int *>( buffer ) = static_cast<int>( property );
    size = sizeof( int );
  }

  inline void write_cast_FLOAT( IScalarProperty &property, char *buffer, size_t &size )
  {
    *reinterpret_cast<float *>( buffer ) = static_cast<float>( property );
    size = sizeof( float );
  }

  inline void write_cast_DOUBLE( IScalarProperty &property, char *buffer, size_t &size )
  {
    *reinterpret_cast<double *>( buffer ) = static_cast<double>( property );
    size = sizeof( double );
  }

  typedef void( *WriteCastFunction )( IScalarProperty &property, char *buffer, size_t &size );
  typedef std::unordered_map<Type, WriteCastFunction> WriteCastFunctionMap;

  const WriteCastFunctionMap WRITE_CAST_MAP =
  {
    { Type::UCHAR , write_cast_UCHAR },
    { Type::INT, write_cast_INT },
    { Type::FLOAT, write_cast_FLOAT },
    { Type::DOUBLE, write_cast_DOUBLE }
  };

  struct PropertyDefinition
  {
    PropertyDefinition( const std::string &name, Type type, bool isList, Type listLengthType = Type::UCHAR )
      : name( name ), type( type ), isList( isList ), listLengthType( listLengthType ),
        conversionFunction( CONVERSION_MAP.at( type ) ),
        castFunction( CAST_MAP.at( type ) ),
        writeConvertFunction( WRITE_CONVERT_MAP.at( type ) ),
        writeCastFunction( WRITE_CAST_MAP.at( type ) )
    {};
    PropertyDefinition( const Property &p )
      : PropertyDefinition( p.name, p.type, p.isList )
    {};

    Property getProperty() const;

    std::string name;
    Type type;
    bool isList;
    Type listLengthType;
    ConversionFunction conversionFunction;
    CastFunction castFunction;
    WriteConvertFunction writeConvertFunction;
    WriteCastFunction writeCastFunction;
  };

  struct ElementDefinition
  {
    ElementDefinition() : ElementDefinition( "", 0, 0 ) {};
    ElementDefinition( const std::string &name, ElementSize size, std::size_t startLine )
      : name( name ), size( size ), startLine( startLine ) {};
    ElementDefinition( const Element &e )
      : name( e.name ), size( e.size )
    {
      for ( const auto &p : e.properties )
      {
        properties.emplace_back( p );
      }
    };

    Element getElement() const;

    std::string name;
    ElementSize size;
    std::vector<PropertyDefinition> properties;
    std::size_t startLine;
  };

  class FileParser
  {
    public:
      explicit FileParser( const PATH_STRING &filename );
      FileParser( const FileParser &other ) = delete;
      ~FileParser();

      std::vector<Element> definitions() const;
      //void setElementInserter(std::string elementName, IElementInserter* inserter);
      void setElementReadCallback( std::string elementName, ElementReadCallback &readCallback );
      void read();

    private:
      void readHeader();
      void parseLine( const textio::SubString &substr, const ElementDefinition &elementDefinition, ElementBuffer &buffer );
      void readBinaryElement( std::ifstream &fs, const ElementDefinition &elementDefinition, ElementBuffer &buffer );

    private:
      typedef std::map<std::string, ElementReadCallback> CallbackMap;

    private:
      PATH_STRING m_filename;
      File::Format m_format;
      std::streamsize m_dataOffset;
      textio::LineReader m_lineReader;
      textio::Tokenizer m_lineTokenizer;
      textio::Tokenizer::TokenList m_tokens;
      std::vector<ElementDefinition> m_elements;
      CallbackMap m_readCallbackMap;
  };

  std::string formatString( File::Format format );
  std::string typeString( Type type );
}
