#pragma once

#include <type_traits>

template<class ConProc, class DisProc>
class Proxy
{
    static_assert(std::is_invocable_v<ConProc>);
    static_assert(std::is_invocable_v<DisProc>);
public:
    Proxy(ConProc, DisProc):
        con_proc_(std::move(ConProc{})),
        dis_proc_(std::move(DisProc{}))
    {
        std::invoke(con_proc_);
    }

    ~Proxy()
    {
        std::invoke(dis_proc_);
    }

private:
    ConProc con_proc_;
    DisProc dis_proc_;
};

template<class DisProc>
class Proxy<void, DisProc>
{
    static_assert(std::is_invocable_v<DisProc>);
public:
    Proxy(DisProc dis_proc_):
        dis_proc_(std::move(dis_proc_))
    {
    }

    ~Proxy()
    {
        std::invoke(dis_proc_);
    }

private:
    DisProc dis_proc_;
};
