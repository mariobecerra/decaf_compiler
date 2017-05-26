

#ifndef _H_hashtable
#define _H_hashtable

#include <map>
#include <string.h>

struct ltstr
{
    bool operator() (const char *s1, const char *s2) const
    { return strcmp(s1, s2) < 0; }
};

template <class Value> class Iterator;

template <class Value> class Hashtable
{
  private:
    std::multimap<const char*, Value, ltstr> mmap;

  public:
    
    Hashtable() {}

    
    int NumEntries() const;

    
    
    
    
    
    
    void Enter(const char *key, Value value,
               bool overwriteInsteadOfShadow = true);

    
    
    
    
    void Remove(const char *key, Value value);

    
    
    
    Value Lookup(const char *key);

    
    
    Iterator<Value> GetIterator();

};


template<class Value> class Iterator
{
  friend class Hashtable<Value>;

  private:
    typename std::multimap<const char*, Value , ltstr>::iterator cur, end;
    Iterator(std::multimap<const char*, Value, ltstr>& t)
      : cur(t.begin()), end(t.end()) {}

  public:
    
    
    
    Value GetNextValue();
};

#include "hashtable.cc" 

#endif

