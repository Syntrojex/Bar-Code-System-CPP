#include "Scanner.h"
#include <iostream>
using namespace std;
int main()
{
    Scanner camScanner;
    string six;
    if (camScanner.scan(six))
    {
        cout << "Barcode Number: " << six << endl;
    }
    else
    {
        cout << "Error: " << camScanner.errMessage() << endl;
    }
    return 0;
}