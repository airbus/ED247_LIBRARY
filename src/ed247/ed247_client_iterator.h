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

    // Invalid iterator Ctor
    client_iterator() : _container(nullptr), _owner(false) { }

    // Initialize iterator by wrapping provided container
    void wrap(container_t& container) {
      free();
      _container = &container;
      _iterator = _container->end();
      _owner = false;
    }

    // Initialize iterator by copying provided container
    void copy(const container_t& container) {
      free();
      _container = new container_t(container);
      _iterator = _container->end();
      _owner = true;

    }

    bool is_initialized() {
      return _container != nullptr;
    }

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
      return _container && _iterator != _container->end();
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
    void free() {
      if (_container && _owner) delete _container;
      _container = nullptr;
    }

    ~client_iterator() {
      free();
    }

  protected:
    container_t* _container;
    iterator_t   _iterator;
    bool         _owner;
  };

}

#endif
