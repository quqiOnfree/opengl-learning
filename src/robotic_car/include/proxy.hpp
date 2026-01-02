#pragma once

#include <type_traits>

template <class ConProc, class DisProc> class Proxy {
  static_assert(std::is_invocable_v<ConProc>);
  static_assert(std::is_invocable_v<DisProc>);

public:
  template <class SubConProc, class SubDisProc>
  Proxy(SubConProc &&con_proc, SubDisProc &&dis_proc)
      : con_proc_(std::forward<SubConProc>(con_proc)),
        dis_proc_(std::forward<SubDisProc>(dis_proc)) {
    std::invoke(con_proc_);
  }

  ~Proxy() { std::invoke(dis_proc_); }

private:
  ConProc con_proc_;
  DisProc dis_proc_;
};

template <class DisProc> class Proxy<void, DisProc> {
  static_assert(std::is_invocable_v<DisProc>);

public:
  template <class SubDisProc>
  Proxy(SubDisProc &&dis_proc_)
      : dis_proc_(std::forward<SubDisProc>(dis_proc_)) {}

  ~Proxy() { std::invoke(dis_proc_); }

private:
  DisProc dis_proc_;
};
