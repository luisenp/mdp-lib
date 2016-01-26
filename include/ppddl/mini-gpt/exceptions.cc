#include "exceptions.h"

Exception::Exception( const std::string& msg )
  : msg_(msg)
{
}

std::ostream&
operator<<( std::ostream& os, const Exception& e )
{
  return os << "<exception>: ERROR: " << e.msg_;
}
