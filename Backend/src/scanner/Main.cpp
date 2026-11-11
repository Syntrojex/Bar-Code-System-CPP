#include "Scanner.h"
#include <iostream>
using namespace std;
int main()
{
    Scanner camScanner;
    string six;
    // call scaner to display the six digits or the error message
    if (camScanner.scan(six))
    {
        cout << "Barcode Number: " << six << endl;
    }
    else
    { // error message dispaly in terminal
        cout << "Error: " << camScanner.errMessage() << endl;
    }
    return 0;
}