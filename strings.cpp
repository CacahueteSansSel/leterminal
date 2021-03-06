#include "strings.h"

namespace Terminal {

namespace Strings {

SecuredString* s_leterminal;
SecuredString* s_leterminal_v;
SecuredString* s_ok;
SecuredString* s_dfuText;

void init() {
    s_leterminal = SecuredString::fromBufferUnsafe("L.E. Terminal");
    s_leterminal_v = SecuredString::fromBufferUnsafe("L.E. Terminal v");
    s_ok = SecuredString::fromBufferUnsafe("OK");
    s_dfuText = SecuredString::fromBufferUnsafe("Calculator is in DFU mode");
}

SecuredString* leterminal() {
    return s_leterminal;
}

SecuredString* leterminal_v() {
    return s_leterminal_v;
}

SecuredString* ok() {
    return s_ok;
}

SecuredString* dfuMode() {
    return s_dfuText;
}

}

}