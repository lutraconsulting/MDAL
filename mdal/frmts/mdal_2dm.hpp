/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_2DM_HPP
#define MDAL_2DM_HPP

#include <QString>
#include "mdal_defines.hpp"
#include "mdal.h"

namespace MDAL {

class Loader2dm {
public:
    Loader2dm(const QString& meshFile);
    Mesh* load(Status* status);

private:
    QString mMeshFile;
};

} // namespace MDAL
#endif //MDAL_2DM_HPP
