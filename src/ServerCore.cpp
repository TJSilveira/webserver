#include "../includes/ServerCore.hpp"
#include <cstddef>
#include <iostream>
#include <ostream>
#include <vector>

// Constructors
ServerCore::ServerCore(void) {}
ServerCore::ServerCore(const ServerCore &other) { *this = other; }

// Destructor
ServerCore::~ServerCore(void) {}

// Overloaded Operator
ServerCore &ServerCore::operator=(const ServerCore &other) {
  if (this != &other) {
    this->_deque = other._deque;
    this->_vec = other._vec;
  }
  return *this;
}
