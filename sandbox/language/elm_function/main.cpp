#include <iostream>
#include <vector>
#include <string>
#include <map>

/* 
    Compute using recursion rather than Binet's formula in order to test
    out the virtual machine.
*/
int fibonacci (int n)
{
    if (n == 0 || n == 1)
        return n;
    else
        return fibonacci(n - 1) + fibonacci(n - 2);
}

int 
main (int argc, char** argv)
{
    const int kMax = 30;

    std::cout << "Computing Fibonacci Numbers" << std::endl;
    for (int i = 0; i <= kMax; i+= 5)
    {
        std::cout << i << " => " << fibonacci(i) << std::endl;
    }
	return 0;
}