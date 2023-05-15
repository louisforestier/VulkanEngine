#pragma once

#include <vk_types.h>

#define PROFILING

#ifdef PROFILING
#define PROFILER_CHECK(x) x
#else
#define PROFILER_CHECK(x) 
#endif

namespace vkutil
{
    class VulkanProfiler;

    struct ScopeTimer
    {
        uint32_t _startTimestamp;
        uint32_t _endTimestamp;
        std::string _name;
    };
    
    struct StatRecorder
    {
        uint32_t _query;
        std::string _name;

    };

    class VulkanScopeTimer
    {
    public:
        VulkanScopeTimer(VkCommandBuffer commands, VulkanProfiler& pf, const char* name);
        ~VulkanScopeTimer();
    private:
        VulkanProfiler& _profiler;
        VkCommandBuffer _cmd;
        ScopeTimer _timer;
    };    
    
    class VulkanPipelineStatRecorder
    {
    public:
        VulkanPipelineStatRecorder(VkCommandBuffer commands, VulkanProfiler& pf, const char* name);
        ~VulkanPipelineStatRecorder();
    private:
        VulkanProfiler& _profiler;
        VkCommandBuffer _cmd;
        StatRecorder _timer;
    };
    
    class VulkanProfiler
    {
    public:
        void init(VkDevice device, float timestampPeriod, int perFramePoolSizes = 100);

        void grab_queries(VkCommandBuffer cmd);

        void cleanup();

        double get_stat(const std::string& name);
        VkQueryPool get_timer_pool();
        VkQueryPool get_stat_pool();

        void add_timer(ScopeTimer& timer);

        void add_stat(StatRecorder& timer);
        uint32_t get_timestamp_id();
        uint32_t get_stat_id();

        std::unordered_map<std::string, double> _timing;
        std::unordered_map<std::string, int32_t> _stats;

    private:
        struct QueryFrameState {
            std::vector<ScopeTimer> _frameTimers;
            VkQueryPool _timerPool;
            uint32_t _timerLast;
            std::vector<StatRecorder> _statRecorders;
            VkQueryPool _statPool;
            uint32_t _statLast;
        };

        static constexpr int QUERY_FRAME_OVERLAP = 3;

        int _currentFrame;
        float _period;
        std::array<QueryFrameState, QUERY_FRAME_OVERLAP> _queryFrames;

        VkDevice _device;        
    };

} // namespace vkutil
