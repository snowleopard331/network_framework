/**
* author    :   jovi
* email     :   jovi.qiu@7road.com
* date      :   2018/06/08
*/

#ifndef _CALLBACK_H_
#define _CALLBACK_H_

#include <map>
#include <typeindex>
#include <functional>
#include <memory>

// �洢�õĻ�������
struct Func_t
{

};

// �洢�ص�������map�ṹ
using callbacks_t = std::map<std::type_index, std::unique_ptr<Func_t>>;

// �洢����index��ӳ��ṹ
using callbackTypeToIndex = std::map<unsigned int, std::type_index>;

// һ���ص�����������
template<typename ...A>
struct Cb_t :
    public Func_t
{
    using cb = std::function<void(A...)>;
    cb callback;

    Cb_t(cb pCallback) : callback(pCallback) {}
};


class Callback
{
public:
    template<typename ...A>
    static void call(std::type_index index, A&& ... args);

public:
    static callbacks_t              callbacks;
};

#define CALLBACK_FUNCTION_ADD(CBType, pFunc, ...)  \
    do \
    {   \
        if(pFunc != nullptr) \
        {   \
            using func = Cb_t<__VA_ARGS__>;  \
            std::unique_ptr<func> f(new func(&pFunc));  \
            std::type_index index(typeid(f));   \
            Callback::callbacks.insert(callbacks_t::value_type(index, std::move(f))); \
            Callback::cbTypeIndex.insert(callbackTypeToIndex::value_type(CBType, index));   \
        }   \
    } while (0);


#define CALLBACK_MEMBER_FUNCTION_ADD_ARG_1(pFunc, pObj, Type1)  \
    do \
    {   \
        if (pFunc != nullptr)   \
        {   \
            using func = Cb_t<Type1>; \
            std::unique_ptr<func> f(new func(std::bind(pFunc, pObj, std::placeholders::_1)));  \
            std::type_index index(typeid(f));   \
            Callback::callbacks.insert(callbacks_t::value_type(index, std::move(f)));   \
        }   \
    } while (0);

#define CALLBACK_MEMBER_FUNCTION_ADD_ARG_2(pFunc, pObj, Type1, Type2)  \
    do \
    {   \
        if (pFunc != nullptr)   \
        {   \
            using func = Cb_t<Type1, Type2>; \
            std::unique_ptr<func> f(new func(std::bind(pFunc, pObj, std::placeholders::_1, std::placeholders::_2)));   \
            std::type_index index(typeid(f));   \
            Callback::callbacks.insert(callbacks_t::value_type(index, std::move(f)));   \
        }   \
    } while (0);

#define CALLBACK_MEMBER_FUNCTION_ADD_ARG_3(pFunc, pObj, Type1, Type2, Type3)  \
    do \
    {   \
        if (pFunc != nullptr)   \
        {   \
            using func = Cb_t<Type1, Type2, Type3>; \
            std::unique_ptr<func> f(new func(std::bind(pFunc, pObj, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));    \
            std::type_index index(typeid(f));   \
            Callback::callbacks.insert(callbacks_t::value_type(index, std::move(f)));   \
        }   \
    } while (0);

//#define CALLBACK_MEMBER_FUNCTION_ADD(pFunc, pObj, ArgsCount, ...)  \
//    do  \
//    {   \
//        switch (ArgsCount)  \
//        {   \
//        case 1: \
//        {   \
//            CALLBACK_MEMBER_FUNCTION_ADD_ARG_1(pFunc, pObj, __VA_ARGS__);   \
//            break;  \
//        }   \
//        case 2: \
//        {   \
//            CALLBACK_MEMBER_FUNCTION_ADD_ARG_2(pFunc, pObj, __VA_ARGS__);   \
//            break;  \
//        }   \
//        default:    \
//            break;  \
//        }   \
//    } while (0);




#endif//_CALLBACK_H_