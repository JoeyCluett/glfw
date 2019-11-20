#include <iostream>
#include <reactphysics3d/reactphysics3d.h>

using namespace std;
using namespace reactphysics3d;

int main(int argc, char* argv[]) {
    cout << "Hello React\n";

    rp3d::SphereShape* sphereshape = new rp3d::SphereShape(1.0f);

    delete sphereshape;

    return 0;
}
