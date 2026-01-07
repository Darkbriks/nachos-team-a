#ifndef FRAMEPROVIDER_H
#define FRAMEPROVIDER_H

class BitMap;

/**
 * Interface for frame allocation strategies.
 */
class IAllocationStrategy {
    public:
        virtual ~IAllocationStrategy() {}

        /**
         * Selects a frame to allocate.
         * @param bitmap Bitmap representing frame usage.
         * @param numFrames Total number of frames.
         * @return Selected frame number, or -1 if none available.
         */
        virtual int SelectFrame(BitMap* bitmap, unsigned int numFrames) = 0;
};

class SequentialAllocationStrategy : public IAllocationStrategy {
    public:
        int SelectFrame(BitMap* bitmap, unsigned int numFrames) override;
};

class RandomAllocationStrategy : public IAllocationStrategy {
    public:
        int SelectFrame(BitMap* bitmap, unsigned int numFrames) override;
};

class FrameProvider {
    private :
        BitMap *bitmap;
        unsigned int numFrames;
        IAllocationStrategy* allocationStrategy;

    public:
        FrameProvider() = delete;
        explicit FrameProvider(IAllocationStrategy* strategy = nullptr);
        ~FrameProvider();

        /**
        * Allocates an empty frame.
        * @return Frame number, or -1 if no frames are available.
        */
        int GetEmptyFrame();

        /**
        * Sets the allocation strategy.
        * @param strategy New allocation strategy.
        */
        void SetAllocationStrategy(IAllocationStrategy* strategy);

        /**
        * Releases a previously allocated frame.
        * @param frameNumber Frame number to release.
        */
        void ReleaseFrame(int frameNumber);

        /**
        * Gets the number of available frames.
        * @return Number of available frames.
        */
        int NumAvailFrame();
};

#endif // FRAMEPROVIDER_H
