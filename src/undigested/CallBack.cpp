/**
* author    :   jovi
* email     :   jovi.qiu@7road.com
* date      :   2018/06/08
*/

#include "CallBack.h"

callbacks_t Callback::callbacks;

template<typename ...A>
void Callback::call(std::type_index index, A&& ... args)
{
    using func_t = Cb_t<A...>;
    using cb_t = std::function<void(A...)>;

    auto itCB = callbacks.find(index);
    if (itCB == callbacks.end())
    {
        return;
    }

    const Func_t& base = *(itCB->second);
    const cb_t& fun = static_cast<const func_t&>(base).callback;
    fun(std::forward<A>(args)...);  // 当用于一个指向模板实参类型的右值引用函数参数（T&&）时，std::forward会保持实参类型的所有细节
}