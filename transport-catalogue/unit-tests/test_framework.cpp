#include "test_framework.h"

using namespace std;

void AssertImpl(bool value, const string& expr_str, const string& file,
                const string& func, unsigned line, const string& hint) {
    if (!value) {
        cerr << file << "(" << line << "): " << func << ": ";
        cerr << "ASSERT(" << expr_str << ") failed.";
        if (!hint.empty()) {
            cerr << " Hint: " << hint;
        }
        cerr << endl;

        abort();
    }
}

ostream& operator<<(ostream& out, const trc::geo::Coordinates& coordintes) {
    out << "{ " << coordintes.lat << ", " << coordintes.lng << " }";
    return out;
}
