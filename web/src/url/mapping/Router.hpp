/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi, <lganzzzo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#ifndef network_url_Router_hpp
#define network_url_Router_hpp

#include "./Subscriber.hpp"
#include "./Pattern.hpp"

#include "../../../../../oatpp-lib/core/src/collection/LinkedList.hpp"

#include "../../../../../oatpp-lib/core/src/base/String.hpp"

#include "../../../../../oatpp-lib/core/src/base/PtrWrapper.hpp"
#include "../../../../../oatpp-lib/core/src/base/Controllable.hpp"
#include "../../../../../oatpp-lib/core/src/base/Environment.hpp"

namespace oatpp { namespace web { namespace url { namespace mapping {
  
template<class Param, class ReturnType>
class Router : public base::Controllable{
public:
  typedef Subscriber<Param, ReturnType> UrlSubscriber;
public:
  
  class Route {
  private:
    UrlSubscriber* m_subscriber;
  public:
    
    Route(UrlSubscriber* subscriber, const std::shared_ptr<Pattern::MatchMap>& pMatchMap)
      : m_subscriber(subscriber)
      , matchMap(pMatchMap)
    {}
    
    ReturnType processUrl(const Param& param){
      return m_subscriber->processUrl(param);
    }
    
    bool isNull(){
      return m_subscriber == nullptr;
    }
    
    const std::shared_ptr<Pattern::MatchMap> matchMap;
    
  };

public:
  
  class Pair : public base::Controllable{
  protected:
    Pair(const std::shared_ptr<Pattern>& pPattern, const std::shared_ptr<UrlSubscriber>& pSubscriber)
      : pattern(pPattern)
      , subscriber(pSubscriber)
    {}
  public:
    
    static std::shared_ptr<Pair> createShared(const std::shared_ptr<Pattern>& pattern, const std::shared_ptr<UrlSubscriber>& subscriber){
      return std::shared_ptr<Pair>(new Pair(pattern, subscriber));
    }
    
    const std::shared_ptr<Pattern> pattern;
    const std::shared_ptr<UrlSubscriber> subscriber;
    
  };
  
private:
  std::shared_ptr<oatpp::collection::LinkedList<std::shared_ptr<Pair>>> m_subscribers;
protected:
  Router()
    : m_subscribers(oatpp::collection::LinkedList<std::shared_ptr<Pair>>::createShared())
  {}
public:
  
  static std::shared_ptr<Router> createShared(){
    return std::shared_ptr<Router>(new Router());
  }
  
  void addSubscriber(const base::String::PtrWrapper& urlPattern,
                     const std::shared_ptr<UrlSubscriber>& subscriber){
    auto pattern = Pattern::parse(urlPattern);
    auto pair = Pair::createShared(pattern, subscriber);
    m_subscribers->pushBack(pair);
  }
  
  Route getRoute(const base::String::PtrWrapper& url){
    auto curr = m_subscribers->getFirstNode();
    while(curr != nullptr) {
      const std::shared_ptr<Pair>& pair = curr->getData();
      curr = curr->getNext();
      
      auto match = pair->pattern->match(url); // new match object is created here
      if(match){
        return Route(pair->subscriber.get(), match);
      }
    }
    return Route(nullptr, nullptr);
  }
  
  void logRouterMappings() {
    auto curr = m_subscribers->getFirstNode();
    while(curr != nullptr){
      const std::shared_ptr<Pair>& pair = curr->getData();
      curr = curr->getNext();
      auto mapping = pair->pattern->toString();
      OATPP_LOGD("Router", "url '%s' -> mapped", (const char*) mapping->getData());
    }
  }
  
};
  
}}}}

#endif /* Router_hpp */