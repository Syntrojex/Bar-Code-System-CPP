#include "reader/builtin.h"
#include <QRegularExpression>

namespace Reader
{

    bool isValidCode(const QString &code)
    {
        static const QRegularExpression re("^[1-9][0-9]{5}$"); // Input Validation
        return re.match(code).hasMatch();
    }
}
