#include <vk_profiler.h>

namespace vkutil 
{
    VulkanScopeTimer::VulkanScopeTimer(VkCommandBuffer commands, VulkanProfiler &pf, const char *name)
        : _cmd(commands), _profiler(pf)
    {
        _timer._name = name;
        _timer._startTimestamp = _profiler.get_timestamp_id();

        VkQueryPool pool = _profiler.get_timer_pool();

        vkCmdWriteTimestamp(_cmd, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, pool, _timer._startTimestamp);
    }

    VulkanScopeTimer::~VulkanScopeTimer()
    {
        _timer._endTimestamp= _profiler.get_timestamp_id();
        VkQueryPool pool = _profiler.get_timer_pool();
        vkCmdWriteTimestamp(_cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, pool, _timer._endTimestamp);
        _profiler.add_timer(_timer);
    }

    VulkanPipelineStatRecorder::VulkanPipelineStatRecorder(VkCommandBuffer commands, VulkanProfiler &pf, const char *name)
        : _cmd(commands), _profiler(pf)
    {
        _timer._name = name;
        _timer._query = _profiler.get_stat_id();

        vkCmdBeginQuery(_cmd, _profiler.get_stat_pool(), _timer._query, 0);
    }

    VulkanPipelineStatRecorder::~VulkanPipelineStatRecorder()
    {
        VkQueryPool pool = _profiler.get_stat_pool();
        vkCmdEndQuery(_cmd, pool, _timer._query);

        _profiler.add_stat(_timer);
    }

    void VulkanProfiler::init(VkDevice device, float timestampPeriod, int perFramePoolSizes)
    {
        _period = timestampPeriod;
        _device = device;
        _currentFrame = 0;
        int poolSize = perFramePoolSizes;

        VkQueryPoolCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
        createInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
        createInfo.queryCount = poolSize;

        for (size_t i = 0; i < QUERY_FRAME_OVERLAP; i++)
        {
            vkCreateQueryPool(_device, &createInfo, nullptr, &_queryFrames[i]._statPool);
            _queryFrames[i]._timerLast = 0;
        }

        createInfo.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;

        for (size_t i = 0; i < QUERY_FRAME_OVERLAP; i++)
        {
            createInfo.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT;
            vkCreateQueryPool(_device, &createInfo, nullptr, &_queryFrames[i]._statPool);
            _queryFrames[i]._statLast = 0;
        }
    }

    void VulkanProfiler::grab_queries(VkCommandBuffer cmd)
    {
        int frame = _currentFrame;
        _currentFrame = (_currentFrame + 1) % QUERY_FRAME_OVERLAP;

        vkCmdResetQueryPool(cmd, _queryFrames[_currentFrame]._timerPool, 0, _queryFrames[_currentFrame]._timerLast);
        _queryFrames[_currentFrame]._timerLast = 0;
        _queryFrames[_currentFrame]._frameTimers.clear();

        vkCmdResetQueryPool(cmd, _queryFrames[_currentFrame]._statPool, 0, _queryFrames[_currentFrame]._statLast);
        _queryFrames[_currentFrame]._statLast = 0;
        _queryFrames[_currentFrame]._statRecorders.clear();

        QueryFrameState& state = _queryFrames[frame];
        std::vector<uint64_t> queryState;
        queryState.resize(state._timerLast);
        if (state._timerLast != 0)
        {
            //copy results into a host visible buffer
            vkGetQueryPoolResults(
                _device,
                state._timerPool,
                0,
                state._timerLast,
                queryState.size() * sizeof(uint64_t),
                queryState.data(),
                sizeof(uint64_t),
                VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT
            );
        }
        std::vector<uint64_t> statResults;
        statResults.resize(state._statLast);
        if (state._statLast != 0)
        {
            //copy results into a host visible buffer
            vkGetQueryPoolResults(
                _device,
                state._statPool,
                0,
                state._statLast,
                statResults.size() * sizeof(uint64_t),
                statResults.data(),
                sizeof(uint64_t),
                VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT
            );
        }

        for (auto& timer : state._frameTimers)
        {
            uint64_t begin = queryState[timer._startTimestamp]; 
            uint64_t end = queryState[timer._endTimestamp]; 
            uint64_t timestamp = end - begin;
            
            //store timing queries as ms
            _timing[timer._name] = (double(timestamp) * _period) / 1000000.0;
        }

        for (auto& st : state._statRecorders)
        {
            uint64_t result = statResults[st._query];

            _stats[st._name] = static_cast<int32_t>(result);
        }        
    }

    void VulkanProfiler::cleanup()
    {
        for (size_t i = 0; i < QUERY_FRAME_OVERLAP; i++)
        {
            vkDestroyQueryPool(_device, _queryFrames[i]._timerPool, nullptr);
        }        
    }

    double VulkanProfiler::get_stat(const std::string &name)
    {
        auto it = _timing.find(name);
        if (it != _timing.end())
            return (*it).second;
        else return 0.0;
    }

    VkQueryPool VulkanProfiler::get_timer_pool()
    {
        return _queryFrames[_currentFrame]._timerPool; 
    }

    VkQueryPool VulkanProfiler::get_stat_pool()
    {
        return _queryFrames[_currentFrame]._statPool;
    }
    
    void VulkanProfiler::add_timer(ScopeTimer& timer)
    {
        _queryFrames[_currentFrame]._frameTimers.push_back(timer);
    }

    void VulkanProfiler::add_stat(StatRecorder& recorder)
    {
        _queryFrames[_currentFrame]._statRecorders.push_back(recorder);
    }

    uint32_t VulkanProfiler::get_timestamp_id()
    {
        uint32_t q = _queryFrames[_currentFrame]._timerLast;
        _queryFrames[_currentFrame]._timerLast++;
        return q;
    }
    uint32_t VulkanProfiler::get_stat_id()
    {
        uint32_t q = _queryFrames[_currentFrame]._statLast;
        _queryFrames[_currentFrame]._timerLast++;
        return q;
    }
}