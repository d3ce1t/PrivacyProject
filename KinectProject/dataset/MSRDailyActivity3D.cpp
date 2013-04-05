#include "MSRDailyActivity3D.h"
#include "MSRDailyActivity3DInstance.h"

namespace dai {

MSRDailyActivity3D::MSRDailyActivity3D()
    : Dataset("/home/jose/Dropbox/DataSet_Descriptor/MSRDailyActivity3D.xml")
{

}

DataInstance& MSRDailyActivity3D::getDepthInstance(int activity, int actor, int sample)
{
    DataInstance* result = 0;
    const InstanceInfo instanceInfo = m_metadata->instance(Depth, activity, actor, sample);

    result = new MSRDailyActivity3DInstance(instanceInfo);


    return *result;
}

DataInstance &MSRDailyActivity3D::getColorInstance(int activity, int actor, int sample)
{

}

}
