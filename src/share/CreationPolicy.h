/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2015/10/12
*/

#ifndef _CREATION_POLICY_H_
#define _CREATION_POLICY_H_

namespace Evil
{
    // OperatorNew policy creates an object on the heap using new
    // using this with Singleton
    template<class T>
    class OperatorNew
    {
    public:
        static T* Create()
        {
            return (new T);
        }

        static void Destroy(T* obj)
        {
            delete obj;
        }
    };
}

#endif//_CREATION_POLICY_H_