#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <iostream>
#include <string>

class Exception
{
  std::string msg_;
  friend std::ostream& operator<<(std::ostream& os, const Exception& e);

public:
  explicit Exception( const std::string& msg );
};

std::ostream& operator<<( std::ostream& os, const Exception& e );

#endif // EXCEPTIONS_H
