//
// Iterator to handle C-list (ed247.h)
// Note: this is NOT a true C++ iterator
//
#ifndef ED247_CLIENT_ITERATOR
#define ED247_CLIENT_ITERATOR
#include <vector>

namespace ed247 {

  template <typename T>
  struct client_iterator {
    typedef T                               value_t;
    typedef typename std::vector<T>         container_t;
    typedef typename container_t::iterator  iterator_t;

    // Ctor (hold reference to container)
    client_iterator(container_t& container) : _container(&container), _iterator(container.end()), _owner(false) {}

    virtual client_iterator& advance() {
      if (_iterator == _container->end()) {
        _iterator = _container->begin();
      } else {
        _iterator++;
      }
      return *this;
    }

    client_iterator& operator++() {
      return advance();
    }

    T& get_value() {
      return *_iterator;
    }

    T& operator*() {
      return get_value();
    }

    bool valid() {
      return _iterator != _container->end();
    }

    operator bool() {
      return valid();
    }

    uint32_t container_size() {
      return _container->size();
    }

    container_t& container() {
      return *_container;
    }

    // Delete container if we are the owner
    ~client_iterator() {
      if (_owner) delete _container;
    }

  protected:
    // Derived classes shall implement a copy() method that create a client_iterator which hold container
    client_iterator(container_t* container, bool owner = false) : _container(container), _iterator(container->end()), _owner(owner) {}

    container_t* _container;
    iterator_t   _iterator;
    bool         _owner;
  };

}

#endif
