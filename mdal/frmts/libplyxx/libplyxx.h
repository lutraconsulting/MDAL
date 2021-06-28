#pragma once

#include <vector>
#include <array>
#include <map>
#include <unordered_map>
#include <type_traits>
#include <cassert>
#include <memory>
#include <functional>

#include "textio.h"

#ifdef _WIN32
#define PATH_STRING std::wstring
#define Str(s) L##s
#else
#define PATH_STRING std::string
#define Str(s) s
#endif

namespace libply
{
  enum class Type
  {
    INT8,
    UINT8,
    INT16,
    UINT16,
    INT32,
    UINT32,
    FLOAT32,
    FLOAT64
  };

  class IScalarProperty
  {
    public:
      virtual ~IScalarProperty() {}

      virtual IScalarProperty &operator=( unsigned int value ) = 0;
      virtual IScalarProperty &operator=( int value ) = 0;
      virtual IScalarProperty &operator=( float value ) = 0;
      virtual IScalarProperty &operator=( double value ) = 0;

      virtual operator unsigned int() = 0;
      virtual operator int() = 0;
      virtual operator float() = 0;
      virtual operator double() = 0;
  };

  template<typename InternalType>
  class ScalarProperty: public IScalarProperty
  {
    public :

      virtual ~ScalarProperty() {}

      virtual ScalarProperty &operator=( unsigned int value ) override
      { m_value = static_cast<InternalType>( value ); return *this; };
      virtual ScalarProperty &operator=( int value ) override
      { m_value = static_cast<InternalType>( value ); return *this; };
      virtual ScalarProperty &operator=( float value ) override
      { m_value = static_cast<InternalType>( value ); return *this; };
      virtual ScalarProperty &operator=( double value ) override
      { m_value = static_cast<InternalType>( value ); return *this; };

      virtual operator unsigned int() override
      {
        return static_cast<unsigned int>( m_value );
      };
      virtual operator int() override
      {
        return static_cast<int>( m_value );
      };
      virtual operator float() override
      {
        return static_cast<float>( m_value );
      };
      virtual operator double() override
      {
        return static_cast<double>( m_value );
      };

    public:
      InternalType value() const { return m_value; };

    private :
      InternalType m_value;
  };

  struct ElementDefinition;

  class ElementBuffer
  {
    public:
      ElementBuffer() = default;
      ElementBuffer( const ElementDefinition &definition );

    public:
      void reset( size_t size );
      size_t size() const { return properties.size(); };
      IScalarProperty &operator[]( size_t index );

    private:
      void appendScalarProperty( Type type );
      void appendListProperty( Type type );
      std::unique_ptr<IScalarProperty> getScalarProperty( Type type );

    private:
      bool m_isList;
      Type m_listType;
      std::vector<std::unique_ptr<IScalarProperty>> properties;
  };

  struct Property
  {
    Property( const std::string &name, Type type, bool isList )
      : name( name ), type( type ), isList( isList ) {};

    std::string name;
    Type type;
    bool isList;
  };

  typedef std::size_t ElementSize;

  struct Element
  {
    Element( const std::string &name, ElementSize size, const std::vector<Property> &properties )
      : name( name ), size( size ), properties( properties ) {};

    std::string name;
    ElementSize size;
    std::vector<Property> properties;
  };

  typedef std::function< void( ElementBuffer & ) > ElementReadCallback;

  class FileParser;

  typedef std::vector<Element> ElementsDefinition;

  class File
  {
    public:
      File( const PATH_STRING &filename );
      ~File();

      ElementsDefinition definitions() const;
      void setElementReadCallback( std::string elementName, ElementReadCallback &readCallback );
      void read();

    public:
      enum class Format
      {
        ASCII,
        BINARY_LITTLE_ENDIAN,
        BINARY_BIG_ENDIAN
      };

    private:
      PATH_STRING m_filename;
      std::unique_ptr<FileParser> m_parser;
  };


  typedef std::function< void( ElementBuffer &, size_t index ) > ElementWriteCallback;

  class FileOut
  {
    public:
      FileOut( const PATH_STRING &filename, File::Format format );

      void setElementsDefinition( const ElementsDefinition &definitions );
      void setElementWriteCallback( const std::string &elementName, ElementWriteCallback &writeCallback );
      void write();

    private:
      void createFile();
      void writeHeader();
      void writeData();

    private:
      PATH_STRING m_filename;
      File::Format m_format;
      ElementsDefinition m_definitions;
      std::map<std::string, ElementWriteCallback> m_writeCallbacks;
  };
}
