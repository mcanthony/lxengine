#include <iostream>
#include <vector>
#include <string>
#include <map>


std::string
string_from_file  (const char* filename)
{
    FILE* fp = fopen(filename, "r");
    if (fp)
    {
        std::string out;

        // Check the file size
        fseek(fp, 0, SEEK_END);
        const size_t bufferSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        // Allocate the buffer
        out.resize(bufferSize, '\0');

        // Read in the file
        fread(&out[0], sizeof(char), bufferSize, fp);
        fclose(fp);

        // CRLF issues may mean the file size is larger than the string size.
        // Correct that here.
        size_t end = out.find('\0');
        if (end != std::string::npos)
            out.resize(end);

        return out;
    }
    else
        throw "File not found";
}


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

    std::string script = string_from_file("data/fibonacci_basic.elm");

    std::cout << "Computing Fibonacci Numbers" << std::endl;
    for (int i = 0; i <= kMax; i+= 5)
    {
        std::cout << i << " => " << fibonacci(i) << std::endl;
    }
	return 0;
}