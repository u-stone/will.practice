// seh-cpp-exception.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
// https://docs.microsoft.com/en-us/cpp/build/reference/eh-exception-handling-model?view=msvc-160
// compiler_options_EHA.cpp
// compile with: /EHa
#include <iostream>
#include <excpt.h>
using namespace std;

void fail()
{
    // generates SE and attempts to catch it using catch(...)
    try
    {
        int i = 0, j = 1;
        j /= i;   // This will throw a SE (divide by zero).
        printf("%d", j);
    }
    catch (...)
    {
        // catch block will only be executed under /EHa
        cout << "Caught an exception in catch(...)." << endl;
    }
}

class Base {
public:
    virtual void f1() = 0;
    virtual void f2(int) = 0;
};

class Derived : public Base {
public:
    virtual void f1() {

    }
    virtual void f2(int) {
        std::cout << "hello" << value[99];

    }
private:
    int value[100] = { 0 };
};

int main()
{
    __try
    {
        //fail();
        Derived* p = (Derived*)0x300;
        p->f2(2);
    }

    // __except will only catch an exception here
    __except (EXCEPTION_EXECUTE_HANDLER)
    //__finally
    {
        // if the exception was not caught by the catch(...) inside fail()
        cout << "An exception was caught in __except." << endl;
    }
}