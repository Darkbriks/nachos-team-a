#ifndef STACKMANAGER_H
#define STACKMANAGER_H

class AddrSpace;
class Lock;

struct StackRegion {
    unsigned int base;      // High address (top of stack, where SP starts)
    unsigned int limit;     // Low address (bottom of stack)
    bool inUse;             // Currently allocated to a thread
    bool userProvided;      // Stack was provided by user (don't free memory)
    unsigned short threadTid; // TID of thread using this stack (for debugging)

    StackRegion() : base(0), limit(0), inUse(false), userProvided(false), threadTid(0) {}

    StackRegion(const unsigned int b, const unsigned int l, const bool u, const bool up, const unsigned int tid)
        : base(b), limit(l), inUse(u), userProvided(up), threadTid(tid) {}
};

class StackManager {
private:
    AddrSpace* addrSpace;
    unsigned int stackAreaTop;      // Highest valid stack address
    unsigned int stackAreaBottom;   // Lowest valid stack address
    unsigned int nextStackTop;      // Next stack will be allocated below this
    unsigned int maxStacks;
    unsigned int allocatedCount;

    StackRegion* regions;
    Lock* lock;

public:
    /**
     * @brief Create a stack manager for an address space
     *
     * @param space The address space to manage stacks for
     * @param stackAreaTop Top of stack area (highest address, e.g., numPages * PageSize)
     * @param stackAreaBottom Bottom limit (lowest address stacks can reach)
     * @param maxStacks Maximum number of concurrent stacks
     */
    StackManager(AddrSpace* space, unsigned int stackAreaTop, unsigned int stackAreaBottom, unsigned int maxStacks);

    ~StackManager();

    /**
     * @brief Allocate a new stack region (from top, growing downward)
     *
     * @param size Requested stack size in bytes (rounded up to page boundary)
     * @param base Output: base address (high address)
     * @param limit Output: limit address (low address)
     * @return 0 on success, negative error code on failure
     */
    int AllocateStack(unsigned int size, unsigned int* base, unsigned int* limit);

    /**
     * @brief Register a user-provided stack (no allocation, just tracking)
     *
     * @param base Stack base (high address)
     * @param size Stack size
     * @param limit Output: computed limit address
     * @return 0 on success, negative error code on failure
     */
    int RegisterUserStack(unsigned int base, unsigned int size, unsigned int* limit);

    /**
     * @brief Free a previously allocated stack
     *
     * @param base Base address of stack to free
     * @return 0 on success, -E_INVAL if not found
     */
    int FreeStack(unsigned int base);

    /**
     * @brief Mark a stack as in use by a specific thread
     * For now, is only used for debugging purposes
     *
     * @param base Stack base address
     * @param tid Thread ID using this stack
     */
    void MarkInUse(unsigned int base, unsigned int tid) const;

    /**
     * @brief Get stack info for a given base address
     *
     * @param base Stack base address
     * @param outRegion Output parameter for stack region
     * @return true if found, false if not found
     */
    [[nodiscard]] bool GetStackInfo(unsigned int base, StackRegion& outRegion) const;

    /**
     * @brief Check if an address is within any allocated stack
     *
     * @param addr Address to check
     * @return true if within a stack region
     */
    [[nodiscard]] bool IsInStack(unsigned int addr) const;

    [[nodiscard]] unsigned int GetAllocatedCount() const { return allocatedCount; }
    [[nodiscard]] unsigned int GetMaxStacks() const { return maxStacks; }
    [[nodiscard]] unsigned int GetNextStackTop() const { return nextStackTop; }
    [[nodiscard]] unsigned int GetStackAreaTop() const { return stackAreaTop; }
    [[nodiscard]] unsigned int GetStackAreaBottom() const { return stackAreaBottom; }

    [[nodiscard]] bool IsValidUserStackPointer(unsigned int sp) const;
    [[nodiscard]] bool IsKnownStackBase(unsigned int base) const;

    /**
     * @brief Debug: print all stack regions
     */
    void Print();

private:
    /**
     * @brief Find a free slot in regions array
     * @return Index or -1 if none available
     */
    [[nodiscard]] int FindFreeSlot() const;

    /**
     * @brief Find region by base address
     * @return Index or -1 if not found
     */
    [[nodiscard]] int FindByBase(unsigned int base) const;

    /**
     * @brief Round up to page boundary
     */
    unsigned int RoundUpToPage(unsigned int size);
};

#endif