#ifndef FRAMEPROVIDER_H
#define FRAMEPROVIDER_H

#include "bitmap.h"

class FrameProvider{

    private :

        BitMap *bitmap;

    public:

        FrameProvider();
        ~FrameProvider();
        void GetEmptyFrame();
        void ReleaseFrame();
        int NumAvailFrame();
};

#endif // FRAMEPROVIDER_H
