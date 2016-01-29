#ifndef RIGIDBODY_CLASS
#define RIGIDBODY_CLASS

#include "NatNetTypes.h"

class RigidBody
{
public:
    RigidBody() {}

    RigidBody( sRigidBodyData data)
        : tx (data.x)
        , ty (data.y)
        , tz (data.z)
        , rx (data.qx)
        , ry (data.qy)
        , rz (data.qz)
        , rw (data.qw)
    {}

    float tx;
    float ty;
    float tz;
    float rx;
    float ry;
    float rz;
    float rw;

};

#endif
