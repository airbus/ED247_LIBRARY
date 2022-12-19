//
// Handle C-list (ed247.h)
// This classes contain both the list and its 'iterator'
//
// * client_list<T> inherit from an emty C struct (template class CBaseList)
// * client_list<T> is implemented by templace class client_list_container wich support various containers (vectors, maps, ...)
// * The underlayed container must hold std::shared_ptr<T>
//
#ifndef ED247_CLIENT_ITERATOR
#define ED247_CLIENT_ITERATOR
#include <memory>
#include <algorithm>


namespace ed247 {

  // Define iterator_shared_get() that call shared_ptr::get() on the provided iterator.
  // Support vector-type and map-type iterators.
  template <typename>
  struct is_pair : std::false_type { };

  template <typename T, typename U>
  struct is_pair<std::pair<T, U>> : std::true_type { };

  template<class Return, class Iterator, typename std::enable_if<!is_pair<typename Iterator::value_type>::value, bool>::type = true>
  Return iterator_shared_get(Iterator& itr) {
    return itr->get();
  }
  template<class Return, class Iterator, typename std::enable_if<is_pair<typename Iterator::value_type>::value, bool>::type = true>
  Return iterator_shared_get(Iterator& itr) {
    return itr->second.get();
  }

  enum class ContextOwned { True, False };

  template <class CBaseList, typename T>
  struct client_list : public CBaseList {
    typedef T                  value_t;
    typedef std::shared_ptr<T> container_value_t;

    virtual ~client_list() {}

    // Return true if this client_list is owned by ed247::Context. (i.e. shall not be free)
    virtual bool is_context_owned() = 0;

    virtual uint32_t size() const = 0;

    virtual T* get_current() = 0;
    virtual T* get_next() = 0;               // Looping get: will return begin() after end().
    virtual void reset_iterator() = 0;

    virtual void free() = 0;
  };

  template <class CBaseList, typename T, class Container, ContextOwned context_owned = ContextOwned::False>
  struct client_list_container : public client_list<CBaseList, T> {
    typedef typename Container::iterator iterator_t;

    // Initialize iterator by wrapping provided container (container will not be freed)
    static client_list_container* wrap(Container& container) {
      return new client_list_container(&container, false);
    }

    // Initialize iterator by copying provided container (the copy will be freed)
    static client_list_container* copy(const Container& container) {
      return new client_list_container(new Container(container), true);
    }

    virtual bool is_context_owned() {
      return context_owned == ContextOwned::True;
    }

    uint32_t size() const override {
      return _container->size();
    }

    // Delete container if we are the owner
    virtual void free() override {
      if (_container && _container_owner) delete _container;
      _container = nullptr;
    }

    ~client_list_container() {
      free();
    }

    void reset_iterator() override {
      _iterator = _container->end();
    }

    T* get_current() override {
      if (!_container) return nullptr;
      if (_iterator == _container->end()) return nullptr;
      return iterator_shared_get<T*>(_iterator);
    }

    T* get_next() override {
      if (!_container) return nullptr;
      if (_iterator == _container->end()) {
        _iterator = _container->begin();
      } else {
        _iterator++;
      }
      return get_current();
    }

  protected:
    client_list_container(Container* container, bool container_owner) :
      _container(container), _container_owner(container_owner) {
      _iterator = _container->end();
    }

    Container*  _container;
    iterator_t  _iterator;
    bool        _container_owner;
  };

}

#endif
