//
// Handle C-list (ed247.h)
// This class contain both the list and its 'iterator'
//
// * client_list<T> inherit from a C struct defined in C ed247 header (template class c_base_list)
// * client_list<T> may have several implementation for different containers: templace class client_list_container
// * The underlayed container shall hold std::shared_ptr<T>
//
#ifndef ED247_CLIENT_ITERATOR
#define ED247_CLIENT_ITERATOR
#include <vector>

namespace ed247 {

  template <class c_base_list, typename T>
  struct client_list : public c_base_list {
    typedef T                  value_t;
    typedef std::shared_ptr<T> container_value_t;

    virtual uint32_t size() const = 0;

    virtual T* get_current() = 0;
    virtual T* get_next() = 0;
    virtual void reset_iterator() = 0;

    virtual void free() = 0;
  };

  template <class c_base_list, typename T, class container_t = std::vector<std::shared_ptr<T>>>
  struct client_list_container : public client_list<c_base_list, T> {
    typedef typename container_t::iterator iterator_t;

    // Invalid iterator Ctor
    client_list_container() : _container(nullptr), _owner(false) { }

    // Initialize iterator by wrapping provided container (will not be freed)
    void wrap(container_t& container) {
      free();
      _container = &container;
      _iterator = _container->end();
      _owner = false;
    }

    // Initialize iterator by copying provided container (the copy will be freed)
    void copy(const container_t& container) {
      free();
      _container = new container_t(container);
      _iterator = _container->end();
      _owner = true;

    }

    uint32_t size() const override {
      return _container->size();
    }

    // Delete container if we are the owner
    virtual void free() override {
      if (_container && _owner) delete _container;
      _container = nullptr;
    }

    ~client_list_container() {
      free();
    }


    void reset_iterator() override {
      _iterator = _container->end();
    }

    T* get_current() override {
      if (_iterator == _container->end()) return nullptr;
      return _iterator->get();
    }

    T* get_next() override {
      if (_iterator == _container->end()) {
        _iterator = _container->begin();
      } else {
        _iterator++;
      }
      return get_current();
    }

  protected:
    container_t* _container;
    iterator_t   _iterator;
    bool         _owner;
  };

}

#endif
